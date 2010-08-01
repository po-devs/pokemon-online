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

    Channel(const QString &name);

    /* other properties can be added later,
       such as mode, ops, topic... */

    static bool validName(const QString &name);
    static QNickValidator *checker;
};

#endif // CHANNEL_H
