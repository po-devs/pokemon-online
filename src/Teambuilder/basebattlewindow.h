#ifndef BASEBATTLEWINDOW_H
#define BASEBATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"
#include "client.h"

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>

class BaseBattleDisplay;
class QScrollDownTextEdit;
class QClickPBar;

struct BaseBattleInfo
{
    BaseBattleInfo(const PlayerInfo & me, const PlayerInfo &opp, bool doubles, int myself=0, int opponent=1);
    /* name [0] = mine, name[1] = other */
    PlayerInfo pInfo[2];
    QVector<bool> sub;
    QVector<qint16> specialSprite;
    QVector<qint16> lastSeenSpecialSprite;

    quint16 time[2];
    bool ticking[2];
    int startingTime[2];

    bool doubles;
    int numberOfSlots;

    int myself;
    int opponent;

    /* Opponent pokemon */
    ShallowBattlePoke pokemons[2][6];
    QVector<bool> pokeAlive;
    QVector<quint8> currentIndex;

    ShallowBattlePoke &currentShallow(int spot) {
        return pokemons[player(spot)][currentIndex[spot]];
    }
    const ShallowBattlePoke &currentShallow(int spot) const {
        return pokemons[player(spot)][currentIndex[spot]];
    }

    QString name(int x) const {
        return pInfo[x].team.name;
    }

    int slot(int player, int poke=0) {
        return player + poke*2;
    }

    int player(int slot) const {
        return slot %2;
    }

    /* Stat boosts & team status */
    QList<BattleDynamicInfo> statChanges;
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
    PROPERTY(bool, started);
public:
    BaseBattleInfo *myInfo;
    const BaseBattleInfo &info() const {
        return *myInfo;
    }
    BaseBattleInfo &info() {
        return *myInfo;
    }

    BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, bool doubles);

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
        DynamicStats = 32,
        Spectating,
        SpectatorChat,
        AlreadyStatusMessage,
        TempPokeChange,
        ClockStart = 37,
        ClockStop = 38,
        Rated,
        TierSection,
        EndMessage,
        PointEstimate,
        MakeYourChoice,
        Avoid
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

    //void playCry(int pokenum);

    void printLine(const QString &str);
    void printHtml(const QString &str);
    QString name(int spot) const;
    virtual QString nick(int spot) const;
    QString rnick(int spot) const;
    int player(int spot) const;
    int opponent(int player) const;

    bool musicPlayed() const;
    void playCry(int pokemon);

public slots:
    void receiveInfo(QByteArray);
    void sendMessage();
    void clickClose();
    void delay(qint64 msec=0, bool forceDelay=true);
    void undelay();

    void animateHPBar();
    void ignoreSpectators(bool);

    void musicPlayStop();
    void enqueueMusic();
    void criesProblem(Phonon::State newState);
signals:
    void battleCommand(int battleId, const BattleChoice &);
    void battleMessage(int battleId, const QString &str);
    void closedBW(int);
protected:
    int delayed;
    bool ignoreSpecs;

    QGridLayout *mylayout;
    QScrollDownTextEdit *mychat;
    QLineEdit *myline;
    BaseBattleDisplay *mydisplay;
    QPushButton *myclose, *mysend;

    QCheckBox *saveLogs;
    QCheckBox *musicOn;

    /* The device which outputs the sound */
    Phonon::AudioOutput *audioOutput;
    /* The media the device listens from */
    Phonon::MediaObject *mediaObject;
    /* The media sources for the music */
    QList<QString> sources;
    /* The device for cries */
    Phonon::AudioOutput *cryOutput;
    /* The media the device listens from for pokemon cries */
    Phonon::MediaObject * cryObject;
    /* The pokemon cries stored in memory */
    QHash<int, QByteArray> cries;
    QBuffer cryBuffer;

    QLinkedList<QByteArray> delayedCommands;

    bool blankMessage;
    bool battleEnded;

    BaseBattleWindow(){delayed=0;ignoreSpecs=false;}
    void init();
    void checkAndSaveLog();

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

    QVector<QLabel *> nick;
    QVector<QLabel *> level;
    QVector<QLabel *> status;
    QVector<QLabel *> gender;
    QVector<QClickPBar *> bars;

    QProgressBar *timers[2];
    QLabel * trainers[2];

    /* The pokeballs to indicate how well a team is doing */
    QLabel *advpokeballs[6];
    QLabel *mypokeballs[6];
};

/* The graphics zone, where both pokes are displayed */
class BaseGraphicsZone : public QGraphicsView
{
    Q_OBJECT
public:
    BaseGraphicsZone(BaseBattleInfo *info);
    /* displays that poke */
    template <class T>
    void switchTo(const T &poke, int spot, bool sub, int specialSprite=0);
    /* Display blank */
    void switchToNaught(int spot);
    /* For tool tips */
    void mouseMoveEvent(QMouseEvent *e);

    /* Loads a pixmap if not loaded otherwise go see graphics */
    QPixmap loadPixmap(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub);
    /* We are using a qmap to store the graphics already loaded. So the key of the pixmap
        is a combination of 2 bools, 1 quin8; and one quint16 */
    qint32 key(quint16 num, quint8 forme, bool shiny, bool back, quint8 gender, bool sub) const;
    QHash<qint32, QPixmap> graphics;
    /* Current pixmaps displayed */
    QVector<QGraphicsPixmapItem *> items;
    QGraphicsScene scene;

    QVector<QString> tooltips;
    BaseBattleInfo *mInfo;

    BaseBattleInfo & info() {
        return *mInfo;
    }
};

/* Yeepee, at last templates */
template <class T>
void BaseGraphicsZone::switchTo(const T &poke, int spot, bool sub, int specialSprite)
{
    items[spot]->setPixmap(loadPixmap(specialSprite?specialSprite:poke.num(), specialSprite?0:poke.forme(), poke.shiny(),
                                      info().player(spot) == info().myself , poke.gender(), sub));
}

#endif // BASEBATTLEWINDOW_H
