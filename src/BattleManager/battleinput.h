#ifndef BATTLEINPUT_H
#define BATTLEINPUT_H

#include "battlecommandmanager.h"
#include <vector>

class BattleConfiguration;

class BattleInput : public BattleCommandManager<BattleInput>
{
public:
    BattleInput(const BattleConfiguration *conf);

    void receiveData(QByteArray data);
    void dealWithCommandInfo(QDataStream&, uchar command,int spot);

    void pause();
    void unpause();

    bool delayed();


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

protected:
    int delayCount;
    std::vector<QByteArray> delayedCommands;
    unsigned mCount; /* Used to know the index in delayedCommands */
    const BattleConfiguration *conf;
};

#endif // BATTLEINPUT_H
