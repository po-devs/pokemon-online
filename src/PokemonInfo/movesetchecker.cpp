#include "movesetchecker.h"
#include "pokemoninfo.h"

QList<QList<QSet<int > > >* MoveSetChecker::legalCombinations[2];
QString MoveSetChecker::dir;

QString MoveSetChecker::path(const QString &arg)
{
    return dir + arg;
}

void MoveSetChecker::init(const QString &dir)
{
    MoveSetChecker::dir = dir;
    legalCombinations[0] = new (QList<QList<QSet<int > > >);
    legalCombinations[1] = new (QList<QList<QSet<int > > >);

    for (int gen = 3; gen <= 4; gen++) {
        QFile in(path("legal_combinations_" + QString::number(gen) + "G.txt"));
        in.open(QIODevice::ReadOnly);
        QList<QByteArray> pokes = in.readAll().split('\n');

        foreach(QByteArray poke, pokes) {
            legalCombinations[gen-3]->push_back(QList<QSet<int> >());
            if (poke.length() > 0) {
                QList<QByteArray> combs = poke.split('|');
                foreach(QByteArray comb, combs) {
                    QList<QByteArray> moves = comb.split(' ');
                    QSet<int> toPush;
                    foreach(QByteArray move, moves) {
                        toPush.insert(move.toInt());
                    }
                    legalCombinations[gen-3]->back().push_back(toPush);
                }
            }
        }
    }
}

bool MoveSetChecker::isValid(int pokenum, int move1, int move2, int move3, int move4, QSet<int> *invalid_moves) {
    QSet<int> moves;
    moves << move1 << move2 << move3 << move4;
    moves.remove(0);

    if (!PokemonInfo::Moves(pokenum).contains(moves)) {
        if (invalid_moves) {
            *invalid_moves = moves.subtract(PokemonInfo::Moves(pokenum));
        }
        return false;
    }

    /* now we know the pokemon at least know all moves */

    moves.subtract(PokemonInfo::RegularMoves(pokenum,4));

    if (moves.empty() || moves.size() == 1)
        return true;

    foreach(QSet<int> combination, (*legalCombinations[4-3])[pokenum]) {
        if (combination.contains(moves))
            return true;
    }

    /* Now we have to go back to third gen, and not use 4 gen egg mvoes / special moves */
    moves.subtract(PokemonInfo::RegularMoves(pokenum,3));

    if (moves.empty())
        return true;
    if (moves.size() == 1) {
        if (PokemonInfo::EggMoves(pokenum,3).contains(*moves.begin()) ||
            PokemonInfo::SpecialMoves(pokenum,3).contains(*moves.begin()))
            return true;
        else {
            if (invalid_moves) {
                *invalid_moves = moves;
            }
            return false;
        }
    }

    foreach(QSet<int> combination, (*legalCombinations[3-3])[pokenum]) {
        if (combination.contains(moves))
            return true;
    }

    if (invalid_moves) {
        *invalid_moves = moves;
    }
    return false;
}
