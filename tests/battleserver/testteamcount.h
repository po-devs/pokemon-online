#ifndef TESTTEAMCOUNT_H
#define TESTTEAMCOUNT_H

#include <PokemonInfo/battlestructs.h>
#include "battleservertest.h"

class BattleInput;
class AdvancedBattleData;

class TestTeamCount : public BattleServerTest
{
    Q_OBJECT
public:
    ~TestTeamCount();
public slots:
    void onBattleCreated(int b, int p1, int p2, const TeamBattle &team, const BattleConfiguration &conf, const QString &tier);
    void onBattleServerConnected();
    void onBattleMessage(int battleid, int playerid, const QByteArray &array);
private:
    QHash<int, BattleInput*> battles;
    QHash<int, BattleConfiguration*> confs;
    QHash<int, AdvancedBattleData*> datas;

    TeamBattle tb1, tb2;
};

#endif // TESTTEAMCOUNT_H
