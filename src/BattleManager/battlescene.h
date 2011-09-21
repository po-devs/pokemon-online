#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battlecommandmanager.h"
#include "battledata.h"

class QDeclarativeView;
class BattleSceneProxy;
class ProxyDataContainer;

class BattleScene: public QObject, public BattleCommandManager<BattleScene>
{
    Q_OBJECT
public:
    typedef BattleData<ProxyDataContainer>* battledata_ptr;

    BattleScene(battledata_ptr data);
    ~BattleScene();

    QDeclarativeView *getWidget();
    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

    Q_INVOKABLE void debug(const QString&m);
signals:
    void printMessage(const QString&);
private:
    battledata_ptr mData;
    battledata_ptr data();

    BattleSceneProxy *mOwnProxy;

    QDeclarativeView *mWidget;
};


#endif // BATTLESCENE_H
