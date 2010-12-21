#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtCore>
#include "../PokemonInfo/networkstructs.h"

class QNickValidator;
class Player;

struct Channel {
    QString name;
    QSet<Player *> players;
    QHash<int, Battle> battleList;
    QFile logfile;
    int logDay;

    Channel(const QString &name);
    ~Channel();

    /* other properties can be added later,
       such as mode, ops, topic... */

    static bool validName(const QString &name);
    static QNickValidator *checker;

    void log(const QString &message);
};

#endif // CHANNEL_H
