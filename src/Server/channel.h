#ifndef CHANNEL_H
#define CHANNEL_H

class Player;
template<class T> class QPointer;

unsigned int qHash (const QPointer<Player>&);

#include <QtCore>
#include "../PokemonInfo/networkstructs.h"

class QNickValidator;
class Server;

class Channel : public QObject {
    Q_OBJECT

    PROPERTY(qint32, id);
    PROPERTY(QString, name);

public:
    Channel(const QString &name, int id);
    ~Channel();

    void addBattle(int battleid, const Battle &b);
    void leaveRequest(int pid, bool onlydisconnect);
    void playerJoin(int pid);
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

signals:
    void closeRequest(int id);

public:
    QSet<int> players;
    QSet<int> disconnectedPlayers;
    QHash<int, Battle> battleList;
    Server *server;
};

#endif // CHANNEL_H
