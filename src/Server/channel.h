#ifndef CHANNEL_H
#define CHANNEL_H

class Player;
template<class T> class QPointer;

unsigned int qHash (const QPointer<Player>&);

#include <QtCore>
#include "../PokemonInfo/networkstructs.h"

class QNickValidator;

class Channel : public QObject {
    Q_OBJECT

    PROPERTY(qint32, id);
    PROPERTY(QString, name);

public:
    Channel(const QString &name, int id);
    ~Channel();

    void addBattle(int battleid, const Battle &b);
    void leaveRequest(Player *p, bool onlydisconnect);
    void playerJoin(Player *p);

    /* other properties can be added later,
       such as mode, ops, topic... */

    static bool validName(const QString &name);
    static QNickValidator *checker;

    void log(const QString &message);

signals:
    void closeRequest(int id);

public:
    QSet<Player *> players;
    QSet<QPointer<Player> > disconnectedPlayers;
    QHash<int, Battle> battleList;
    QFile logfile;
    int logDay;
};

#endif // CHANNEL_H
