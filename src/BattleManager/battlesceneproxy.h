#ifndef BATTLESCENEPROXY_H
#define BATTLESCENEPROXY_H

#include <QObject>

#include "battledataaccessor.h"

class BattleScene;

class BattleSceneProxy : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY (BattleDataProxy* data READ data CONSTANT)

    BattleSceneProxy(BattleScene*);
    BattleDataProxy *data();
private:
    BattleScene *scene;
};

#endif // BATTLESCENEPROXY_H
