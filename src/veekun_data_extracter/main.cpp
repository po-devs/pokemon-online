#include <QtCore>
#include <QtXml>
#include "../PokemonInfo/pokemoninfo.h"
#include <algorithm>
#include <set>
#include "../Utilities/functions.h"

QStringList readFile(const char *path) {
    QFile in(path);
    in.open(QIODevice::ReadOnly);

    QStringList list = QString::fromUtf8(in.readAll()).split("\n");

    return list;
}

template <int x, typename lambda>
void transform(const QStringList& list, lambda f) {
    foreach(QString s, list) {
        if (s.length() > 0) {
            QVector<int> params;

            foreach(QString s2, s.split(",")) {params.push_back(s2.toInt());}

            unpack<x>(params.begin(), f);
        }
    }
}

QHash<int, QVector<int> > genList;
QHash<int, Pokemon::gen> idToGen;

QDebug & operator << (QDebug &s, Pokemon::gen g) {
    s << g.toString();

    return s;
}

/*   gen        method          poke   moves */
QHash<int, QHash<QString, QMap<int, std::set<int> > > > moves;

QString methods[] = {"", "level", "egg", "tutor", "tm_and_hm", "special", "special", "special", "special", "special", "level"};

int main(int argc, char *argv[])
{
    transform<2>(readFile("../../pokedex/pokedex/data/csv/version_groups.csv"),
                 [&](int id, int gen){genList[gen].push_back(id); idToGen.insert(id, Pokemon::gen(gen, genList[gen].count()-1));});

    transform<4>(readFile("../../pokedex/pokedex/data/csv/pokemon_moves.csv"),
                 [&](int poke, int version, int move, int method){ if (poke == 0) {return;}
                 moves[version][methods[method]][poke].insert(move);});

    QHashIterator<int, QHash<QString, QMap<int, std::set<int> > > > it (moves);

    while (it.hasNext()) {
        it.next();

        qDebug() << "Version: " << it.key();

        Pokemon::gen gen(idToGen[it.key()]);

        QDir dir;
        dir.mkdir(gen.toString());
        dir.cd(gen.toString());

        QHashIterator<QString, QMap<int, std::set<int> > > it2(it.value());

        while (it2.hasNext()) {
            it2.next();

            QFile out(dir.absoluteFilePath(it2.key() + "_moves.txt"));
            out.open(QIODevice::WriteOnly);

            QMapIterator<int, std::set<int> > it3(it2.value());

            while (it3.hasNext()) {
                it3.next();

                QStringList mv;
                foreach(int x, it3.value()) {mv.push_back(QString::number(x));}

                out.write(QString("%1:0 %2\n").arg(it3.key()).arg(mv.join(" ")).toUtf8());
            }
        }
    }

    QCoreApplication a(argc, argv);
    a.exec();
}
