#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtCore>
#include <PokemonInfo/networkstructs.h>

class QNickValidator;
class Server;
class Player;

class Channel : public QObject {
    Q_OBJECT

    PROPERTY(qint32, id);
    PROPERTY(QString, name);

public:
    Channel(const QString &name, int id);
    ~Channel();

    void addBattle(int battleid, const Battle &b);
    void leaveRequest(int pid);
    void playerJoin(int pid);
    void addDisconnectedPlayer(int pid);
    /* When a player is reconnecting, even though he never appeared offline.
     * We resend the data to him */
    void onReconnect(int pid);

    /* other properties can be added later,
       such as mode, ops, topic... */

    static bool validName(const QString &name);
    static QNickValidator *checker;

    void log(const QString &message);
    void onRemoval();

    void warnAboutRemoval();

    bool isEmpty() const;
    int count() const;

signals:
    void closeRequest(int id);

private:
    void addBattles(Player *p);
    void removeBattles(Player *p);
    void notifyJoin(int pid);
    void notifyLeave(int pid);
public:
    QSet<int> players;
    QSet<int> disconnectedPlayers;
    QHash<int, Battle> battleList;
    Server *server;
};

#endif // CHANNEL_H
