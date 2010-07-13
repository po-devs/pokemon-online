#ifndef ANTIDOS_H
#define ANTIDOS_H

#include <QtCore>
#include <ctime>


/* A class to detect flood and ban DoSing IPs */
class AntiDos : public QObject
{
    Q_OBJECT
    friend class AntiDosWindow;
public:
    AntiDos();

    static AntiDos * obj() {
        return instance;
    }

    void init();
    /* Returns true if an ip is allowed a new connection */
    bool connecting(const QString &ip);
    /* Warned that a new command is issued, with what length */
    bool transferBegin(int id, int length, const QString &ip);
    /* Warned that a player/IP disconnected */
    void disconnect(const QString &ip, int id);
signals:
    /* If rules are infriged, kick / ban the corresponding id/ip in functions
       of the number of times rules are infriged */
    void kick(int id);
    void ban(const QString &ip);
private:
    QHash<QString, int> connectionsPerIp;
    QHash<QString, QList<time_t> > loginsPerIp;
    QHash<int, QList<QPair<time_t, size_t> > > transfersPerId;
    QHash<int, size_t> sizeOfTransfers;
    QHash<QString, QList<time_t> > kicksPerIp;

    static AntiDos *instance;

    int max_people_per_ip, max_commands_per_user, max_kb_per_user, max_login_per_ip, ban_after_x_kicks;

    void addKick(const QString &ip);
};

#endif // ANTIDOS_H
