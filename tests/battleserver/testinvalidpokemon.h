#ifndef TESTINVALIDPOKEMON_H
#define TESTINVALIDPOKEMON_H

#include "battleservertest.h"

class TestInvalidPokemon : public BattleServerTest
{
public:
    void onBattleServerConnected();
    void onBattleCreated(int b, int p1, int p2, const TeamBattle &team, const BattleConfiguration &conf, const QString &tier);
    void onBattleMessage(int b, int p, const QByteArray&);
};

#endif // TESTINVALIDPOKEMON_H
