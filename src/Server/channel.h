#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtCore>

struct Channel {
    QString name;
    QSet<int> players;

    /* other properties can be added later,
       such as mode, ops, topic... */
};

#endif // CHANNEL_H
