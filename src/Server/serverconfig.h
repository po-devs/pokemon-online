#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QtGui>
#include "security.h"

class ServerWindow : public QWidget
{
    Q_OBJECT
public:
    ServerWindow(QWidget *parent = 0);
signals:
    void privacyChanged(int priv);
    void mainChanChanged(const QString &mainchan);
    void nameChanged(const QString &name);
    void descChanged(const QString &desc);
    void announcementChanged(const QString &ann);
    void maxChanged(int num);
    void logSavingChanged(bool logSaving);
    void useChannelFileLogChanged(bool useChannelFileLog);
    void useBattleFileLogChanged(bool useBattleFileLog);
    void latencyChanged(bool lowDelay);
    void safeScriptsChanged(bool safeScripts);
    void proxyServersChanged(const QString &ips);
    void serverPasswordChanged(const QString &pass);
    void usePasswordChanged(bool usePassword);
    void showTrayPopupChanged(bool show);
    void minimizeToTrayChanged(bool allow);

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
    QCheckBox *battleFileLog;
    QCheckBox *lowLatency;
    QCheckBox *safeScripts;
    QCheckBox *trayPopup;
    QCheckBox *minimizeToTray;
    QLineEdit *proxyServers;
    QLineEdit *serverPassword;
    QCheckBox *usePassword;
};

#endif // SERVERCONFIG_H
