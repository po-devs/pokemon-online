#include "regularbattlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "pokemoninfoaccessor.h"
#include "proxydatacontainer.h"

RegularBattleScene::RegularBattleScene(battledata_ptr dat) : mData(dat), peeking(false),
    pauseCount(0)
{

}

void RegularBattleScene::launch() {
    emit launched();
}

RegularBattleScene::~RegularBattleScene()
{
}

bool RegularBattleScene::reversed()
{
    return data()->role(1) == BattleConfiguration::Player;
}

RegularBattleScene::battledata_ptr RegularBattleScene::data()
{
    return mData;
}

ProxyDataContainer * RegularBattleScene::getDataProxy()
{
    return data()->exposedData();
}

void RegularBattleScene::pause()
{
    pauseCount =+ 1;
    baseClass::pause();
}

void RegularBattleScene::unpause()
{
    pauseCount -= 1;

    if (pauseCount == 0) {
        if (commands.size() > 0) {
            commands[0]->apply();
            delete commands[0];
            commands.erase(commands.begin(), commands.begin()+1);
        }
    }

    baseClass::unpause();
}

void RegularBattleScene::onUseAttack(int spot, int attack) {
    emit attackUsed(spot, attack);
}
