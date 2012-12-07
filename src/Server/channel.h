#ifndef CHANNEL_H
#define CHANNEL_H

class Player;
template<class T> class QPointer;

unsigned int qHash (const QPointer<Player>&);

#include <QtCore>
#include "../PokemonInfo/networkstructs.h"

class QNickValidator;

struct Channel {
    QString name;
    QSet<Player *> players;
    QSet<QPointer<Player> > disconnectedPlayers;
    QHash<int, Battle> battleList;
    QFile logfile;
    int logDay;
    int id;

    Channel(const QString &name, int id);
    ~Channel();

    /* other properties can be added later,
       such as mode, ops, topic... */

    static bool validName(const QString &name);
    static QNickValidator *checker;

    void log(const QString &message);
};

#endif // CHANNEL_H
