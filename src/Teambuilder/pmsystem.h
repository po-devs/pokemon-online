#ifndef PMSYSTEM_H
#define PMSYSTEM_H

#include <QtGui>

#include "logmanager.h"
#include "../Utilities/functions.h"
#include "../Utilities/otherwidgets.h"

class QScrollDownTextBrowser;
class QIRCLineEdit;
struct PMStruct;

class PMSystem : public QWidget {
    Q_OBJECT

public:
    PMSystem(bool withTabs);
    ~PMSystem();

    void setServerName(const QString &name);
    bool hasPM(PMStruct *PM);
    void startPM(PMStruct *newPM);
    void checkTabbing();
    void changePMs();

    void flash(PMStruct *pm);
    QHash<int, PMStruct*> myPMWindows;

signals:
    void notification(const QString &title, const QString &message);

private slots:
    void closeTab(int tabNum);
    void tabChanged(int tabNum);
    void togglePMs(bool toggled);
    void messageReceived(PMStruct *pm, const QString &mess);
    void PMDisconnected(bool value);
    void removePM(int pm);
    void changeId(int old, int newid);

private:
    QExposedTabWidget *myPMs;
    bool tabbedPMs;
};

struct PMStruct : public QWidget
{
    Q_OBJECT
    PROPERTY(int, id)

public:
    PMStruct(int id, const QString &ownName, const QString &name, const QString &content = "", int ownAuth=0);
    ~PMStruct() {
        log->close();
        emit destroyed(id(), m_name);
    }

    void changeName(const QString &newname);
    void changeSelf(const QString &newname);
    void printLine(const QString &line, bool self = false);
    void disable();
    void reuse(int id);
    void disconnected(bool value);

    QString name() const {
        return m_name;
    }

    enum State {
        NoMessages,
        NewMessage
    };

    int state;

signals:
    void messageReceived(PMStruct *pm, const QString &mess);
    void messageEntered(int id, const QString &mess);
    void challengeSent(int id);
    void destroyed(int id, QString name);
    void ignore(int id, bool);
    void idChanged(int oldid, int newid);
    void controlPanel(int id);

public slots:
    void sendMessage();
    void ignore(bool);
    void challenge();
    void emitCp();

private:
    QString m_name;
    QString m_ownName;
    bool SaveLog;

    Log *log;

    void printHtml(const QString &htmlCode, bool timestamps = true);

    QScrollDownTextBrowser *m_mainwindow;
    QIRCLineEdit *m_textToSend;
    QPushButton *m_challenge, *m_send;
};

#endif // PMSYSTEM_H
