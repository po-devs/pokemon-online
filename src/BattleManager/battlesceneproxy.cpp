#include "battlesceneproxy.h"
#include "battlescene.h"
#include "battledataaccessor.h"

BattleSceneProxy::BattleSceneProxy(BattleScene * scene) : scene(scene)
{

}

QObject *BattleSceneProxy::getDataProxy()
{
    return scene->getProxy();
}

