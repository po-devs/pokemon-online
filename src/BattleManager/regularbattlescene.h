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
#include <QGraphicsView>
#include <QGraphicsPixmapItem>

class ProxyDataContainer;
class QProgressBar;
class QLabel;
class QClickPBar;
class QHBoxLayout;
class QGridLayout;
class GraphicsZone;

class RegularBattleScene: public QWidget, public BattleCommandManager<RegularBattleScene, BattleSceneFlow<BattleEnum, RegularBattleScene> >
{
    Q_OBJECT
public:
    typedef AdvancedBattleData* battledata_ptr;
    typedef BattleCommandManager<RegularBattleScene, BattleSceneFlow<BattleEnum, RegularBattleScene> > baseClass;

    RegularBattleScene(battledata_ptr data=0, BattleDefaultTheme*theme=0);
    ~RegularBattleScene();

    ProxyDataContainer *getDataProxy();

    void pause();

    /* Should the players be reversed positions in the visual scene? */
    bool reversed() const;
    int opponent() const;
    int myself() const;
    bool isPlayer(int spot) const;

    template <enumClass val, typename... Params>
    bool shouldStartPeeking(param<val>, Params...) {
        return false;
    }

    template <enumClass val, typename... Params>
    bool shouldContinuePeeking(param<val>, Params...) {
        return false;
    }

    void onUseAttack(int spot, int attack);
    void onPokeballStatusChanged(int player, int poke, int status);
    void onKo(int spot) { updatePoke(spot); }
    void onMajorStatusChange(int spot, int, bool){ updatePoke(spot);}
    void onSendOut(int spot, int previndex, ShallowBattlePoke*, bool) {
        updatePoke(spot);
        updateBall(data()->player(spot), previndex);
    }
    void onHpChange(int spot, int newHp);

    void updateBall(int player, int poke);
    void updateBallStatus(int player, int poke);
    void updatePoke(int spot);
    void updateHp(int spot, int val = -1);
    static QString health(int lifePercent);

    bool isPeeking() const { return peeking; }
    bool isPaused() const {return pauseCount > 0;}

    void startPeeking() { peeking = true; }
    void stopPeeking() { peeking = false; }
signals:
    void printMessage(const QString&);
    void attackUsed(int spot, int attack);
public slots:
    void updateTimers();
    void changeBarMode();

    void unpause();
protected slots:
    void animateHpBar();
private:
    battledata_ptr mData;
    battledata_ptr data();
    const battledata_ptr data() const;

    bool peeking;
    int pauseCount;

    struct Gui {
        GraphicsZone *zone;

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

    struct Info {
        Info(int nslots);

        QVector<int> time;
        QVector<int> startingTime;
        QVector<int> ticking;

        QVector<int> percentage;

        int animatedSpot;
        int animatedValue;
    };

    Gui gui;
    Info info;

    void setupGui();
    QHBoxLayout *createTeamLayout(QLabel** labels);
    QGridLayout *createHPBarLayout(int slot);
    QWidget *createFullBarLayout(int nslots, int player);
};

/* The graphics zone, where both pokes are displayed */
class GraphicsZone : public QGraphicsView
{
    typedef AdvancedBattleData* battledata_ptr;
public:
    GraphicsZone(battledata_ptr info, BattleDefaultTheme *theme);
    /* displays that poke */
    template <class T>
    void switchTo(const T &poke, int spot, bool sub, Pokemon::uniqueId specialSprite = Pokemon::NoPoke) {
        items[spot]->setPixmap(loadPixmap(specialSprite != Pokemon::NoPoke ? specialSprite:poke.num(), poke.shiny(),
                                          info()->player(spot) == myself(), poke.gender(), sub));
        updatePos(spot);
    }

    /* Display blank */
    void switchToNaught(int spot);
    /* For tool tips */
    void mouseMoveEvent(QMouseEvent *e);
    /* Updates the position of an item */
    void updatePos(int spot);

    /* Loads a pixmap if not loaded otherwise go see graphics */
    QPixmap loadPixmap(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub);
    /* We are using a qmap to store the graphics already loaded. So the key of the pixmap
            is a combination of 2 bools, 1 quin8; and one quint16 */
    quint64 key(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub) const;
    QHash<qint32, QPixmap> graphics;
    /* Current pixmaps displayed */
    QVector<QGraphicsPixmapItem *> items;
    QGraphicsScene scene;

    QVector<QString> tooltips;
    battledata_ptr mInfo;

    battledata_ptr info() {
        return mInfo;
    }

    const battledata_ptr info() const {
        return mInfo;
    }

    int opponent() const;
    int myself() const;
    bool reversed() const;
};

#endif // REGULARBATTLESCENE_H
