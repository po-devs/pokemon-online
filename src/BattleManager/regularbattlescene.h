#ifndef REGULARBATTLESCENE_H
#define REGULARBATTLESCENE_H

/*
 * Non-QML battle scene
 */

#include "battlesceneflow.h"
#include "battlecommandmanager.h"
#include "advancedbattledata.h"
#include "defaulttheme.h"
#include <QWidget>

class BattleSceneProxy;
class ProxyDataContainer;
class QProgressBar;
class QLabel;
class QClickPBar;
class QHBoxLayout;
class QGridLayout;

class RegularBattleScene: public QWidget, public BattleCommandManager<RegularBattleScene, BattleSceneFlow<BattleEnum, RegularBattleScene> >
{
    Q_OBJECT
public:
    typedef AdvancedBattleData* battledata_ptr;
    typedef BattleCommandManager<RegularBattleScene, BattleSceneFlow<BattleEnum, RegularBattleScene> > baseClass;

    RegularBattleScene(battledata_ptr data=0, BattleDefaultTheme*theme=0);
    ~RegularBattleScene();

    ProxyDataContainer *getDataProxy();

    Q_INVOKABLE void pause();
    Q_INVOKABLE void unpause();

    Q_PROPERTY(bool reversed READ reversed() CONSTANT)

    /* Should the players be reversed positions in the visual scene? */
    bool reversed() const;
    int opponent() const;
    int myself() const;
    void launch();

    template <enumClass val, typename... Params>
    bool shouldStartPeeking(param<val>, Params...) {
        return false;
    }

    template <enumClass val, typename... Params>
    bool shouldContinuePeeking(param<val>, Params...) {
        return false;
    }
    void onUseAttack(int spot, int attack);

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking() { peeking = true; }
    void stopPeeking() { peeking = false; }
signals:
    void printMessage(const QString&);
    void attackUsed(int spot, int attack);
private:
    battledata_ptr mData;
    battledata_ptr data();
    const battledata_ptr data() const;

    bool peeking;
    int pauseCount;

    struct Gui {
        QWidget *zone;

        QVector<QLabel *> nick;
        QVector<QLabel *> level;
        QVector<QLabel *> status;
        QVector<QLabel *> gender;
        QVector<QClickPBar *> bars;

        QProgressBar *timers[2];
        QLabel * trainers[2];

        /* The pokeballs to indicate how well a team is doing */
        QLabel *pokeballs[2][6];

        BattleDefaultTheme *theme;
    };

    Gui gui;

    void setupGui();
    QHBoxLayout *createTeamLayout(QLabel** labels);
    QGridLayout *createHPBarLayout(int slot);
    QWidget *createFullBarLayout(int nslots, int player);
};

#endif // REGULARBATTLESCENE_H
