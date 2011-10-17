#ifndef PMWINDOW_H
#define PMWINDOW_H

#include <QtGui>
#include "../Utilities/functions.h"

class QScrollDownTextBrowser;
class QIRCLineEdit;

struct PMWindow : public QWidget
{
    Q_OBJECT
    PROPERTY(int, id);
public:
    PMWindow(int id, const QString &ownName, const QString &name, const QString &content = "", bool html = false);
    ~PMWindow() {
        emit destroyed(id());
    }

    void changeName(const QString &newname);
    void changeSelf(const QString &newname);
    void printLine(QString line, bool self = false);
    void disable();
signals:
    void messageEntered(int id, const QString &mess);
    void challengeSent(int id);
    void destroyed(int id);
    void ignore(int id, bool);
public slots:
    void sendMessage();
    void ignore(bool);
private:
    QString m_name;
    QString m_ownName;
    bool escape_html;
    QString name() const {
        return m_name;
    }

    void printHtml(const QString &htmlCode, bool timestamps=true);

    QScrollDownTextBrowser *m_mainwindow;
    QIRCLineEdit *m_textToSend;
    QPushButton *m_challenge, *m_send;
};

#endif // PMWINDOW_H
