#ifndef ANTIDOS_H
#define ANTIDOS_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QList>
#include <QTimer>
#include <QStringList>

class QSettings;

/* A class to detect flood and ban DoSing IPs */
class AntiDos : public QObject
{
    Q_OBJECT
    friend class AntiDosWindow;
public:
    AntiDos(QSettings &settings);

    static AntiDos * obj() {
        return instance;
    }

    static void init(QSettings &settings);

    /* Returns true if an ip is allowed a new connection */
    bool connecting(const QString &ip);
    /* Warned that a new command is issued, with what length */
    bool transferBegin(int id, int length, const QString &ip);
    /* Warned that a player/IP disconnected */
    void disconnect(const QString &ip, int id);

    /* Changes the shows IP of the proxy using player */
    bool changeIP(const QString &newIp, const QString &oldIp);

    void clearIP(const QString &ip);

    int connections(const QString &ip);

    int numberOfDiffIps();

    QString dump() const;

    QString notificationsChannel;
signals:
    /* If rules are infriged, kick / ban the corresponding id/ip in functions
       of the number of times rules are infriged */
    void kick(int id);
    void ban(const QString &ip);
public slots:
    /* Reloads all DOS data */
    void loadVals(QSettings &settings);
    /* Clears data stored */
    void clearData();
private:
    QHash<QString, int> connectionsPerIp;
    QHash<QString, QList<time_t> > loginsPerIp;
    QHash<int, QList<QPair<time_t, size_t> > > transfersPerId;
    QHash<int, int> sizeOfTransfers;
    QHash<QString, QList<time_t> > kicksPerIp;
    QTimer timer;
    static AntiDos *instance;

    QStringList trusted_ips;
    int max_people_per_ip, max_commands_per_user, max_kb_per_user, max_login_per_ip, ban_after_x_kicks;
    bool on;

    void addKick(const QString &ip);
};
#endif // ANTIDOS_H
