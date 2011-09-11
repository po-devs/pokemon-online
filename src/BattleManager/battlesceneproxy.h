#ifndef BATTLESCENEPROXY_H
#define BATTLESCENEPROXY_H

#include <QObject>

class BattleScene;

class BattleSceneProxy : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QObject *data READ getDataProxy)

    BattleSceneProxy(BattleScene*);

    QObject *getDataProxy();
private:
    BattleScene *scene;
};

#endif // BATTLESCENEPROXY_H
