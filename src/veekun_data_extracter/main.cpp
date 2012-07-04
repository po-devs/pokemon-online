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

int maxnum = 649;

using namespace Pokemon;

int formes[] = {Deoxys, Deoxys, Deoxys, Wormadam_G, Wormadam_S, Shaymin, Giratina, Rotom_H, Rotom_W, Rotom_F, Rotom_S, Rotom_C, Castform, Castform, Castform,
               Basurao, Hihidaruma, Meloia, Torunerosu, Borutorosu, Randorosu, Kyurem_B, Kyurem_W, Keldeo};

QString methods[] = {"", "level", "egg", "tutor", "tm_and_hm", "special", "special", "special", "special", "special", "level"};

int main(int argc, char *argv[])
{
    transform<2>(readFile("../../pokedex/pokedex/data/csv/version_groups.csv"),
                 [&](int id, int gen){genList[gen].push_back(id); idToGen.insert(id, Pokemon::gen(gen, genList[gen].count()-1));});

    /* Reorder colosseum / emerald / FRLG to have colosseum go first */
    std::swap(genList[3][2], genList[3][3]);
    std::swap(genList[3][1], genList[3][2]);

    for (int i = 0; i < genList[3].count(); i++) {
        idToGen[genList[3][i]] = Pokemon::gen(3, i);
    }

    transform<4>(readFile("../../pokedex/pokedex/data/csv/pokemon_moves.csv"),
                 [&](int poke, int version, int move, int method){ if (poke == 0) {return;}
    if (poke > maxnum) {
        poke = formes[poke-maxnum-1];
    }
                 moves[version][methods[method]][poke].insert(move);});

    QMutableHashIterator<int, QHash<QString, QMap<int, std::set<int> > > > it (moves);

    while (it.hasNext()) {
        it.next();

        qDebug() << "Version: " << it.key();

        Pokemon::gen gen(idToGen[it.key()]);

//        if (gen > Pokemon::gen(4, 0)) {
//            for (int i = 1; i <= 10; i++) {
//                if (it.value().value(methods[i]).contains(Rotom)) it.value()[methods[i]][Rotom_C] = it.value()[methods[i]][Rotom];
//                if (it.value().value(methods[i]).contains(Rotom)) it.value()[methods[i]][Rotom_H] = it.value()[methods[i]][Rotom];
//                if (it.value().value(methods[i]).contains(Rotom)) it.value()[methods[i]][Rotom_W] = it.value()[methods[i]][Rotom];
//                if (it.value().value(methods[i]).contains(Rotom)) it.value()[methods[i]][Rotom_S] = it.value()[methods[i]][Rotom];
//                if (it.value().value(methods[i]).contains(Rotom)) it.value()[methods[i]][Rotom_F] = it.value()[methods[i]][Rotom];
//            }
//            it.value()["level"][Rotom_C].insert(Move::LeafStorm);
//            it.value()["level"][Rotom_W].insert(Move::HydroPump);
//            it.value()["level"][Rotom_S].insert(Move::AirSlash);
//            it.value()["level"][Rotom_H].insert(Move::Overheat);
//            it.value()["level"][Rotom_F].insert(Move::Blizzard);
//        }

//        if (gen > Pokemon::gen(5, 0)) {
//            for (int i = 1; i <= 10; i++) {
//                if (it.value().value(methods[i]).contains(Kyurem)) it.value()[methods[i]][Kyurem_W] = it.value()[methods[i]][Kyurem];
//                if (it.value().value(methods[i]).contains(Kyurem)) it.value()[methods[i]][Kyurem_B] = it.value()[methods[i]][Kyurem];
//            }
//            it.value()["level"][Kyurem_W].insert(Move::CrossFlame);
//            it.value()["level"][Kyurem_W].insert(Move::IceBurn);
//            it.value()["level"][Kyurem_B].insert(Move::CrossThunder);
//            it.value()["level"][Kyurem_B].insert(Move::FreezeShock);
//        }

        QDir dir;
        dir.mkdir(QString("%1G").arg(int(gen.num)));
        dir.cd(QString("%1G").arg(int(gen.num)));
        dir.mkdir(QString("Subgen %1").arg(int(gen.subnum)));
        dir.cd(QString("Subgen %1").arg(int(gen.subnum)));


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

                out.write(Pokemon::uniqueId(it3.key()).toLine(mv.join(" ")).toUtf8() + "\n");
            }
        }
    }

    QCoreApplication a(argc, argv);
    a.exec();
}
