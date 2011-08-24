namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);


#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/movesetchecker.h"
#include <algorithm>

QList< QSet<QSet<int> > > legalCombinations;

QMultiHash<QString, int> pokesOfGroup;

//We're with that are 9 bits
uint qHash(QSet<int> s) {
    quint64 toHash = 0;

    QList<int> toSort = s.toList();
    std::sort(toSort.begin(), toSort.end());

    for (int i =0; i < toSort.size() ; i++) {
        toHash += toSort[i] << ((i*9) % 64);
    }

    return qHash(toHash);
}

QString getLine(const QString & filename, int linenum)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    for (int i = 0; i < linenum; i++)
    {
        filestream.readLine();
    }

    return filestream.readLine();
}

int main(int, char**)
{
    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/");
    MoveInfo::init("db/moves/");

    int gen = 5;
    int pokenum = PokemonInfo::TrueCount(gen);

    qDebug() << "Gen " << gen;
    qDebug() << "Pokemons: " << pokenum;
    qDebug() << "";

    int count = 0;

    for (int i = 0; i < pokenum; i++) {
        QString group1(getLine("db/pokes/poke_egg_group_1.txt",i).section(' ', 1));
        QString group2(getLine("db/pokes/poke_egg_group_2.txt",i).section(' ', 1));

        if (group1.toInt() != 0)
            group1 = "";
        if (group2.toInt() != 0)
            group2 = "";

        pokesOfGroup.insert(group1,i);
        pokesOfGroup.insert(group2,i);
    }
    pokesOfGroup.remove("");

    /* Now we have the groups made, just showing to make sure there's no typo */
    qDebug() << pokesOfGroup.size() << " keys: ";

    foreach(QString s, pokesOfGroup.keys().toSet()) {
        qDebug() << "Group " << s;;
    }

    QHash<Pokemon::uniqueId, QList<QSet<int> > > oldC = MoveSetChecker::breedingCombinationsOf(gen);

    for (int i = 0; i< pokenum; i++)
    {
        qDebug() << "Doing poke " << PokemonInfo::Name(i);
        legalCombinations.push_back(QSet<QSet<int> > ());

        QString groups[2] = {getLine("db/pokes/poke_egg_group_1.txt",i).section(' ', 1),
                             getLine("db/pokes/poke_egg_group_2.txt",i).section(' ', 1)};
        if (groups[0].toInt() != 0)
            groups[0] = "";
        if (groups[1].toInt() != 0)
            groups[1] = "";

        //Removed because event combinations need to be preserved
//        if (groups[0] == "" && groups[1] == "")
//            continue;

        QSet<int> eggMoves = PokemonInfo::EggMoves(i, gen);
        QSet<int> regularMoves = PokemonInfo::RegularMoves(i, gen);

        //We don't want useless data
        foreach(int move, regularMoves) {
            eggMoves.remove(move);
        }

        eggMoves.remove(0);

        if (eggMoves.size() == 0)
            continue;

        /* All egg moves combinations */
        QSet<QSet<int> > allCombinations;

        QList<int> toList = eggMoves.toList();

        QList<QSet<int> > combinations = oldC[i];
        allCombinations = combinations.toSet();

        foreach(int move, toList) {
            foreach(int move2, toList) {
                if (move != move2)
                    allCombinations.insert(QSet<int> () << move << move2);
            }
            foreach(QSet<int> combination, combinations) {
                if (!combination.contains(move)) {
                    combination.insert(move);
                    allCombinations.insert(combination);
                }
            }
        }

//        /* Brute force to without recursion for the heck of it to get all combinations (non ordered, of course)*/
//        QVector<int> currentVect;

//        currentVect.resize(1);

//        while (currentVect.size() < toList.size()) {
//            currentVect.push_back(0);
//            for (int i = 0; i < currentVect.size(); i++) {
//                currentVect[i] = i;
//            }

//            int posInVect = 0;
//            while(posInVect != -1) {
//                QSet<int> toPush;
//                for (int j = 0; j < currentVect.size(); j++) {
//                    toPush.insert(toList[currentVect[j]]);
//                }
//                allCombinations.insert(toPush);
//                /* on to the next */
//                posInVect = currentVect.size()-1;
//                while (posInVect != -1 && currentVect[posInVect] == toList.size()-(currentVect.size()-posInVect)) {
//                    posInVect--;
//                }
//                if (posInVect >= 0) {
//                    currentVect[posInVect]++;
//                    posInVect++;
//                    while(posInVect < currentVect.size()) {
//                        currentVect[posInVect] = currentVect[posInVect-1] + 1;
//                        posInVect++;
//                    }
//                }
//            }
//        }

        /* Saves up time */
        foreach(QSet<int> combination, allCombinations) {
            if (MoveSetChecker::isAnEggMoveCombination(i, gen, combination)) {
                legalCombinations[i].insert(combination);
                allCombinations.remove(combination);
            }
        }
        /* Adding all the combinations of special moves */
        legalCombinations[i].unite(combinations.toSet());

        for(int c = 0;c < 2; c++) {
            if (groups[c].trimmed().length() == 0) {
                continue;
            }


            foreach(int poke, pokesOfGroup.values(groups[c])) {
                /* If the pokemon is female only we don't want her eggmoves as she can't be father */
                if (PokemonInfo::Gender(poke) == Pokemon::Female) {
                    continue;
                }

                /* Not losing time with Smeargle - Adding all egg moves directly */
                if (poke == Pokemon::Smeargle) {
                    allCombinations.clear();
                    allCombinations.insert(PokemonInfo::EggMoves(i, gen));
                }

                //We get the regular moves of that poke
                QSet<int> regularMoves = PokemonInfo::RegularMoves(poke, gen);
                //And now the "special moves" of that poke.
                QSet<int> specialMoves = PokemonInfo::EggMoves(poke,gen).unite(PokemonInfo::SpecialMoves(poke,gen)).unite(PokemonInfo::PreEvoMoves(poke, gen));

                /* And now, we assume that the poke can inherit only 1 special move of the father and any number of regular moves.
                   Of course that may not be true, and some poke given in events have many eggmoves or egg moves
                    + special moves that they could give another poke. This is solved by running the program multiple time :p. */


                /* We look at all egg moves combinations possible and see if the father can give them */
                foreach(QSet<int> combination, allCombinations) {
                    QSet<int> copy = combination;

                    foreach(int move, combination) {
                        if (regularMoves.contains(move)) {
                            copy.remove(move);
                        }
                    }
                    /* Now then, if copy.size() is 0 then all moves in the combination are
                       part from the regular moves of the father. Otherwise, all regular moves
                        are removed and the remaining moves are in copy and tested to see if the
                        father could learn them legally */
                    if (copy.empty() || MoveSetChecker::isValid(poke, gen, copy)) {
                        legalCombinations[i].insert(combination);
                        /* we remove it to avoid doing it again */
                        allCombinations.remove(combination);
                        /* and we continue! */

                        if (!MoveSetChecker::isAnEggMoveCombination(i, gen, combination)) {
                            count ++;
                            qDebug() << "Combination added for " << PokemonInfo::Name(i) << " learnt from " << PokemonInfo::Name(poke);

                            foreach (int z , combination) {
                                qDebug() << "- " << MoveInfo::Name(z);
                            }

                            qDebug() << "";
                        }

                        continue;
                    }
                    /* Else, given our hypothesis, all moves but 1 must be regular and 1 move
                       is special. (1 move combinations don't appear in EggMoveCombinations, that's why we need a second if) */
                    if (copy.size() == 1) {
                        /* last remaining move, are you in the special moves of your father? */
                        if (specialMoves.contains(*copy.begin())) {
                            legalCombinations[i].insert(combination);
                            allCombinations.remove(combination);

                            if (!MoveSetChecker::isAnEggMoveCombination(i, gen, combination)) {
                                count ++;
                                qDebug() << "Combination added for " << PokemonInfo::Name(i) << " learnt from " << PokemonInfo::Name(poke);

                                foreach (int z , combination) {
                                    qDebug() << "- " << MoveInfo::Name(z);
                                }

                                qDebug() << "";
                            }
                        }
                    }
                }
            }
        }

        /* Removing all "sub-lists" (like, if 3 moves form a combination, then
           2 too) to spare some space. Pokemons that have all 4 moves available are
            happy with that! */
        foreach(QSet<int> s, legalCombinations[i]) {
            foreach(QSet<int> s2, legalCombinations[i]) {
                if (s2.size() != s.size() && s2.contains(s)) {
                    legalCombinations[i].remove(s);
                }
            }
        }

        //BIGGEST MISTAKE EVER
//        /* Uniting all sets that have a size of 4 or bigger, as those sets ... */
//        QSet<int> united;
//        foreach(QSet<int> s, legalCombinations[i]) {
//            if (s.size() < 4)
//                continue;
//            united.unite(s);
//            legalCombinations[i].remove(s);
//        }
//        if (united.size() > 0)
//            legalCombinations[i].insert(united);
    }

    /* Now we proudly save the obtained combinations */

    QFile out("db/pokes/legal_combinations_" +QString::number(gen) + "G.txt");
    out.open(QIODevice::WriteOnly);

    bool space, ord, newline;
    newline = false;
    for (int i = 0; i < pokenum; i++) {
        if (legalCombinations[i].size() == 0)
            continue;

        if (newline)
            out.putChar('\n');
        ord=false;
        out.write(QString("%1:0 ").arg(i).toUtf8());
        foreach(QSet<int> sset,legalCombinations[i]) {
            sset.remove(0);
            if (sset.size() == 0)
                continue;

            if (ord)
                out.putChar('|');
            space = false;

            foreach(int val, sset) {
                if (space)
                    out.putChar(' ');
                out.write(QByteArray::number(val));
                space = true;
            }
            ord = true;
        }
        newline = true;
    }

    qDebug() << count << " combinations added!";

    return 0;
}
