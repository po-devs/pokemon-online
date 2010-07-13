#ifndef MOVESETCHECKER_H
#define MOVESETCHECKER_H

#include "pokemonstructs.h"
#include <QtCore>

class MoveSetChecker
{
    static QList<QList<QSet<int > > > *legalCombinations[2];
    static QString dir;

    static QString path(const QString &arg);
public:
    static void init(const QString &dir);
    static bool isValid(int pokenum, const QSet<int> &moves, QSet<int> *invalid_moves=NULL);
    static bool isValid(int pokenum, int move1,int move2, int move3, int move4, QSet<int> *invalid_moves=NULL);
    static bool isAnEggMoveCombination(int pokenum, int gen, QSet<int> moves);
};

#endif // MOVESETCHECKER_H
