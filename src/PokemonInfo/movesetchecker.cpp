namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include "movesetchecker.h"
#include "pokemoninfo.h"

QHash<Pokemon::uniqueId, QList<QSet<int > > > MoveSetChecker::legalCombinations[NUMBER_GENS];
QString MoveSetChecker::dir;

QString MoveSetChecker::path(const QString &arg)
{
    return dir + arg;
}

void MoveSetChecker::init(const QString &dir)
{
    MoveSetChecker::dir = dir;

    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();

    for (int gen = GEN_MIN; gen <= GEN_MAX; gen++) {
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
            legalCombinations[gen-GEN_MIN].insert(id, QList<QSet<int > >());
            if (data.length() > 0) {
                QStringList combs = data.split('|');
                foreach(QString comb, combs) {
                    QStringList moves = comb.split(' ');
                    QSet<int> toPush;
                    foreach(QString move, moves) {
                        toPush.insert(move.toInt());
                    }
                    legalCombinations[gen-GEN_MIN][id].push_back(toPush);
                }
            }
        }

        foreach(Pokemon::uniqueId id, ids) {
            if (!PokemonInfo::IsForme(id))
                continue;
            if (!legalCombinations[gen-GEN_MIN].contains(PokemonInfo::OriginalForme(id)))
                continue;
            if (legalCombinations[gen-GEN_MIN].contains(id))
                continue;
            legalCombinations[gen-GEN_MIN][id] = legalCombinations[gen-GEN_MIN][PokemonInfo::OriginalForme(id)];
        }
    }
}

bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, int gen, int move1, int move2, int move3, int move4, int ability,
                             int gender, bool maledw, QSet<int> *invalid_moves, QString *error) {
    QSet<int> moves;
    moves << move1 << move2 << move3 << move4;

    return isValid(pokeid, gen, moves, ability, gender, maledw, invalid_moves, error);
}

static QString getCombinationS(const QSet<int> &invalid_moves) {
    QString s;
    bool comma(false);
    foreach(int move, invalid_moves) {
        if (comma)
            s += ", ";
        comma = true;
        s += MoveInfo::Name(move);
    }
    return s;
}

bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, int gen, const QSet<int> &moves2, int ability, int gender,
                             bool maledw, QSet<int> *invalid_moves, QString *error) {
    QSet<int> moves = moves2;
    moves.remove(0);

    for (int g = gen; g >= GEN_MIN; g--) {
        if (!PokemonInfo::Moves(pokeid, g).contains(moves)) {
            moves.subtract(PokemonInfo::Moves(pokeid, g));
            if (invalid_moves) {
                *invalid_moves = moves;
            }
            if (error) {
                *error = QObject::tr("%1 can't learn the following moves: %2.")
                         .arg(PokemonInfo::Name(pokeid), getCombinationS(moves));
            }
            return false;
        }
        if (g < 5) {
            AbilityGroup ab = PokemonInfo::Abilities(pokeid);

            if (ability != ab.ab(0) && ability != ab.ab(1) && ability != 0) {
                if (invalid_moves) {
                    *invalid_moves = moves;
                }
                if (error) {
                    *error = QObject::tr("%1 can't learn the following moves from older generations at the same time as having the Dream World ability %2: %3.")
                             .arg(PokemonInfo::Name(pokeid), AbilityInfo::Name(ability), getCombinationS(moves));
                }
                return false;
            }
        }

        if (maledw && gender == Pokemon::Female) {
            return false;
        }

        /* now we know the pokemon at least knows all moves */
        moves.subtract(PokemonInfo::RegularMoves(pokeid, g));

        /* If there's a pre evo move and an old gen move, you must check the pre evo has the combination in the old gen */
        QSet<int> moves3 = moves;
        moves3.subtract(PokemonInfo::PreEvoMoves(pokeid,g));

        if (moves3.size() != moves.size()) {
            if (moves3.empty() || (moves3.size() == 1 && PokemonInfo::HasMoveInGen(pokeid, *moves3.begin(), g)))
                return true;

            int pokemon = PokemonInfo::PreEvo(pokeid.pokenum);

            if (isValid(pokemon, g, moves,ability,gender,maledw))
                return true;
        }


        if (moves.empty())
            return true;

        if (!maledw) {
            if (moves.size() == 1 && PokemonInfo::HasMoveInGen(pokeid, *moves.begin(), g)) {
                return true;
            }

            if (isAnEggMoveCombination(pokeid, g, moves)) {
                return true;
            }
        }



        if (g == 5) {
            AbilityGroup ab = PokemonInfo::Abilities(pokeid);

            if (ability == ab.ab(2) && (gender != Pokemon::Female || PokemonInfo::Gender(pokeid) == Pokemon::FemaleAvail)) {
                if (moves.size() == 1 && PokemonInfo::dreamWorldMoves(pokeid).contains(moves)) {
                    return true;
                }
            }
        }

        foreach(int move, moves) {
            if (g > 3 && MoveInfo::isHM(move, g-1)) {
                if (error) {
                    *error = QObject::tr("%1 can't have HM %2 inherited from past generations.").arg(PokemonInfo::Name(pokeid), MoveInfo::Name(move));
                }

                return false;
            }
        }
    }

    /* The remaining moves are considered invalid */
    if (invalid_moves) {
        *invalid_moves = moves;
    }
    if (error) {
        if (moves.size() == 1) {
            *error = QObject::tr("%1 can't learn %2 with moves from older generations.").arg(PokemonInfo::Name(pokeid), MoveInfo::Name(*moves.begin()));
        } else {
            *error = QObject::tr("%1 can't learn the following move combination: %2.").arg(PokemonInfo::Name(pokeid), getCombinationS(moves));
        }
    }

    return false;
}

/* Used by ChainBreeding */
bool MoveSetChecker::isAnEggMoveCombination(const Pokemon::uniqueId &pokeid, int gen, QSet<int> moves)
{
    foreach(QSet<int> combination, legalCombinations[gen-GEN_MIN].value(pokeid)) {
        if (combination.contains(moves))
            return true;
    }

    return false;
}

QList<QSet<int> > MoveSetChecker::combinationsFor(Pokemon::uniqueId pokenum, int gen)
{
    return legalCombinations[gen-GEN_MIN].value(pokenum);
}
