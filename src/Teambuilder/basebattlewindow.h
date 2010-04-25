#ifndef BASEBATTLEWINDOW_H
#define BASEBATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"
#include "client.h"
#include <phonon/phononnamespace.h>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>

class BaseBattleDisplay;
class QScrollDownTextEdit;
class QClickPBar;


enum {
    Myself,
    Opponent
};

struct BaseBattleInfo
{
    BaseBattleInfo(const PlayerInfo & me, const PlayerInfo &opp);
    /* name [0] = mine, name[1] = other */
    PlayerInfo pInfo[2];
    bool sub[2];
    quint16 specialSprite[2];
    quint16 time[2];
    bool ticking[2];
    int startingTime[2];


    /* Opponent pokemon */
    ShallowBattlePoke pokemons[2][6];
    bool pokeAlive[2];
    quint8 currentIndex[2];

    ShallowBattlePoke &currentShallow(int player) {
        return pokemons[player][currentIndex[player]];
    }
    const ShallowBattlePoke &currentShallow(int player) const {
        return pokemons[player][currentIndex[player]];
    }

    QString name(int x) const {
        return pInfo[x].team.name;
    }

    /* Stat boosts & team status */
    BattleDynamicInfo statChanges[2];
};

/* The battle window called by the client, online */
class Client;

class BaseBattleWindow : public QWidget
{
    Q_OBJECT

    PROPERTY(int, battleId);
    PROPERTY(Client *, client);
    PROPERTY(int, animatedHpSpot);
    PROPERTY(int, animatedHpGoal);
public:
    BaseBattleInfo *myInfo;
    const BaseBattleInfo &info() const {
        return *myInfo;
    }
    BaseBattleInfo &info() {
        return *myInfo;
    }

    BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent);

    enum BattleCommand
    {
        SendOut,
        SendBack,
        UseAttack,
        OfferChoice,
        BeginTurn,
        ChangePP,
        ChangeHp,
        Ko,
        Effective, /* to tell how a move is effective */
        Miss,
        CriticalHit = 10,
        Hit, /* for moves like fury double kick etc. */
        StatChange,
        StatusChange,
        StatusMessage,
        Failed,
        BattleChat,
        MoveMessage,
        ItemMessage,
        NoOpponent,
        Flinch = 20,
        Recoil,
        WeatherMessage,
        StraightDamage,
        AbilityMessage,
        AbsStatusChange,
        Substitute,
        BattleEnd,
        BlankMessage,
        CancelMove,
        Clause = 30,
        DynamicInfo = 31,
        DynamicStats,
        Spectating,
        SpectatorChat,
        AlreadyStatusMessage,
        TempPokeChange,
        ClockStart = 37,
        ClockStop = 38,
        Rated,
        TierSection
    };

    enum TempPokeChange {
        TempMove,
        TempAbility,
        TempItem,
        TempSprite,
        DefiniteForm,
        AestheticForme
    };

    enum WeatherM
    {
        ContinueWeather,
        EndWeather,
        HurtWeather
    };

    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    enum StatusFeeling
    {
        FeelConfusion,
        HurtConfusion,
        FreeConfusion,
        PrevParalysed,
        PrevFrozen,
        FreeFrozen,
        FeelAsleep,
        FreeAsleep,
        HurtBurn,
        HurtPoison
    };

    virtual void switchToNaught(int spot);
    virtual void addSpectator(bool add, int id);

    void playCry(int pokenum);
    Phonon::MediaObject *music;
    Phonon::AudioOutput *musicOutput;
    Phonon::MediaObject *cry;
    Phonon::AudioOutput *cryOutput;

    void printLine(const QString &str);
    void printHtml(const QString &str);
    QString name(int spot) const;
    virtual QString nick(int spot) const;
    QString rnick(int spot) const;

    void delay(int msec=0);
public slots:
    void receiveInfo(QByteArray);
    void sendMessage();
    void clickClose();
    void undelay();

    void animateHPBar();
    void ignoreSpectators(bool);
signals:
    void battleCommand(const BattleChoice &);
    void battleMessage(const QString &str, int);
    void closedBW(int);
protected:
    QGridLayout *mylayout;
    QScrollDownTextEdit *mychat;
    QLineEdit *myline;
    BaseBattleDisplay *mydisplay;
    QPushButton *myclose, *mysend;
    bool ignoreSpecs;

    QLinkedList<QByteArray> delayedCommands;
    bool delayed;

    bool blankMessage;
    bool battleEnded;

    BaseBattleWindow(){delayed=false;ignoreSpecs=false;}
    void init();

    void closeEvent(QCloseEvent *);
    virtual void dealWithCommandInfo(QDataStream &s, int command, int spot, int truespot);
};

class BaseGraphicsZone;

class BaseBattleDisplay : public QWidget
{
    Q_OBJECT
public:
    BaseBattleInfo* myInfo;
    BaseBattleInfo &info() const {
        return *myInfo;
    }

    BaseBattleDisplay(BaseBattleInfo &i);

    virtual void updatePoke(int spot);
    virtual void updateHp(int spot);
    virtual void updateToolTip(int spot);
    void changeStatus(int spot, int poke, int status);
public slots:
    void updateTimers();

protected:
    QString health(int lifePercent);

    BaseGraphicsZone *zone;
    QLabel *nick[2];
    QLabel *level[2];
    QLabel *status[2];
    QClickPBar *bars[2];
    QProgressBar *timers[2];
    QLabel *trainers[2];
    QLabel *gender[2];
    /* The pokeballs to indicate how well a team is doing */
    QLabel *advpokeballs[6];
    QLabel *mypokeballs[6];
};

/* The graphics zone, where both pokes are displayed */
class BaseGraphicsZone : public QGraphicsView
{
    Q_OBJECT
public:
    BaseGraphicsZone();
    /* displays that poke */
    template <class T>
    void switchTo(const T &poke, int spot, bool sub, int specialSprite=0);
    /* Display blank */
    void switchToNaught(int spot);
    /* For tool tips */
    bool event(QEvent *event);

    /* Loads a pixmap if not loaded otherwise go see graphics */
    QPixmap loadPixmap(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub);
    /* We are using a qmap to store the graphics already loaded. So the key of the pixmap
        is a combination of 2 bools, 1 quin8; and one quint16 */
    qint32 key(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub) const;
    QHash<qint32, QPixmap> graphics;
    /* Current pixmaps displayed */
    QGraphicsPixmapItem *mine, *foe;
    QGraphicsScene scene;

    QString tooltips[2];
};

/* Yeepee, at last templates */
template <class T>
void BaseGraphicsZone::switchTo(const T &poke, int spot, bool sub, int specialSprite)
{
    if (spot == Myself)
        mine->setPixmap(loadPixmap(specialSprite?specialSprite:poke.num(), poke.forme(), poke.shiny(), true, poke.gender(), sub));
    else
        foe->setPixmap(loadPixmap(specialSprite?specialSprite:poke.num(), poke.forme(), poke.shiny(), false, poke.gender(), sub));
}

#endif // BASEBATTLEWINDOW_H
