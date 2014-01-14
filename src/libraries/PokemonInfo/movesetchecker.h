#ifndef MOVESETCHECKER_H
#define MOVESETCHECKER_H

#include "pokemonstructs.h"
#include <QtCore>

class MoveSetChecker
{
public:
    static void init(const QString &dir="db/pokes/", bool enforceMinLevels = true);
    static void loadCombinations(const QString &file, Pokemon::gen gen, QHash<Pokemon::gen, QHash<Pokemon::uniqueId, QList<QSet<int> > > > &set);
    static bool isValid(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, const QSet<int> &moves, int ability = 0, int gender = 0,
                        int level=100, bool maledw = false, QSet<int> *invalid_moves=NULL, QString *error = NULL);
    static bool isValid(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int move1,int move2, int move3, int move4, int ability = 0, int gender = 0,
                        int level = 100, bool maledw = false,
                        QSet<int> *invalid_moves=NULL, QString *error = NULL);
    static bool isAnEggMoveCombination(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, QSet<int> moves);
    static QList<QSet<int> > combinationsFor(Pokemon::uniqueId pokenum, Pokemon::gen gen);
    static QHash<Pokemon::uniqueId, QList<QSet<int> > > eventCombinationsOf(Pokemon::gen gen);
    static QHash<Pokemon::uniqueId, QList<QSet<int> > > breedingCombinationsOf(Pokemon::gen gen);

    static bool enforceMinLevels;
private:
    static QHash<Pokemon::gen, QHash<Pokemon::uniqueId, QList<QSet<int> > > > legalCombinations, breedingCombinations, eventCombinations;

    static QString dir;

    static void loadGenData(const Pokemon::gen &g);
    static QString path(const QString &arg, const Pokemon::gen &g);
};

#endif // MOVESETCHECKER_H
