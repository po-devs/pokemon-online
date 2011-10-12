#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battlecommandmanager.h"
#include "advancedbattledata.h"

class QDeclarativeView;
class BattleSceneProxy;
class ProxyDataContainer;

class BattleScene: public QObject, public BattleCommandManager<BattleScene>
{
    Q_OBJECT
public:
    typedef AdvancedBattleData* battledata_ptr;

    BattleScene(battledata_ptr data=0);
    ~BattleScene();

    enum StatDirection {
        StatDown = -1,
        NoStat = 0,
        StatUp = 1
    };

    QDeclarativeView *getWidget();
    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

    /* Prints debug text in the console and battle log */
    Q_INVOKABLE void debug(const QString&m);

    /* Consecutive stat ups or stat downs are not animated. This
      function tells whether or not a stat up/stat down should be
      animated. */
    Q_INVOKABLE bool isFreshForStatChange(int slot, StatDirection direction);

    Q_ENUMS(StatDirection)

    Q_PROPERTY(bool reversed READ reversed() CONSTANT)

    bool reversed();
    void launch();

    template <enumClass val, typename... Params>
    bool shouldInvoke(Params...) {
        if (val != BattleEnum::StatChange) {
            info.lastStatChange = NoStat;
        }

        return true;
    }

    void onStatBoost(int spot, int stat, int boost, bool silent);

signals:
    void printMessage(const QString&);
    void launched();
private:
    battledata_ptr mData;
    battledata_ptr data();

    BattleSceneProxy *mOwnProxy;

    QDeclarativeView *mWidget;

    struct BattleSceneInfo {
        StatDirection lastStatChange;
        int lastSlot;

        BattleSceneInfo() {
            lastSlot = -1;
            lastStatChange = NoStat;
        }
    };

    BattleSceneInfo info;
};


#endif // BATTLESCENE_H
