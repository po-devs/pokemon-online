namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include <QtCore/QCoreApplication>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/movesetchecker.h"
#include <iostream>
using namespace std;
int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    cout << "This program will update the event combinations with the contents of event_combinations_gen_[1/2/3/4/5/6].txt" << endl;
    cout << "Press enter to continue" << endl;

    cin.ignore();
    cout << "Loading move & pokemon database" << endl;

    GenInfo::init();
    MoveInfo::init();
    PokemonInfo::init();
    MoveSetChecker::init();

    for (int gen = GEN_MIN; gen <= GenInfo::GenMax(); gen++) {
        QString file = "database/event_combinations_gen_" + QString::number(gen) + ".txt";
        cout << "Reading " << file.toStdString().c_str() << endl;

        QHash<Pokemon::uniqueId, QList<QSet<int> > > events = MoveSetChecker::eventCombinationsOf(gen);

        QFile in(file);
        in.open(QIODevice::ReadOnly);
        QList<QString> pokes = QString::fromUtf8(in.readAll()).split('\n');

        foreach(QString poke, pokes) {
            if (!poke.contains(":"))
                continue;
            Pokemon::uniqueId id = PokemonInfo::Number(poke.section(":", 0, 0).trimmed());
            QString data = poke.section(":", 1).trimmed();

            if (id == Pokemon::NoPoke) {
                cout << "Invalid line: " << poke.toStdString().c_str() << " (Pokemon name)" << endl;
                continue;
            }

            if (data.length() > 0) {
                QStringList moves = data.split('+');
                QSet<int> toPush;
                foreach(QString move, moves) {
                    int moveN = MoveInfo::Number(move.trimmed());
                    toPush.insert(moveN);
                }

                if (toPush.contains(0)) {
                    cout << "Invalid line: " << poke.toStdString().c_str() << " (Move name)" << endl;
                    continue;
                }

                if (!events.contains(id))
                    events.insert(id, QList<QSet<int > >());

                if (!events[id].contains(toPush))
                    events[id].push_back(toPush);
            }
        }

        /* Now we proudly save the obtained combinations */

        QFile out(QString("db/pokes/%1G/event_combinations.txt").arg(gen));
        out.open(QIODevice::WriteOnly);

        bool space, ord, newline;
        newline = false;
        for (int i = 0; i <= PokemonInfo::TrueCount(); i++) {
            if (!events.contains(i))
                continue;

            if (newline)
                out.putChar('\n');
            ord=false;
            out.write(QString("%1:0 ").arg(i).toUtf8());
            foreach(QSet<int> sset,events[i]) {
                sset.remove(0);
                if (sset.size() == 0)
                    continue;

                if (ord)
                    out.putChar('|');
                space = false;

                QList<int> sortedList = sset.toList();
                qSort(sortedList);
                foreach(int val, sortedList) {
                    if (space)
                        out.putChar(' ');
                    out.write(QByteArray::number(val));
                    space = true;
                }
                ord = true;
            }
            newline = true;
        }

        if (out.size() == 0) {
            out.close();
            out.remove();
        } else {
            out.copy(QString("db/pokes/%1G/Subgen 0/event_combinations.txt").arg(gen));
        }
    }

    cout << "Finished! Press a key to end." << endl;
    cin.ignore();

    return 0;
}
