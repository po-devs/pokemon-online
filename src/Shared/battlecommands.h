#ifndef BATTLECOMMANDS_H
#define BATTLECOMMANDS_H

namespace BattleCommands {
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
        Failed,
        BattleChat,
        MoveMessage,
        ItemMessage,
        NoOpponent,
        Flinch,
        Recoil,
        WeatherMessage,
        StraightDamage,
        AbilityMessage,
        AbsStatusChange,
        Substitute,
        BattleEnd,
        BlankMessage,
        CancelMove,
        Clause,
        DynamicInfo,
        DynamicStats,
        Spectating,
        SpectatorChat,
        AlreadyStatusMessage,
        ChangeTempPoke,
        ClockStart,
        ClockStop,
        Rated,
        TierSection,
        EndMessage,
        PointEstimate,
        StartChoices,
        Avoid,
        RearrangeTeam,
        SpotShifting
    };

    enum ChangeTempPoke {
        TempMove,
        TempAbility,
        TempItem,
        TempSprite,
        DefiniteForme,
        AestheticForme,
        DefMove,
        TempPP
    };

    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    enum WeatherM
    {
        ContinueWeather,
        EndWeather,
        HurtWeather
    };

};

#endif // BATTLECOMMANDS_H
