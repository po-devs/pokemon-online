#ifndef BATTLESCENEPROXY_H
#define BATTLESCENEPROXY_H

#include <QObject>

class BattleScene;

class BattleSceneProxy : public QObject
{
    Q_OBJECT
public:
    BattleSceneProxy(BattleScene*);
private:
    BattleScene *scene;
};

#endif // BATTLESCENEPROXY_H
