#ifndef SMOGONBUILD_H
#define SMOGONBUILD_H

#include <QString>
#include <QList>

using namespace std;

/*
 * This build object is used to represent a pokemon build from Smogon
 * The use of QLists are in many cases due to the fact that Smogon gives you
 *   multiple choices of what item or move you want to use.
 */

class smogonbuild{
public:
    smogonbuild();
//    ~SmogonBuild();
#ifndef QT5
    void printBuild();
#endif
    QString EVListToString();

    QString buildName;
    QList<QString> *item;
    QList<QString> *ability;
    QList<QString> *nature;
    QList<int> *EVList;

    QList<QString> *move1;
    QList<QString> *move2;
    QList<QString> *move3;
    QList<QString> *move4;

    QString description;
};

#endif // SMOGONBUILD_H
