#include "MainWindow.h"
#include "Config.h"
#include "ActionExecutor.h"
#include "GamepadHandler.h"
#include "InterceptionWrapper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QFrame>
#include <iostream>

ClickableLineEdit::ClickableLineEdit(QWidget* parent) : QLineEdit(parent) {}

void ClickableLineEdit::mousePressEvent(QMouseEvent* event) {
    emit clicked(event);
    QLineEdit::mousePressEvent(event);
}

MainWindow::MainWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("TibiaGamepad v1.1.7 (C++)");
    resize(680, 750);
    
    setStyleSheet(R"(
        QWidget { background-color: #242424; color: white; font-family: Arial; }
        QTabWidget::pane { border: 1px solid #444; border-radius: 8px; }
        QTabBar { alignment: center; }
        QTabBar::tab { background: #333; color: #ccc; padding: 8px 30px; margin: 2px; border-radius: 4px; }
        QTabBar::tab:selected { background: #1f538d; color: white; }
        QPushButton { border-radius: 6px; color: white; }
        QLineEdit { background-color: #404244; border: 1px solid #777; border-radius: 4px; padding: 5px; color: white; font-weight: bold; }
        QLineEdit:focus { border: 1px solid #1f538d; background-color: #4a4c4e; }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    tabView = new QTabWidget(this);
    mainLayout->addWidget(tabView);

    QWidget* tabGeneral = new QWidget();
    QWidget* tabButtons = new QWidget();
    QWidget* tabLogs = new QWidget();

    tabView->addTab(tabGeneral, "Geral");
    tabView->addTab(tabButtons, "Mapeamento");
    tabView->addTab(tabLogs, "Logs");

    setupGeneralTab(tabGeneral);
    setupButtonsTab(tabButtons);
    setupLogsTab(tabLogs);

    if (Config::load()) {
        updateUiFromConfig();
    }

    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &MainWindow::updatePeripheralStatus);
    statusTimer->start(1000);

    animationTimer = new QTimer(this);
    animationTimer->setSingleShot(true);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::resetButtonColor);

    // Initial hardware report and start polling
    std::cout << "[LOG] " << intercept::get_device_list_report() << std::endl;
    GamepadHandler::init();
    GamepadHandler::start();
}

MainWindow::~MainWindow() {
    delete logRedirector;
    GamepadHandler::stop();
    ActionExecutor::cleanup();
    intercept::cleanup();
}

void MainWindow::setupGeneralTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setAlignment(Qt::AlignTop);

    QLabel* title = new QLabel("Painel de Controle");
    title->setFont(QFont("Arial", 22, QFont::Bold));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    layout->addSpacing(15);

    statusBtn = new QPushButton("ATIVAR EMULADOR");
    statusBtn->setFixedHeight(55);
    statusBtn->setFont(QFont("Arial", 16, QFont::Bold));
    statusBtn->setStyleSheet("background-color: green;");
    connect(statusBtn, &QPushButton::clicked, this, &MainWindow::toggleEmulator);
    layout->addWidget(statusBtn);
    layout->addSpacing(15);

    QLabel* stickLabel = new QLabel("Analógico do Mouse:");
    stickLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(stickLabel);

    QHBoxLayout* stickLayout = new QHBoxLayout();
    rbLeft = new QRadioButton("LEFT");
    rbRight = new QRadioButton("RIGHT");
    stickGroup = new QButtonGroup(this);
    stickGroup->addButton(rbLeft);
    stickGroup->addButton(rbRight);
    
    QString rbStyle = "QRadioButton { background: #333; padding: 5px 15px; border-radius: 4px; } QRadioButton::indicator { width: 0px; } QRadioButton:checked { background: #1f538d; }";
    rbLeft->setStyleSheet(rbStyle);
    rbRight->setStyleSheet(rbStyle);
    
    stickLayout->addStretch();
    stickLayout->addWidget(rbLeft);
    stickLayout->addWidget(rbRight);
    stickLayout->addStretch();
    layout->addLayout(stickLayout);
    
    if (Config::MOUSE_STICK == "LEFT") rbLeft->setChecked(true); else rbRight->setChecked(true);
    
    connect(stickGroup, &QButtonGroup::buttonClicked, [&](QAbstractButton* button) {
        Config::MOUSE_STICK = button->text().toStdString();
        std::cout << "[LOG] Analógico do Mouse alterado para: " << Config::MOUSE_STICK << std::endl;
    });

    layout->addSpacing(15);
    QLabel* speedLabel = new QLabel("Velocidade do Mouse:");
    speedLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(speedLabel);

    sMouse = new QSlider(Qt::Horizontal);
    sMouse->setRange(1, 20);
    sMouse->setValue(Config::MOUSE_SENSITIVITY.load());
    connect(sMouse, &QSlider::valueChanged, [](int value) {
        Config::MOUSE_SENSITIVITY = value;
        std::cout << "[LOG] Sensibilidade do Mouse: " << value << std::endl;
    });
    layout->addWidget(sMouse);

    layout->addSpacing(35);
    QPushButton* saveBtn = new QPushButton("SALVAR PERFIL");
    saveBtn->setFixedHeight(45);
    saveBtn->setStyleSheet("background-color: #1f538d; font-weight: bold;");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveProfile);
    layout->addWidget(saveBtn);
    
    resetBtn = new QPushButton("REINICIAR DRIVERS (SEGURANÇA)");
    resetBtn->setFixedHeight(45);
    resetBtn->setStyleSheet("background-color: #6c757d; font-weight: bold; margin-top: 10px;");
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetDrivers);
    layout->addWidget(resetBtn);
    
    layout->addSpacing(30);
    
    QFrame* statusPanel = new QFrame();
    statusPanel->setStyleSheet("QFrame { background-color: #333; border-radius: 10px; border: 1px solid #444; }");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusPanel);
    
    QLabel* statusTitle = new QLabel("STATUS DOS PERIFÉRICOS");
    statusTitle->setStyleSheet("border: none; color: #aaa; font-weight: bold; font-size: 10px;");
    statusTitle->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(statusTitle);
    
    auto createStatusRow = [&](const QString& name, QLabel*& label) {
        QHBoxLayout* row = new QHBoxLayout();
        QLabel* nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("border: none; font-weight: bold;");
        label = new QLabel("OFFLINE");
        label->setFixedWidth(100);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("border: none; background-color: #444; border-radius: 5px; color: white; font-size: 10px; padding: 2px;");
        row->addWidget(nameLabel);
        row->addStretch();
        row->addWidget(label);
        statusLayout->addLayout(row);
    };
    
    createStatusRow("🎮 Controle:", gamepadStatus);
    createStatusRow("⌨️ Teclado:", keyboardStatus);
    createStatusRow("🖱️ Mouse:", mouseStatus);
    
    layout->addWidget(statusPanel);
}

void MainWindow::setupButtonsTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border: none; background-color: transparent;");
    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* hBtn = new QLabel("BOTÃO"); hBtn->setFixedWidth(80); hBtn->setStyleSheet("color: gray; font-weight: bold; font-size: 11px;"); hBtn->setAlignment(Qt::AlignCenter);
    QLabel* hMod = new QLabel("MODIFICADOR"); hMod->setFixedWidth(140); hMod->setStyleSheet("color: gray; font-weight: bold; font-size: 11px;"); hMod->setAlignment(Qt::AlignCenter);
    QLabel* hKey = new QLabel("TECLA/MOUSE"); hKey->setStyleSheet("color: gray; font-weight: bold; font-size: 11px;"); hKey->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(hBtn);
    headerLayout->addWidget(hMod);
    headerLayout->addWidget(hKey);
    scrollLayout->addLayout(headerLayout);

    QStringList botoes = {"A", "B", "X", "Y", "LB", "RB", "LT", "RT", "BACK", "START", "UP", "DOWN", "LEFT", "RIGHT", "L3", "R3"};
    for (const QString& btn : botoes) {
        QHBoxLayout* rowLayout = new QHBoxLayout();
        QLabel* lblBtn = new QLabel(btn); lblBtn->setFixedWidth(80); lblBtn->setFont(QFont("Arial", 12, QFont::Bold));
        ClickableLineEdit* valMod = new ClickableLineEdit(); valMod->setFixedWidth(130); valMod->setReadOnly(true); valMod->setAlignment(Qt::AlignCenter);
        ClickableLineEdit* valKey = new ClickableLineEdit(); valKey->setMinimumWidth(160); valKey->setReadOnly(true); valKey->setAlignment(Qt::AlignCenter);
        QPushButton* clearBtn = new QPushButton("X"); clearBtn->setFixedSize(25, 25); clearBtn->setStyleSheet("QPushButton { background-color: #444; border-radius: 12px; } QPushButton:hover { background-color: red; }");
        rowLayout->addWidget(lblBtn); rowLayout->addWidget(valMod); rowLayout->addWidget(valKey); rowLayout->addWidget(clearBtn);
        scrollLayout->addLayout(rowLayout);
        entryWidgets[btn] = qMakePair(valMod, valKey);
        connect(valMod, &ClickableLineEdit::clicked, [this, btn, valMod](QMouseEvent* e) { handleMapClick(btn, 0, valMod, e); });
        connect(valKey, &ClickableLineEdit::clicked, [this, btn, valKey](QMouseEvent* e) { handleMapClick(btn, 1, valKey, e); });
        connect(clearBtn, &QPushButton::clicked, [this, btn]() { clearRow(btn); });
    }
    scrollArea->setWidget(scrollWidget);
    layout->addWidget(scrollArea);
}

void MainWindow::setupLogsTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);
    logTextEdit = new QTextEdit();
    logTextEdit->setReadOnly(true);
    logTextEdit->setFont(QFont("Consolas", 12));
    logTextEdit->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4;");
    layout->addWidget(logTextEdit);
    QPushButton* clearBtn = new QPushButton("Limpar Console");
    clearBtn->setFixedHeight(25);
    clearBtn->setStyleSheet("background-color: #444;");
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearLogs);
    layout->addWidget(clearBtn);
    logRedirector = new LogRedirector(std::cout, logTextEdit);
}

void MainWindow::updateUiFromConfig() {
    for (auto it = entryWidgets.begin(); it != entryWidgets.end(); ++it) {
        QString btn = it.key();
        ClickableLineEdit* me = it.value().first;
        ClickableLineEdit* ke = it.value().second;
        std::vector<std::string> mapping = Config::MAP[btn.toStdString()];
        me->setText(QString::fromStdString(mapping[0]).toUpper());
        ke->setText(QString::fromStdString(mapping[1]).toUpper());
    }
    if (Config::MOUSE_STICK == "LEFT") rbLeft->setChecked(true); else rbRight->setChecked(true);
    sMouse->setValue(Config::MOUSE_SENSITIVITY.load());
    std::cout << "[LOG] Interface sincronizada." << std::endl;
}

void MainWindow::saveProfile() {
    Config::save();
    lastSaveBtn = qobject_cast<QPushButton*>(sender());
    if (lastSaveBtn) {
        lastSaveBtn->setStyleSheet("background-color: #28a745; font-weight: bold;");
        animationTimer->start(500);
    }
}

void MainWindow::resetButtonColor() {
    if (lastSaveBtn) {
        lastSaveBtn->setStyleSheet("background-color: #1f538d; font-weight: bold;");
    }
}

void MainWindow::updatePeripheralStatus() {
    auto setStatus = [](QLabel* label, bool connected) {
        if (!label) return;
        if (connected) {
            label->setText("CONECTADO");
            label->setStyleSheet("border: none; background-color: #28a745; border-radius: 5px; color: white; font-size: 10px; padding: 2px; font-weight: bold;");
        } else {
            label->setText("OFFLINE");
            label->setStyleSheet("border: none; background-color: #dc3545; border-radius: 5px; color: white; font-size: 10px; padding: 2px; font-weight: bold;");
        }
    };
    setStatus(gamepadStatus, GamepadHandler::is_connected());
    setStatus(keyboardStatus, intercept::get_keyboard_count() > 0);
    setStatus(mouseStatus, intercept::get_mouse_count() > 0);
}

void MainWindow::clearLogs() { logTextEdit->clear(); }

void MainWindow::clearRow(const QString& btn) {
    Config::MAP[btn.toStdString()] = {"", ""};
    entryWidgets[btn].first->clear();
    entryWidgets[btn].second->clear();
}

void MainWindow::handleMapClick(const QString& btn, int index, ClickableLineEdit* entry, QMouseEvent* event) {
    if (capturing && currentEntryWidget) {
        currentEntryWidget->setStyleSheet("background-color: #343638;");
        qApp->removeEventFilter(this);
    }
    capturing = true;
    currentBtn = btn;
    currentIndex = index;
    currentEntryWidget = entry;
    currentEntryWidget->setStyleSheet("background-color: #8B0000;");
    qApp->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (capturing) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            int key = keyEvent->key();
            QString keyName;
            if (key == Qt::Key_Backspace) keyName = "";
            else {
                keyName = QKeySequence(key).toString().toLower();
                if (keyName.contains("ctrl")) keyName = "ctrl";
                else if (keyName.contains("alt")) keyName = "alt";
                else if (keyName.contains("shift")) keyName = "shift";
                else if (key == Qt::Key_Control) keyName = "ctrl";
                else if (key == Qt::Key_Alt) keyName = "alt";
                else if (key == Qt::Key_Shift) keyName = "shift";
            }
            finishCapture(keyName);
            return true;
        } else if (event->type() == QEvent::MouseButtonPress && currentIndex == 1) {
             QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
             QString mouseBtn;
             if (mouseEvent->button() == Qt::LeftButton) mouseBtn = "MOUSE_LEFT";
             else if (mouseEvent->button() == Qt::MiddleButton) mouseBtn = "MOUSE_MIDDLE";
             else if (mouseEvent->button() == Qt::RightButton) mouseBtn = "MOUSE_RIGHT";
             if (!mouseBtn.isEmpty()) { finishCapture(mouseBtn); return true; }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::finishCapture(const QString& keyName) {
    if (!currentEntryWidget) return;
    
    QString finalKey = keyName;
    if (currentIndex == 0 && !finalKey.isEmpty()) {
        if (finalKey != "ctrl" && finalKey != "alt" && finalKey != "shift") {
            std::cout << "[AVISO] Modificador inválido: " << finalKey.toStdString() << " (Use Ctrl, Alt ou Shift)" << std::endl;
            finalKey = ""; // Discard invalid modifier
        }
    }

    currentEntryWidget->setText(finalKey.toUpper());
    currentEntryWidget->setStyleSheet("background-color: #343638;");
    Config::MAP[currentBtn.toStdString()][currentIndex] = finalKey.toStdString();
    std::cout << "[LOG] Mapeamento: " << currentBtn.toStdString() << "[" << currentIndex << "] = " << finalKey.toStdString() << std::endl;
    capturing = false;
    currentEntryWidget = nullptr;
    qApp->removeEventFilter(this);
}

void MainWindow::toggleEmulator() {
    if (!Config::RUNNING) {
        if (!GamepadHandler::is_connected()) {
            std::cout << "[AVISO] Não é possível ativar: Nenhum controle detectado!" << std::endl;
            QMessageBox::warning(this, "Aviso", "Conecte um controle para ativar o emulador.");
            return;
        }
        static bool threads_initialized = false;
        if (!threads_initialized) {
            ActionExecutor::init();
            ActionExecutor::start_threads();
            threads_initialized = true;
        }
        std::cout << "[LOG] Emulador ATIVADO." << std::endl;
        Config::RUNNING = true;
        statusBtn->setText("PARAR");
        statusBtn->setStyleSheet("background-color: red;");
    } else {
        Config::RUNNING = false;
        for (auto& pair : Config::MOVE_AXIS) { pair.second = 0; intercept::key_up(pair.first); }
        std::cout << "[LOG] Emulador DESATIVADO." << std::endl;
        statusBtn->setText("ATIVAR EMULADOR");
        statusBtn->setStyleSheet("background-color: green;");
    }
}

void MainWindow::resetDrivers() {
    intercept::reset();
    QMessageBox::information(this, "Drivers", "Drivers do Interception reiniciados com sucesso!");
}
