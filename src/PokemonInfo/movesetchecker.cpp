namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include "movesetchecker.h"
#include "pokemoninfo.h"

QHash<Pokemon::uniqueId, QList<QSet<int > > > MoveSetChecker::legalCombinations[NUMBER_GENS];
QHash<Pokemon::uniqueId, QList<QSet<int > > > MoveSetChecker::eventCombinations[NUMBER_GENS];
QHash<Pokemon::uniqueId, QList<QSet<int > > > MoveSetChecker::breedingCombinations[NUMBER_GENS];

static void fill_uid_str(QHash<Pokemon::uniqueId, QString> &container, const QString &filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, PokemonInfoConfig::allFiles(filename, trans)) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream filestream(&file);
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            QString current = filestream.readLine().trimmed();
            QString other_data;
            Pokemon::uniqueId pokeid;
            bool ok = Pokemon::uniqueId::extract(current, pokeid, other_data);
            if(ok) {
                container[pokeid] = other_data;
            }
        }
    }
}

QString MoveSetChecker::dir;
bool MoveSetChecker::enforceMinLevels = true;

QString MoveSetChecker::path(const QString &arg, const Pokemon::gen & g)
{
    if (g != 0) {
        return QString("%1/%2G/%3").arg(dir).arg(g.num).arg(arg);
    } else {
        return dir + arg;
    }
}

void MoveSetChecker::loadCombinations(const QString &file, Pokemon::gen gen, QHash<Pokemon::uniqueId, QList<QSet<int> > > *set)
{
    QHash<Pokemon::uniqueId, QString> temp;
    fill_uid_str(temp, file);

    QHashIterator<Pokemon::uniqueId, QString> it(temp);

    while(it.hasNext()) {
        it.next();

        Pokemon::uniqueId id = it.key();
        /* Even if the hash is empty, it proves it's here, otherwise it would be filled by the data of
           the base forme */
        if (!legalCombinations[gen.num-GEN_MIN].contains(id))
            legalCombinations[gen.num-GEN_MIN].insert(id, QList<QSet<int > >());

        set[gen.num-GEN_MIN][id] = QList<QSet<int > >();

        if (it.value().length() > 0) {
            QStringList combs = it.value().split('|');
            foreach(QString comb, combs) {
                QStringList moves = comb.split(' ');
                QSet<int> toPush;
                foreach(QString move, moves) {
                    toPush.insert(move.toInt());
                }
                legalCombinations[gen.num-GEN_MIN][id].push_back(toPush);
                set[gen.num-GEN_MIN][id].push_back(toPush);
            }
        }
    }
}

void MoveSetChecker::init(const QString &dir, bool enf)
{
    MoveSetChecker::dir = dir;
    MoveSetChecker::enforceMinLevels = enf;

    QList<Pokemon::uniqueId> ids = PokemonInfo::AllIds();

    for(int i = 0; i < NUMBER_GENS; i++) {
        legalCombinations[i].clear();
        breedingCombinations[i].clear();
        eventCombinations[i].clear();
    }

    for (int gen = GEN_MIN; gen <= GEN_MAX; gen++) {
        /* Egg move combinations */
        loadCombinations(path("legal_combinations.txt", gen), gen, breedingCombinations);

        /* Event move combinations */
        loadCombinations(path("event_combinations.txt", gen), gen, eventCombinations);

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

bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int move1, int move2, int move3, int move4, int ability,
                             int gender, int level, bool maledw, QSet<int> *invalid_moves, QString *error) {
    QSet<int> moves;
    moves << move1 << move2 << move3 << move4;

    return isValid(pokeid, gen, moves, ability, gender, level, maledw, invalid_moves, error);
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

bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, const QSet<int> &moves2, int ability, int gender,
                             int level, bool maledw, QSet<int> *invalid_moves, QString *error) {
    QSet<int> moves = moves2;
    moves.remove(0);

    if (!enforceMinLevels)
        level = 100;

    int limit;

    if (gen >= 3)
        limit = 3;
    else
        limit = 1;

    for (int g = gen.num; g >= limit; g--) {
        if (!PokemonInfo::Exists(pokeid, g)) {
            if (PokemonInfo::HasPreEvo(pokeid.pokenum)) {
                return MoveSetChecker::isValid(PokemonInfo::PreEvo(pokeid.pokenum), g, moves, 0, gender, level, maledw,
                                               invalid_moves, error);
            }
            if (invalid_moves) {
                *invalid_moves = moves;
            }
            if (error) {
                *error = QObject::tr("%1 can't learn the following moves while being at level %2: %3.")
                         .arg(PokemonInfo::Name(pokeid), QString::number(level), getCombinationS(moves));
            }
            return false;
        }
        if (PokemonInfo::AbsoluteMinLevel(pokeid, g) > level) {
            if (invalid_moves) {
                *invalid_moves = moves;
            }
            if (error) {
                *error = QObject::tr("%1 can't learn the following moves while being at level %2: %3.")
                         .arg(PokemonInfo::Name(pokeid), QString::number(level), getCombinationS(moves));
            }
            return false;
        }
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

        /* If the pokemon is underleveled, he was caught wild or from a previous gen, anyhow he couldn't have evolved from this gen */
        bool nobreeding = PokemonInfo::MinEggLevel(pokeid, g) > level;

        if (g < 5 && g >= 3) {
            AbilityGroup ab = PokemonInfo::Abilities(pokeid, gen);

            if (ability != 0 && ability != ab.ab(0)) {
                if (ability != ab.ab(1)) {
                    if (invalid_moves) {
                        *invalid_moves = moves;
                    }
                    if (error) {
                        *error = QObject::tr("%1 can't learn the following moves from older generations at the same time as having the ability %2: %3.")
                                .arg(PokemonInfo::Name(pokeid), AbilityInfo::Name(ability), getCombinationS(moves));
                    }
                    return false;
                }

                /* First stage evolutions can't have 4th gen abilities with 3rd gen moves */
                if (g == 3 && gen > 3) {
                    ab = PokemonInfo::Abilities(pokeid, g);
                    if (ab.ab(1) != ability) {
                        if (!PokemonInfo::HasPreEvo(pokeid.pokenum)) {
                            if (invalid_moves) {
                                *invalid_moves = moves;
                            }
                            if (error) {
                                *error = QObject::tr("%1 can't learn the following moves from third generation at the same time as having the fourth generation ability %2: %3.")
                                        .arg(PokemonInfo::Name(pokeid), AbilityInfo::Name(ability), getCombinationS(moves));
                            }
                            return false;
                        }
                        return nobreeding == false &&
                                isValid(PokemonInfo::PreEvo(pokeid.pokenum), g, moves, 0, gender, level, false, invalid_moves, error);
                    }
                }
            }
        }

        if (maledw && gender == Pokemon::Female) {
            return false;
        }

        /* now we know the pokemon at least knows all moves */
        moves.subtract(PokemonInfo::RegularMoves(pokeid, g));

        /* In gen 2 we must allow tradebacks. For that we need movesets without gen 2
           egg moves or special moves */
        if (g == 2) {
            if (PokemonInfo::Exists(pokeid, 1)) {
                bool ok = true;
                foreach(int move, moves) {
                    if (!MoveInfo::Exists(move, 1)) {
                        ok = false;
                        break;
                    }
                }

                if (ok)
                    moves.subtract(PokemonInfo::RegularMoves(pokeid, 1));
            } else if (!nobreeding) {
                int preevo = PokemonInfo::PreEvo(pokeid.pokenum);

                if (preevo != 0 &&  preevo != pokeid.pokenum && PokemonInfo::Exists(preevo, 1)) {
                    bool ok = true;
                    foreach(int move, moves) {
                        if (!MoveInfo::Exists(move, 1)) {
                            ok = false;
                            break;
                        }
                    }

                    if (ok && isValid(preevo, 2, moves))
                        return true;
                }
            }
        }

        if (!nobreeding) {
            /* If there's a pre evo move and an old gen move, you must check the pre evo has the combination in the old gen */
            QSet<int> moves3 = moves;
            moves3.subtract(PokemonInfo::PreEvoMoves(pokeid,g));

            if (moves3.size() != moves.size()) {
                int pokemon = PokemonInfo::PreEvo(pokeid.pokenum);

                int ab2;
                AbilityGroup ab = PokemonInfo::Abilities(pokeid, g);
                if (ability == ab.ab(0)) {
                    ab2 = PokemonInfo::Abilities(pokemon, g).ab(0);
                } else if (ability == ab.ab(1)) {
                    ab2 = PokemonInfo::Abilities(pokemon, g).ab(1);
                } else if (ability == ab.ab(2)) {
                    ab2 = PokemonInfo::Abilities(pokemon, g).ab(2);
                } else {
                    ab2 = 0;
                }
                if (isValid(pokemon, g, moves,ab2,gender,level,maledw))
                    return true;
            }
        }


        if (moves.empty())
            return true;

        if (!maledw) {
            if (moves.size() == 1) {
                if (!nobreeding) {
                    if (PokemonInfo::HasMoveInGen(pokeid, *moves.begin(), g)) {
                        return true;
                    }
                } else {
                    if (PokemonInfo::SpecialMoves(pokeid, g).contains(*moves.begin())) {
                        return true;
                    }
                }
            }

            if (!nobreeding && isAnEggMoveCombination(pokeid, g, moves)) {
                return true;
            }
        }

        if (g == 5) {
            AbilityGroup ab = PokemonInfo::Abilities(pokeid, g);

            if (ability == ab.ab(2)) {
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
bool MoveSetChecker::isAnEggMoveCombination(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, QSet<int> moves)
{
    foreach(QSet<int> combination, legalCombinations[gen.num-GEN_MIN].value(pokeid)) {
        if (combination.contains(moves))
            return true;
    }

    return false;
}

QList<QSet<int> > MoveSetChecker::combinationsFor(Pokemon::uniqueId pokenum, Pokemon::gen gen)
{
    return legalCombinations[gen.num-GEN_MIN].value(pokenum);
}

QHash<Pokemon::uniqueId, QList<QSet<int> > > MoveSetChecker::eventCombinationsOf(Pokemon::gen gen)
{
    return eventCombinations[gen.num-GEN_MIN];
}

QHash<Pokemon::uniqueId, QList<QSet<int> > > MoveSetChecker::breedingCombinationsOf(Pokemon::gen gen)
{
    return breedingCombinations[gen.num-GEN_MIN];
}
