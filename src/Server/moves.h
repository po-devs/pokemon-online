#ifndef MOVES_H
#define MOVES_H

#include <QtCore>

/* Used to tell us everything about a move */
struct MoveEffect : public QVariantMap
{
    MoveEffect(int num);
};

#endif // MOVES_H
