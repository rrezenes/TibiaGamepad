#pragma once
#include <iostream>
#include <QTextEdit>
#include <QScrollBar>
#include <QMetaObject>
#include <QString>

class LogRedirector : public std::basic_streambuf<char> {
public:
    LogRedirector(std::ostream& stream, QTextEdit* text_edit)
        : m_stream(stream), m_text_edit(text_edit) {
        m_old_buf = stream.rdbuf();
        stream.rdbuf(this);
    }

    ~LogRedirector() {
        m_stream.rdbuf(m_old_buf);
    }

protected:
    virtual int_type overflow(int_type v) override {
        if (v == '\n') {
            QString text = QString::fromStdString(m_buffer);
            // Safely execute the UI update on the main thread
            QMetaObject::invokeMethod(m_text_edit, [this, text]() {
                m_text_edit->append(text);
                QScrollBar* sb = m_text_edit->verticalScrollBar();
                sb->setValue(sb->maximum());
            }, Qt::QueuedConnection);
            m_buffer.clear();
        } else {
            m_buffer.push_back(static_cast<char>(v));
        }
        return v;
    }

    virtual std::streamsize xsputn(const char *p, std::streamsize n) override {
        for (std::streamsize i = 0; i < n; ++i) {
            overflow(p[i]);
        }
        return n;
    }

private:
    std::ostream& m_stream;
    std::streambuf* m_old_buf;
    QTextEdit* m_text_edit;
    std::string m_buffer;
};
