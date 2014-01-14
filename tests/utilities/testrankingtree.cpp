#include <QString>
#include <QDebug>
#include <Utilities/rankingtree.h>
#include "testrankingtree.h"

void TestRankingTree::run()
{
    RankingTree<QString> rankings;

    auto *moogle = rankings.insert(1600, "Crystal Moogle");
    auto *mystra= rankings.insert(9000, "Mystra");
    auto *skarm = rankings.insert(666, "SkarmPiss");
    auto *scott =rankings.insert(8008, "Scott TM");
    auto *dn = rankings.insert(1337, "Darkness");

    scott = rankings.changeKey(scott, 999);

    assert(moogle->ranking() == 2);
    assert(mystra->ranking() == 1);
    assert(skarm->ranking() == rankings.count());
    assert(dn->ranking() == 3);
    assert(scott->ranking() == 4);
    assert(rankings.count() == 5);
    assert(rankings.getByRanking(2).node()->data == "Crystal Moogle");
}
