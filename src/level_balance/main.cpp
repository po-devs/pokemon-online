#include <QtCore>
#include <iostream>

int baseStatSum(const QList<int> l, int level)
{
    int ret = 0;
    ret += ((l[0]*level*2+(31+85)*level)/100+level+10);
    ret += ((l[1]*level*2+(31+85)*level)/100+5)*level/100;
    ret += ((l[2]*level*2+(31+85)*level)/100+5);
    ret += ((l[3]*level*2+(31+85)*level)/100+5);
    ret += ((l[4]*level*2+(31+85)*level)/100+5)*level/100;
    ret += ((l[5]*level*2+(31+85)*level)/100+5);
        
    return ret;
}

int main(int argc, char *argv[])
{
    QFile in("db/pokes/poke_stats.txt");

    in.open(QIODevice::ReadOnly);

    QList<QString> s = QString::fromUtf8(in.readAll()).split("\n");
    QList<QList<int> > stats;

    int i = 0;
    foreach(QString str, s) {
        QStringList s2 = str.split(' ');
        stats.push_back(QList<int>());
        foreach(QString str2, s2) {
            stats[i].push_back(str2.toInt());
        }
        std::cout << i << "\n";
        i++;
    }

    int minbasestat = 50000;

    for (int i = 0; i <= 505; i++) {
        if (baseStatSum(stats[i],100) < minbasestat) {
            minbasestat = baseStatSum(stats[i], 100);
            std::cout << "Poke " << i << " has min base stat " << minbasestat << std::endl;
        }
    }

    QList<int > levels;

    for (int i = 0; i <= 505; i++) {
        int level = 100*minbasestat/baseStatSum(stats[i], 100);

        while (baseStatSum(stats[i], level) < minbasestat && level < 100) {
            level ++;
        }
        levels.push_back(level);
    }
    in.close();

    in.setFileName("level_balance.txt");
    in.open(QIODevice::WriteOnly);
    for (int i = 0; i<= 505; i++) {
        in.write(QByteArray::number(levels[i]));
        in.write("\n");
    }

    system("pause");
}
