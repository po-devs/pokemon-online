#ifndef SMOGONBUILD_H
#define SMOGONBUILD_H

#include <vector>

using namespace std;

/*
 * This build object is used to represent a pokemon build from Smogon
 * The use of arrays are in many cases due to the fact that Smogon gives you
 *   multiple choices of what Item or move you want to use.
 */
struct SmogonBuild
{
    QString buildName;
    vector<QString> item;
    vector<QString> nature;
    int EVList[6];
    vector<QString> moves[4];
    QString description;
};

#endif // SMOGONBUILD_H
