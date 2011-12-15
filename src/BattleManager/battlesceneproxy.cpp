#include "battlesceneproxy.h"
#include "battlescene.h"
#include "battledataaccessor.h"

BattleSceneProxy::BattleSceneProxy(BattleScene * scene) : scene(scene)
{

}

ProxyDataContainer* BattleSceneProxy::data()
{
    return scene->getDataProxy();
}

BattleScene* BattleSceneProxy::getScene()
{
    return scene;
}
