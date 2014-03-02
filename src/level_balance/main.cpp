#include <QtCore>
#include <iostream>
#include <PokemonInfo/geninfo.h>
using namespace std;

int baseStatSum(const QList<int> l, int level)
{
    int ret = 0;
    ret += ((l[0]*level*2+(31+85)*level)/100+level+10);
    ret += ((l[1]*level*2+(31+85)*level)/100+5)*level/100;
    ret += ((l[2]*level*2+(31+85)*level)/100+5);
    ret += ((l[3]*level*2+(31+85)*level)/100+5)*level/100;
    ret += ((l[4]*level*2+(31+85)*level)/100+5);
    ret += ((l[5]*level*2+(31+85)*level)/100+5);

    return ret;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    cout << "This program will update level_balance.txt for all generations" << endl;
    cout << "Press enter to continue" << endl;
    cin.ignore();

    GenInfo::init();

    for (int gen = GenInfo::GenMin(); gen <= GenInfo::GenMax(); gen++) {
        QString genS = "db/pokes/" + QString::number(gen) + "G/";
        QFile in(genS + "stats.txt");

        in.open(QIODevice::ReadOnly);

        QList<QString> s = QString::fromUtf8(in.readAll()).split("\n");
        QMap<QString, QList<int> > stats;

        foreach(QString str, s) {
            QStringList s2 = str.split(' ');
            QString id = s2.front();
            s2.removeAt(0);
            foreach(QString str2, s2) {
                stats[id].push_back(str2.toInt());
            }
            std::cout << id.toStdString() << "\n";
        }

        int minbasestat = 50000;

        QMapIterator<QString, QList<int> > it(stats);

        while (it.hasNext()) {
            it.next();
            if (baseStatSum(it.value(),100) < minbasestat) {
                minbasestat = baseStatSum(it.value(), 100);
                std::cout << "Poke " << it.key().toStdString() << " has min base stat " << minbasestat << std::endl;
            }
        }

        QMap<QString, int> levels;

        it.toFront();
        while (it.hasNext()) {
            it.next();
            int level = 100*minbasestat/baseStatSum(it.value(), 100);

            while (baseStatSum(it.value(), level) < minbasestat && level < 100) {
                level ++;
            }
            levels[it.key()] = level;
        }
        in.close();

        in.setFileName(genS + "level_balance.txt");
        in.open(QIODevice::WriteOnly);

        QMapIterator<QString, int> it2(levels);

        while(it2.hasNext()) {
            it2.next();
            in.write(it2.key().toUtf8() + " " + QByteArray::number(it2.value()));
            in.write("\n");
        }

        system("pause");
    }
}
