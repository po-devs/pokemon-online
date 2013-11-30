#ifndef TESTDOUBLES_H
#define TESTDOUBLES_H

#include "battleservertest.h"

class TestDoubles : public BattleServerTest
{
    Q_OBJECT
public slots:
    void onBattleServerConnected();
    void onBattleCreated(int b, int p1, int p2, const TeamBattle &team, const BattleConfiguration &conf, const QString &tier);
};

#endif // TESTDOUBLES_H
