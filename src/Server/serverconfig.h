#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QWidget>
#include "security.h"

class QComboBox;
class QPlainTextEdit;
class QSpinBox;
class QCheckBox;

class ServerWindow : public QWidget
{
    Q_OBJECT
public:
    ServerWindow(QWidget *parent = 0);
signals:
    void privacyChanged(bool priv);
    void mainChanChanged(const QString &mainchan);
    void nameChanged(const QString &name);
    void descChanged(const QString &desc);
    void announcementChanged(const QString &ann);
    void maxChanged(int num);
    void logSavingChanged(bool logSaving);
    void useChannelFileLogChanged(bool useChannelFileLog);
    void inactivePlayersDeleteDaysChanged(int value);
    void latencyChanged(bool lowDelay);
    void safeScriptsChanged(bool safeScripts);
    void overactiveToggleChanged(bool showOveractive);
    void proxyServersChanged(const QString &ips);
    void serverPasswordChanged(const QString &pass);
    void usePasswordChanged(bool usePassword);
    void showTrayPopupChanged(bool show);
    void minimizeToTrayChanged(bool allow);
    void clickConditionChanged(bool click);

private slots:
    void apply();
private:
    QComboBox *serverPrivate;
    QLineEdit *serverName;
    QLineEdit *mainChan;
    QPlainTextEdit *serverDesc;
    QPlainTextEdit *serverAnnouncement;
    QSpinBox *serverPlayerMax;
    QSpinBox *serverPort;
    QCheckBox *saveLogs;
    QCheckBox *channelFileLog;
    QSpinBox *deleteInactive;
    QCheckBox *lowLatency;
    QCheckBox *safeScripts;
    QCheckBox *trayPopup;
    QCheckBox *minimizeToTray;
    QCheckBox *doubleClick;
    QCheckBox *showOveractive;
    QLineEdit *proxyServers;
    QLineEdit *serverPassword;
    QCheckBox *usePassword;
};

#endif // SERVERCONFIG_H
