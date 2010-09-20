#ifndef MOVESETCHECKER_H
#define MOVESETCHECKER_H

#include "pokemonstructs.h"
#include <QtCore>

class MoveSetChecker
{
public:
    static void init(const QString &dir);
    static bool isValid(const Pokemon::uniqueId &pokeid, int gen, const QSet<int> &moves, int ability, QSet<int> *invalid_moves=NULL);
    static bool isValid(const Pokemon::uniqueId &pokeid, int gen, int move1,int move2, int move3, int move4, int ability,
                        QSet<int> *invalid_moves=NULL);
    static bool isAnEggMoveCombination(const Pokemon::uniqueId &pokeid, int gen, QSet<int> moves);
    static QList<QSet<int> > combinationsFor(Pokemon::uniqueId pokenum, int gen);
private:
    static QHash<Pokemon::uniqueId, QList<QSet<int> > > legalCombinations[3];
    static QString dir;

    static QString path(const QString &arg);
};

#endif // MOVESETCHECKER_H
