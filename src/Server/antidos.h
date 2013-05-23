#ifndef ANTIDOS_H
#define ANTIDOS_H

#include <QWidget>
#include <QHash>
#include <ctime>
#include <QTimer>

class QSpinBox;
class QLineEdit;
class QCheckBox;

class AntiDosWindow : public QWidget
{
    Q_OBJECT
public:
    AntiDosWindow();
public slots:
    void apply();
private:
    QSpinBox *max_people_per_ip, *max_commands_per_user, *max_kb_per_user, *max_login_per_ip, *ban_after_x_kicks;
    QLineEdit *trusted_ips, *notificationsChannel;
    QCheckBox *aDosOn;
};

/* A class to detect flood and ban DoSing IPs */
class AntiDos : public QObject
{
    Q_OBJECT
    friend class AntiDosWindow;
    friend class ScriptEngine;
public:
    AntiDos();

    static AntiDos * obj() {
        return instance;
    }

    /* Returns true if an ip is allowed a new connection */
    bool connecting(const QString &ip);
    /* Warned that a new command is issued, with what length */
    bool transferBegin(int id, int length, const QString &ip);
    /* Warned that a player/IP disconnected */
    void disconnect(const QString &ip, int id);

    /* Changes the shows IP of the proxy using player */
    bool changeIP(const QString &newIp, const QString &oldIp);

    int numberOfDiffIps();

    QString notificationsChannel;
signals:
    /* If rules are infriged, kick / ban the corresponding id/ip in functions
       of the number of times rules are infriged */
    void kick(int id);
    void ban(const QString &ip);
public slots:
    /* Reloads all DOS data */
    void init();
protected slots:
    void clearData();
private:
    QHash<QString, int> connectionsPerIp;
    QHash<QString, QList<time_t> > loginsPerIp;
    QHash<int, QList<QPair<time_t, size_t> > > transfersPerId;
    QHash<int, size_t> sizeOfTransfers;
    QHash<QString, QList<time_t> > kicksPerIp;
    QTimer timer;

    static AntiDos *instance;

    QStringList trusted_ips;
    int max_people_per_ip, max_commands_per_user, max_kb_per_user, max_login_per_ip, ban_after_x_kicks;
    bool on;

    void addKick(const QString &ip);
};

#endif // ANTIDOS_H
