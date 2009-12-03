#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"

class Player;

class BattleSituation : public QThread
{
    Q_OBJECT

    PROPERTY(int, turn)
public:
    enum {
	AllButPlayer = -2,
	All = -1,
	Player1,
	Player2
    };
    typedef QVariantMap context;

    BattleSituation(Player &p1, Player &p2);
    ~BattleSituation();

    const TeamBattle &pubteam(int id);
    /* returns 0 or 1, or -1 if that player is not involved */
    int spot(int id) const;
    /* The other player */
    int rev(int spot) const;
    /* returns the id corresponding to that spot (spot is 0 or 1) */
    int id(int spot) const;
    /* Return the configuration of the players (1 refer to that player, 0 to that one... */
    BattleConfiguration configuration() const;

    /*
	Below Player is either 1 or 0, aka the spot of the id.
	Use the functions above to make conversions
    */
    TeamBattle &team(int player);
    const TeamBattle &team(int player) const;
    PokeBattle &poke(int player);
    const PokeBattle &poke(int player) const;
    PokeBattle &poke(int player, int poke);
    const PokeBattle &poke(int player, int poke) const;
    int currentPoke(int player) const;
    bool koed(int player) const;
    void changeCurrentPoke(int player, int poke);
    int countAlive(int player) const;

    /* Starts the battle -- use the time before to connect signals / slots */
    void start();
    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
    void run();
    /* requests choice of action from the player */
    bool requestChoice(int player, bool acq = true /*private arg */, bool custom = false); /* return true if the pokemon has a choice to make (including switching & struggle)*/
    void requestChoices(); /* request from both players */
    /* Shows what attacks are allowed or not */
    BattleChoices createChoice(int player) const;
    /* called just after requestChoice(s) */
    void analyzeChoice(int player);
    void analyzeChoices(); 

    /* Commands for the battle situation */
    void beginTurn();
    void endTurn();
    void endTurnStatus();
    /* Attack... */
    /* if special occurence = true, then it means a move like mimic/copycat/metronome has been used. In that case attack does not
	represent the moveslot but rather than that it represents the move num, plus PP will not be lost */
    void useAttack(int player, int attack, bool specialOccurence = false, bool notify = true);
    /* Does not do extra operations,just a setter */
    void changeHp(int player, int newHp);
    /* Sends a poke back to his pokeball (not koed) */
    void sendBack(int player);
    void sendPoke(int player, int poke);
    void koPoke(int player, int source, bool straightattack = false);
    /* Does not do extra operations,just a setter */
    void changeStatMod(int player, int stat, int newstatmod);
    void gainStatMod(int player, int stat, int bonus);
    void loseStatMod(int player, int stat, int malus);
    /* Does not do extra operations,just a setter */
    void changeStatus(int player, int status);
    void healStatus(int player, int status);
    void healConfused(int player);
    void healLife(int player, int healing);
    void inflictStatus(int player, int Status);
    void inflictConfused(int player);
    void inflictConfusedDamage(int player);
    void inflictRecoil(int source, int target);
    void inflictDamage(int player, int damage, int source, bool straightattack = false);
    /* Removes PP.. */
    void changePP(int player, int move, int PP);
    void losePP(int player, int move, int loss);

    int calculateDamage(int player, int target);
    PokeFraction getMod1(int player, int target);
    void applyMoveStatMods(int player, int target);
    bool testAccuracy(int player, int target);
    void testCritical(int player, int target);
    void testFlinch(int player, int target);
    bool testStatus(int player);
    bool testFail(int player);
    bool hasType(int player, int type);
    void requestSwitchIns();
    void requestSwitch(int player);
    int repeatNum(context &move);
    PokeFraction getStatBoost(int player, int stat);
    int getStat(int player, int stat);
    /* conversion for sending a message */
    quint8 ypoke(int, int i) const { return i; } /* aka 'your poke', or what you need to know if it's your poke */
    ShallowBattlePoke opoke(int play, int i) const { return ShallowBattlePoke(poke(play, i));} /* aka 'opp poke', or what you need to know if it's your opponent's poke */

    /* Send a message to the outworld */
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
	CriticalHit,
	Hit, /* for moves like fury double kick etc. */
	StatChange,
	StatusChange,
	StatusMessage,
	Failed
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

    /* Here C++0x would make it so much better looking with variadic templates! */
    void notify(int player, int command, int who);
    template<class T>
    void notify(int player, int command, int who, const T& param);
    template<class T1, class T2>
    void notify(int player, int command, int who, const T1& param1, const T2& param2);
public slots:
    void battleChoiceReceived(int id, const BattleChoice &b);
signals:
    void battleInfo(int id, const QByteArray &info);
private:
    /* To interrupt the thread when needed */
    QSemaphore sem;
    /* To notify the thread to quit */
    bool quit;
    /* if quit==true, throws QuitException */
    void testquit();

    /* What choice we allow the players to have */
    BattleChoices options[2];
    BattleChoice choice[2];
    bool haveChoice[2];

    TeamBattle team1, team2;
    int mycurrentpoke[2]; /* -1 for koed */
    int myid[2];
    QSet<int> koedPokes;

    /* Calls the effects of source reacting to name */
    void calleffects(int source, int target, const QString &name);
    /* This time the pokelong effects */
    void callpeffects(int source, int target, const QString &name);
    /* this time the general battle effects (imprison, ..) */
    void callbeffects(int source, int target, const QString &name);
public:
    /**************************************/
    /*** VIVs: very important variables ***/
    /**************************************/
    /* Those variable are used throughout the battle to describe the situation.
       These are 'dynamic', in contrary to 'static'. For exemple when a pokemon
       is switched in, its type and ability and moves are stored in there, taken
       from their 'static' value that are in the TeamBattle struct. Then they
       can be changed (like with ability swap) but when the poke is sent back then
       back in the dynamic value is restored to the static one. */

    /* Variables that are reset when the poke is switched out.
	Like for exemple a Requiem one... */
    context pokelong[2];
    /* Variables that are reset every turn right before everything else happens
	at the very beginning of a turn */
    context turnlong[2];
    /* General things like last move ever used, etc. */
    context battlelong;

    struct QuitException {};
};

inline void BattleSituation::notify(int player, int command, int who)
{
    /* Doing that cuz we never know */
    testquit();

    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who);

    if (player == All) {
	emit battleInfo(id(Player1), tosend);
	emit battleInfo(id(Player2), tosend);
    } else if (player == AllButPlayer) {
	emit battleInfo(id(rev(who)), tosend);
    } else {
	emit battleInfo(id(player), tosend);
    }
}

template<class T>
void BattleSituation::notify(int player, int command, int who, const T& param)
{
    /* Doing that cuz we never know */
    testquit();

    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param;

    if (player == All) {
	emit battleInfo(id(Player1), tosend);
	emit battleInfo(id(Player2), tosend);
    } else if (player == AllButPlayer) {
	emit battleInfo(id(rev(who)), tosend);
    } else {
	emit battleInfo(id(player), tosend);
    }
}

template<class T1, class T2>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2)
{
    /* Doing that cuz we never know */
    testquit();

    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param1 << param2;

    if (player == All) {
	emit battleInfo(id(Player1), tosend);
	emit battleInfo(id(Player2), tosend);
    } else if (player == AllButPlayer) {
	emit battleInfo(id(rev(who)), tosend);
    } else {
	emit battleInfo(id(player), tosend);
    }
}

#endif // BATTLE_H
