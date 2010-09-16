namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include "movesetchecker.h"
#include "pokemoninfo.h"

QHash<Pokemon::uniqueId, QList<QSet<int > > > MoveSetChecker::legalCombinations[3];
QString MoveSetChecker::dir;

QString MoveSetChecker::path(const QString &arg)
{
    return dir + arg;
}

void MoveSetChecker::init(const QString &dir)
{
    MoveSetChecker::dir = dir;

    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();

    for (int gen = 3; gen <= 5; gen++) {
        QFile in(path("legal_combinations_" + QString::number(gen) + "G.txt"));
        in.open(QIODevice::ReadOnly);
        QList<QByteArray> pokes = in.readAll().split('\n');

        foreach(QByteArray poke, pokes) {
            Pokemon::uniqueId id;
            QString data;
            if (!id.extract(QString(poke), id, data))
                continue;
            /* Even if the hash is empty, it proves it's here, otherwise it would be filled by the data of
               the base forme */
            legalCombinations[gen-3].insert(id, QList<QSet<int > >());
            if (data.length() > 0) {
                QStringList combs = data.split('|');
                foreach(QString comb, combs) {
                    QStringList moves = comb.split(' ');
                    QSet<int> toPush;
                    foreach(QString move, moves) {
                        toPush.insert(move.toInt());
                    }
                    legalCombinations[gen-3][id].push_back(toPush);
                }
            }
        }

        foreach(Pokemon::uniqueId id, ids) {
            if (!PokemonInfo::IsForme(id))
                continue;
            if (!legalCombinations[gen-3].contains(PokemonInfo::OriginalForme(id)))
                continue;
            if (legalCombinations[gen-3].contains(id))
                continue;
            legalCombinations[gen-3][id] = legalCombinations[gen-3][PokemonInfo::OriginalForme(id)];
        }
    }
}

bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, int gen, int move1, int move2, int move3, int move4, QSet<int> *invalid_moves) {
    QSet<int> moves;
    moves << move1 << move2 << move3 << move4;

    return isValid(pokeid, gen, moves, invalid_moves);
}


bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, int gen, const QSet<int> &moves2, QSet<int> *invalid_moves) {
    QSet<int> moves = moves2;
    moves.remove(0);

    if (!PokemonInfo::Moves(pokeid, gen).contains(moves)) {
        if (invalid_moves) {
            *invalid_moves = moves.subtract(PokemonInfo::Moves(pokeid, gen));
        }
        return false;
    }

    if (gen == 3)
        goto gen3;

    /* now we know the pokemon at least know all moves */

    moves.subtract(PokemonInfo::RegularMoves(pokeid,4));

    if (moves.empty() || moves.size() == 1)
        return true;

    if (isAnEggMoveCombination(pokeid, 4, moves)) {
            return true;
    }

gen3:
    /* Now we have to go back to third gen, and not use 4 gen egg mvoes / special moves */
    moves.subtract(PokemonInfo::RegularMoves(pokeid,3));

    if (moves.empty())
        return true;
    if (moves.size() == 1) {
        if (PokemonInfo::EggMoves(pokeid,3).contains(*moves.begin()) ||
            PokemonInfo::SpecialMoves(pokeid,3).contains(*moves.begin()))
            return true;
        else {
            if (invalid_moves) {
                *invalid_moves = moves;
            }
            return false;
        }
    }

    if (isAnEggMoveCombination(pokeid, 3, moves)) {
            return true;
    }

    if (invalid_moves) {
        *invalid_moves = moves;
    }
    return false;
}

/* Used by ChainBreeding */
bool MoveSetChecker::isAnEggMoveCombination(const Pokemon::uniqueId &pokeid, int gen, QSet<int> moves)
{
    foreach(QSet<int> combination, legalCombinations[gen-3].value(pokeid)) {
        if (combination.contains(moves))
            return true;
    }

    return false;
}

QList<QSet<int> > MoveSetChecker::combinationsFor(Pokemon::uniqueId pokenum, int gen)
{
    return legalCombinations[gen-3].value(pokenum);
}
