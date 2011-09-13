#ifndef BATTLESCENEPROXY_H
#define BATTLESCENEPROXY_H

#include <QObject>

#include "proxydatacontainer.h"

class BattleScene;

class BattleSceneProxy : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY (ProxyDataContainer* data READ data CONSTANT)

    BattleSceneProxy(BattleScene*);
    ProxyDataContainer *data();
private:
    BattleScene *scene;
};

#endif // BATTLESCENEPROXY_H
