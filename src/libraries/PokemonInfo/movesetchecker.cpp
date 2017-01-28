#include "movesetchecker.h"
#include "pokemoninfo.h"

QHash<Pokemon::gen, QHash<Pokemon::uniqueId, QList<QSet<int > > > > MoveSetChecker::legalCombinations;
QHash<Pokemon::gen, QHash<Pokemon::uniqueId, QList<QSet<int > > > > MoveSetChecker::eventCombinations;
QHash<Pokemon::gen, QHash<Pokemon::uniqueId, QList<QSet<int > > > > MoveSetChecker::breedingCombinations;

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
    if (g.subnum == static_cast<decltype(g.subnum)>(-1)) {
        return QString("%1/%2G/%3").arg(dir).arg(g.num).arg(arg);
    } else {
        return QString("%1/%2G/Subgen %3/%4").arg(dir).arg(g.num).arg(g.subnum).arg(arg);
    }
}

void MoveSetChecker::loadCombinations(const QString &file, Pokemon::gen gen, QHash<Pokemon::gen, QHash<Pokemon::uniqueId, QList<QSet<int> > > > &set)
{
    QHash<Pokemon::uniqueId, QString> temp;
    fill_uid_str(temp, file);

    QHashIterator<Pokemon::uniqueId, QString> it(temp);

    while(it.hasNext()) {
        it.next();

        Pokemon::uniqueId id = it.key();
        /* Even if the hash is empty, it proves it's here, otherwise it would be filled by the data of
           the base forme */
        if (!legalCombinations[gen].contains(id))
            legalCombinations[gen].insert(id, QList<QSet<int > >());

        set[gen][id] = QList<QSet<int > >();

        if (it.value().length() > 0) {
            QStringList combs = it.value().split('|');
            foreach(QString comb, combs) {
                QStringList moves = comb.split(' ');
                QSet<int> toPush;
                foreach(QString move, moves) {
                    toPush.insert(move.toInt());
                }
                legalCombinations[gen][id].push_back(toPush);
                set[gen][id].push_back(toPush);
            }
        }
    }
}

void MoveSetChecker::init(const QString &dir, bool enf)
{
    MoveSetChecker::dir = dir;
    MoveSetChecker::enforceMinLevels = enf;

    legalCombinations.clear();
    breedingCombinations.clear();
    eventCombinations.clear();

    for (int i = GEN_MIN; i <= GenInfo::GenMax(); i++) {
        //Load only the whole gen for now, will load subgens on the fly when needed
        loadGenData( Pokemon::gen(i,-1) ); // -1 for "whole gen"

        /* Server loads Everything */
        if (PokemonInfoConfig::getFillMode() == FillMode::Server) {
            for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
                loadGenData( Pokemon::gen(i,j) );
            }
        }
    }
}

void MoveSetChecker::loadGenData(const Pokemon::gen &g)
{
    /* Egg move combinations */
    loadCombinations(path("legal_combinations.txt", g), g, breedingCombinations);

    /* Event move combinations */
    loadCombinations(path("event_combinations.txt", g), g, eventCombinations);

    QHash<Pokemon::uniqueId, QList<QSet<int> > > &legal = legalCombinations[g];

    foreach(Pokemon::uniqueId id, legal.keys()) {
        if (PokemonInfo::IsForme(id))
            continue;

        if (!PokemonInfo::HasFormes(id))
            continue;

        foreach (Pokemon::uniqueId forme, PokemonInfo::Formes(id, g)) {
            if (!forme.isForme() || legal.contains(forme)) {
                continue;
            }

            legal[forme] = legal[id];
        }
    }
}

bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int move1, int move2, int move3, int move4, int ability,
                             int gender, int level, bool maledw, QSet<int> *invalid_moves, QString *error, int minGen) {
    QSet<int> moves;
    moves << move1 << move2 << move3 << move4;

    return isValid(pokeid, gen, moves, ability, gender, level, maledw, invalid_moves, error, minGen);
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

/**
 * How a validity check is performed.
 *
 * Params: a list of moves, an ability, a gender, a level, and a gen and a pokemon
 * level is irrelevant if minimum levels not enforced
 * maledw means "male dreamworld"
 *
 * So if the pokemon is gen 1~2, we do a check on both gens. If a pokemon
 * is in gen 3+, we do a check on each gen from gen X to gen 3 (removing
 * level moves & tutor moves of the current gen everytime).
 *
 * Of course if the pokemon already has a valid moveset at gen X, no need
 * to go further, the function returns immediately.
 *
 * If a pokemon doesn't exist at say gen X-1 but has a pre evo that exists
 * for that gen, then that preevo's moveset is tested for those gens.
 *
 * Legal event move combinations are already determined (check the EventCombinations
 * project), and breeding combinations are generated automatically beforehand as well
 * (check the ChainBreeding project)
 *
 *
 * Basically, every "run" (aka everytime it's called, recursively or not), the program
 * first removes the standard moves (level, tutor, TM/HM) as they are compatible with anything
 *
 * Then it checks the remaining moves. If there's only one, then it's good. If there's more than
 * one, then it checks if it's a legal combination (event or breeding). If it is, it's good.
 *
 * Then there are the complications. If there are preevo moves in the mix, it tries calling itself
 * recursively on the preevo. If we don't have any success this gen, we try going a gen back.
 *
 * There are many special cases in there. For example dealing with HMs, or
 * 4th gen evos with 3rd gen moves. But there should be a comment everytime for
 * those exceptions in the code below. */
bool MoveSetChecker::isValid(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, const QSet<int> &moves2, int ability, int gender,
                             int level, bool maledw, QSet<int> *invalid_moves, QString *error, int minGen)
{
    if (gen == Gen::StadiumWithTradebacks) {
        foreach(int move, moves2) {
            if (!MoveInfo::Exists(move, gen)) {
                if (invalid_moves) {
                    invalid_moves->insert(move);
                }
                if (error) {
                    *error = (QObject::tr("%1 can't learn %2.").arg(PokemonInfo::Name(pokeid), MoveInfo::Name(move)));
                }
                return false;
            }
        }

        return isValid(pokeid, Pokemon::gen(2,-1), moves2, ability, gender, level, maledw, invalid_moves, error);
    }

    /* Last Gen = Whole gen */
    if (gen.subnum == GenInfo::NumberOfSubgens(gen.num) -1) {
        gen.subnum = -1;
    }

    QSet<int> moves = moves2;
    moves.remove(0);

    if (!enforceMinLevels)
        level = 100;

    int limit;

    if (gen >= 3)
        limit = std::max(3, minGen);
    else
        limit = std::max(1, minGen);

    for (Pokemon::gen g = gen; g >= limit; g = Pokemon::gen(g.num-1, -1)) {
        if (!PokemonInfo::Exists(pokeid, g)) {
            if(PokemonInfo::IsMegaEvo(pokeid) || (PokemonInfo::OriginalForme(pokeid) == Pokemon::Arceus && PokemonInfo::IsForme(pokeid))) {
                return MoveSetChecker::isValid(PokemonInfo::OriginalForme(pokeid), g, moves, 0, gender, level, maledw,
                                               invalid_moves, error);
            } else if (PokemonInfo::HasPreEvo(pokeid.pokenum)) {
                return MoveSetChecker::isValid(PokemonInfo::PreEvo(pokeid), g, moves, 0, gender, level, maledw,
                                               invalid_moves, error);
            }
            if (invalid_moves) {
                *invalid_moves = moves;
            }
            if (error) {
                *error = QObject::tr("%1 can't learn the following moves: %2.")
                         .arg(PokemonInfo::Name(pokeid), getCombinationS(moves));
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

        if (g >= 3 && ability != 0) {
            AbilityGroup ab = PokemonInfo::Abilities(pokeid, gen);

            if (!PokemonInfo::Abilities(pokeid, g).contains(ability)) {
                /* First stage evolutions can't have 4th gen abilities with 3rd gen moves, except gen 6 */
                if (ability == ab.ab(1) && gen < 6) {
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
                            isValid(PokemonInfo::PreEvo(pokeid), g, moves, 0, gender, level, false, invalid_moves, error);
                }
                if(gen < 6) {
                    if (invalid_moves) {
                        *invalid_moves = moves;
                    }
                    if (error) {
                        *error = QObject::tr("%1 can't learn the following moves from older generations at the same time as having the ability %2: %3.")
                                .arg(PokemonInfo::Name(pokeid), AbilityInfo::Name(ability), getCombinationS(moves));
                    }
                    return false;
                }
                else if(ab.ab(2) == ability) {
                    /* Some Pokemon change hidden abilities from gen 5 to 6 */
                    int priorHA = PokemonInfo::Abilities(pokeid, g).ab(2);
                    if(priorHA != 0 && priorHA != ability)
                    {
                        return isValid(pokeid, g, moves, priorHA, gender, level, false, invalid_moves, error);
                    }
                    else {
                        if (invalid_moves) {
                            *invalid_moves = moves;
                        }
                        if (error) {
                            *error = QObject::tr("%1 can't learn the following moves from older generations at the same time as having the ability %2: %3.")
                                    .arg(PokemonInfo::Name(pokeid), AbilityInfo::Name(ability), getCombinationS(moves));
                        }
                        return false;
                    }
                }
            }
        }

        if (maledw && gender == Pokemon::Female) {
            return false;
        }

        /* For moves that can only be learned by a previous evolution
           at a higher level than the Pokemon evolved */
        if (g < Pokemon::gen(1, 3)) {
            /* const QHash<QString, QHash<QString, QList<QString> > > invalidCombinations = {{"Kakuna", {{"Harden", {"Poison Sting", "String Shot"}}}}, {"Beedrill", {{"Harden", {"Poison Sting", "String Shot"}}}}, {"Nidoking", {{"Thrash", {"Fury Attack"}}}}, {"Rapidash", {{"Pay Day", {"Tail Whip", "Growl"}}}},
                                                                                        {"Exeggutor", {{"Stomp", {"Sleep Powder", "Stun Spore", "Poison Powder"}}}}, {"Fearow", {{"Pay Day", {"Peck"}}}}, {"Jolteon", {{"Focus Energy", {"Thunder Shock"}}}}, {"Flareon", {{"Focus Energy", {"Ember"}}}}, {"Mewtwo", {{"Confusion", {}}, {"Disable", {}}}}}; */
            QHash<QString, QHash<QString, QList<QString> > > invalidCombinations;
            rbyInvalidCombinations(&invalidCombinations);
            if (invalidCombinations.contains(PokemonInfo::Name(pokeid))) {
                foreach(int moveA, moves) {
                    if (invalidCombinations.value(PokemonInfo::Name(pokeid)).contains(MoveInfo::Name(moveA))) {
                        if (invalidCombinations.value(PokemonInfo::Name(pokeid)).value(MoveInfo::Name(moveA)).isEmpty()) {
                            if (invalid_moves) {
                                invalid_moves->insert(moveA);
                            }
                            if (error) {
                                *error = QObject::tr("%1 can't learn %2.")
                                         .arg(PokemonInfo::Name(pokeid), MoveInfo::Name(moveA));
                            }
                            return false;
                        } else {
                            foreach(QString moveB, invalidCombinations.value(PokemonInfo::Name(pokeid)).value(MoveInfo::Name(moveA))) {
                                if (moves.contains(MoveInfo::Number(moveB))) {
                                    if (invalid_moves) {
                                        invalid_moves->insert(moveA);
                                    }
                                    if (error) {
                                        *error = QObject::tr("%1 can't learn both %2 and %3.")
                                                 .arg(PokemonInfo::Name(pokeid), MoveInfo::Name(moveA), moveB);
                                    }
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* now we know the pokemon at least knows all moves */
        moves.subtract(PokemonInfo::RegularMoves(pokeid, g));

        /* In gen 2 we must allow tradebacks. For that we need movesets without gen 2
           egg moves or special moves */
        if (g.num == 2) {
            if (PokemonInfo::Exists(pokeid, 1)) {
                bool ok = true;
                foreach(int move, moves) {
                    if (!MoveInfo::Exists(move, 1)) {
                        ok = false;
                        break;
                    }
                }

                if (ok) {
                    moves.subtract(PokemonInfo::RegularMoves(pokeid, Pokemon::gen(1, Pokemon::gen::wholeGen)));

                    /* Special case: Pikachu has a pre-evo in gen 2, and egg moves
                      from it, but moves it learn from gen 1 and its pre evo can't learn
                      from gen 1 */
                    Pokemon::uniqueId preevo = PokemonInfo::PreEvo(pokeid);
                    if (preevo.pokenum != 0 &&  preevo.pokenum != pokeid.pokenum) {
                        if (ok && isValid(preevo, g, moves))
                            return true;
                    }
                }
            } else if (!nobreeding) {
                Pokemon::uniqueId preevo = PokemonInfo::PreEvo(pokeid);

                if (preevo.pokenum != 0 &&  preevo.pokenum != pokeid.pokenum && PokemonInfo::Exists(preevo, 1)) {
                    bool ok = true;
                    foreach(int move, moves) {
                        if (!MoveInfo::Exists(move, 1)) {
                            ok = false;
                            break;
                        }
                    }

                    if (ok && isValid(preevo, g, moves))
                        return true;
                }
            }
        }

        /* Pokemon that require knowing a certain move to evolve can't have
          4 other moves from their preevolution except in gen 6 */
        if (PokemonInfo::MoveEvolution(pokeid.pokenum)) {
            QSet<int> moves3 = moves;
            moves3.subtract(PokemonInfo::dreamWorldMoves(pokeid, g));
            moves3.subtract(PokemonInfo::EggMoves(pokeid, g));
            moves3.subtract(PokemonInfo::SpecialMoves(pokeid, g));
            if(g >= 6) {
                moves3.subtract(PokemonInfo::PreEvoMoves(pokeid, g));
            }
            if(moves3.size() >= 1) {
                QSet<int> moves4;
                moves4 = PokemonInfo::EggMoves(pokeid, g);
                moves4.unite(PokemonInfo::dreamWorldMoves(pokeid, g));
                if(g >= 6) {
                    moves4.unite(PokemonInfo::PreEvoMoves(pokeid, g));
                }
                moves3.unite(moves4.intersect(moves));
            }
            if(moves3.size() == 4) {
                if (invalid_moves) {
                    *invalid_moves->insert(moves.toList().value(0));
                }
                if (error) {
                    *error = QObject::tr(gen >= 6 ? "%1 can't have 4 moves which are learned from a previous evolution in a past generation or learned by breeding."
                                                  :"%1 can't have 4 moves learned from a previous evolution.")
                            .arg(PokemonInfo::Name(pokeid));
                }
                return false;
            }
        }

        if (!nobreeding) {
            /* If there's a pre evo move and an old gen move, you must check the pre evo has the combination in the old gen */
            QSet<int> moves3 = moves;
            moves3.subtract(PokemonInfo::PreEvoMoves(pokeid,g));

            if (moves3.size() != moves.size()) {
                Pokemon::uniqueId pokemon = PokemonInfo::PreEvo(pokeid);

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

                /* Shedinja can get one of ninjask's move, as an event move, for free */
                if (pokeid == Pokemon::Shedinja) {
                    foreach(int move, moves) {
                        if (PokemonInfo::SpecialMoves(pokeid, g).contains(move)) {
                            QSet<int> moves4 = moves;
                            moves4.remove(move);

                            if (isValid(pokemon, g, moves4, ab2, gender, level, maledw))
                                return true;
                        }
                    }
                }
                if (isValid(pokemon, g, moves, ab2, gender, level, maledw))
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

        if (g.num == 5) {
            AbilityGroup ab = PokemonInfo::Abilities(pokeid, g);

            if (ability == ab.ab(2) || (ability == ab.ab(0) && ab.ab(2) == 0)) {
                if (moves.size() == 1 && PokemonInfo::dreamWorldMoves(pokeid, g).contains(moves)) {
                    return true;
                }
            }
        }

        foreach(int move, moves) {
            if (g > 3 && MoveInfo::isHM(move, Pokemon::gen(g.num-1, GenInfo::NumberOfSubgens(g.num - 1)-1))
                    && !PokemonInfo::HasMoveInGen(pokeid, *moves.begin(), g) && !PokemonInfo::dreamWorldMoves(pokeid, g).contains(moves)) {
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

bool MoveSetChecker::isAnEggMoveCombination(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, QSet<int> moves)
{
    if (!legalCombinations.contains(gen)) {
        loadGenData(gen);
    }
    /* In gen 6, everything is an egg move combinations (except for event moves */
    if (gen >= 6 && PokemonInfo::EggMoves(pokeid, gen).contains(moves)) {
        return true;
    }
    foreach(QSet<int> combination, legalCombinations[gen].value(pokeid)) {
        if (combination.contains(moves))
            return true;
    }

    return false;
}

/* Used by ChainBreeding */
QList<QSet<int> > MoveSetChecker::combinationsFor(Pokemon::uniqueId pokenum, Pokemon::gen gen)
{
    return legalCombinations.value(gen).value(pokenum);
}

QHash<Pokemon::uniqueId, QList<QSet<int> > > MoveSetChecker::eventCombinationsOf(Pokemon::gen gen)
{
    if (!legalCombinations.contains(gen)) {
        loadGenData(gen);
    }
    return eventCombinations.value(gen);
}

QHash<Pokemon::uniqueId, QList<QSet<int> > > MoveSetChecker::breedingCombinationsOf(Pokemon::gen gen)
{
    if (!legalCombinations.contains(gen)) {
        loadGenData(gen);
    }
    return breedingCombinations.value(gen);
}

void MoveSetChecker::rbyInvalidCombinations(QHash<QString, QHash<QString, QList<QString> > >* hash) {
    QHash<QString, QList<QString> > temp;
    temp.insert("Harden", {"Poison Sting", "String Shot"});
    hash->insert("Kakuna", temp);
    hash->insert("Beedrill", temp);
    temp.remove("Harden");
    temp.insert("Thrash", {"Fury Attack"});
    hash->insert("Nidoking", temp);
    temp.remove("Thrash");
    temp.insert("Pay Day", {"Tail Whip", "Growl"});
    hash->insert("Rapidash", temp);
    temp.remove("Pay Day");
    temp.insert("Stomp", {"Sleep Powder", "Stun Spore", "Poison Powder"});
    hash->insert("Exeggutor", temp);
    temp.remove("Stomp");
    temp.insert("Pay Day", {"Peck"});
    hash->insert("Fearow", temp);
    temp.remove("Pay Day");
    temp.insert("Focus Energy", {"Thunder Shock"});
    hash->insert("Jolteon", temp);
    temp.remove("Focus Energy");
    temp.insert("Focus Energy", {"Ember"});
    hash->insert("Flareon", temp);
    temp.remove("Focus Energy");
}
