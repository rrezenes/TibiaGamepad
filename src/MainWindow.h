#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QSlider>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QMap>
#include <QPair>
#include <QMouseEvent>
#include <QLabel>
#include <QTimer>
#include "LogRedirector.h"

// Custom QLineEdit to detect mouse clicks for mapping capture
class ClickableLineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit ClickableLineEdit(QWidget* parent = nullptr);
signals:
    void clicked(QMouseEvent* event);
protected:
    void mousePressEvent(QMouseEvent* event) override;
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Global event filter for key and mouse capture
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void updateUiFromConfig();
    void toggleEmulator();
    void clearLogs();
    void handleMapClick(const QString& btn, int index, ClickableLineEdit* entry, QMouseEvent* event);
    void clearRow(const QString& btn);
    void saveProfile();
    void updatePeripheralStatus();
    void resetButtonColor();
    void resetDrivers();

private:
    void setupGeneralTab(QWidget* tab);
    void setupButtonsTab(QWidget* tab);
    void setupLogsTab(QWidget* tab);
    void finishCapture(const QString& keyName);

    QTabWidget* tabView;
    QPushButton* statusBtn;
    QPushButton* resetBtn;
    QSlider* sMouse;
    QButtonGroup* stickGroup;
    QRadioButton* rbLeft;
    QRadioButton* rbRight;
    QTextEdit* logTextEdit;

    QMap<QString, QPair<ClickableLineEdit*, ClickableLineEdit*>> entryWidgets;

    bool capturing = false;
    QString currentBtn;
    int currentIndex = -1;
    ClickableLineEdit* currentEntryWidget = nullptr;
    LogRedirector* logRedirector;

    // Status indicators
    QLabel* gamepadStatus = nullptr;
    QLabel* keyboardStatus = nullptr;
    QLabel* mouseStatus = nullptr;
    
    QTimer* statusTimer = nullptr;
    QTimer* animationTimer = nullptr;
    QPushButton* lastSaveBtn = nullptr;
};
