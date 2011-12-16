#ifndef BASEBATTLEWINDOW_H
#define BASEBATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"
#include "client.h"

#include "basebattlewindowinterface.h"
#include "../BattleManager/battledatatypes.h"

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>

class BaseBattleDisplay;
class QScrollDownTextBrowser;
class QClickPBar;
class Log;
class SpectatorWindow;

struct BaseBattleInfo
{
    BaseBattleInfo(const PlayerInfo & me, const PlayerInfo &opp, int mode, int myself=0, int opponent=1);
    /* name [0] = mine, name[1] = other */
    PlayerInfo pInfo[2];
    QVector<bool> sub;
    QVector<Pokemon::uniqueId> specialSprite;
    QVector<Pokemon::uniqueId> lastSeenSpecialSprite;

    quint16 time[2];
    bool ticking[2];
    int startingTime[2];

    int mode;
    int numberOfSlots;

    int myself;
    int opponent;

    int gen;

    /* Opponent pokemon */
    ShallowBattlePoke pokemons[2][6];
    QVector<bool> pokeAlive;

    ShallowBattlePoke &currentShallow(int spot) {
        return pokemons[player(spot)][slotNum(spot)];
    }
    const ShallowBattlePoke &currentShallow(int spot) const {
        return pokemons[player(spot)][slotNum(spot)];
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

    int slotNum(int slot) const {
        return slot / 2;
    }

    bool isOut(int , int poke) const {
        return poke < numberOfSlots/2;
    }

    bool multiples() const {
        return mode == ChallengeInfo::Doubles || mode == ChallengeInfo::Triples;
    }

    virtual void switchPoke(int spot, int poke) {
        std::swap(currentShallow(spot), pokemons[player(spot)][poke]);
        pokeAlive[spot] = true;
    }

    virtual void switchOnSide(int player, int s1, int s2) {
        int pk1 = slot(player, s1);
        int pk2 = slot(player, s2);
        std::swap(currentShallow(pk1), currentShallow(pk2));
        std::swap(pokeAlive[pk1], pokeAlive[pk2]);
        std::swap(sub[pk1], sub[pk2]);
        std::swap(specialSprite[pk1], specialSprite[pk2]);
        std::swap(lastSeenSpecialSprite[pk1], lastSeenSpecialSprite[pk2]);
    }

    /* Stat boosts & team status */
    QList<BattleDynamicInfo> statChanges;
};

/* The battle window called by the client, online */
class Client;

class BaseBattleWindow : public BaseBattleWindowInterface
{
    Q_OBJECT

    PROPERTY(int, animatedHpSpot)
    PROPERTY(int, animatedHpGoal)
    PROPERTY(int, ownid)
    PROPERTY(bool, started)
    PROPERTY(bool, usePokemonNames)
    PROPERTY(BattleConfiguration, conf)
public:
    BaseBattleInfo *myInfo;
    const BaseBattleInfo &info() const {
        return *myInfo;
    }
    BaseBattleInfo &info() {
        return *myInfo;
    }

    BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf, int ownid, Client *client);

    int gen() const {
        return info().gen;
    }

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
        TierSection = 40,
        EndMessage,
        PointEstimate,
        MakeYourChoice,
        Avoid,
        RearrangeTeam,
        SpotShifts
    };

    enum TempPokeChange {
        TempMove,
        TempAbility,
        TempItem,
        TempSprite,
        DefiniteForme,
        AestheticForme,
        DefMove,
        TempPP
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

    void printLine(const QString &str, bool silent = false);
    void printHtml(const QString &str, bool silent = false, bool newline = true);
    QString name(int spot) const;
    virtual QString nick(int spot) const;
    QString rnick(int spot) const;
    int player(int spot) const;
    int opponent(int player) const;

    Client *& client() {
        return _mclient;
    }

    const Client * client() const {
        return _mclient;
    }

    bool musicPlayed() const;
    bool flashWhenMoved() const;
    void playCry(int pokemon);
    void close();

public slots:
    void receiveInfo(QByteArray);
    void sendMessage();
    void clickClose();
    void delay(qint64 msec=0, bool forceDelay=true);
    void undelay();

    void animateHPBar();
    void ignoreSpectators();

    void musicPlayStop();
    void enqueueMusic();
    void criesProblem(Phonon::State newState);
signals:
    void battleCommand(int battleId, const BattleChoice &);
protected:
    int delayed;
    int ignoreSpecs;

    enum IgnoreMode {
        NoIgnore,
        IgnoreSpecs,
        IgnoreAll
    };

    QGridLayout *mylayout;
    QScrollDownTextBrowser *mychat;
    QIRCLineEdit *myline;
    QPushButton *myclose, *mysend, *myignore;
    Client *_mclient;

    QCheckBox *saveLogs;
    QCheckBox *musicOn;
    QCheckBox *flashWhenMoveDone;

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
    bool undelayOnSounds;

    QLinkedList<QByteArray> delayedCommands;
    QSet<int> spectators;

    bool blankMessage;
    bool battleEnded;

    Log *log;
    SpectatorWindow *test;

    BaseBattleWindow();
    void init();
    void checkAndSaveLog();

    void closeEvent(QCloseEvent *);
    virtual void dealWithCommandInfo(QDataStream &s, int command, int spot, int truespot);
};

#endif // BASEBATTLEWINDOW_H
