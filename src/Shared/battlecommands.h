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
        Spectating, // When designated a player, tells if the player disconnects/reconnects from the battle
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
        SpotShifting,
        ChoiceMade, /* BattleChoice in parameter. May be used by server to remind players of the choice they made. More importantly, used
                     in replays (battle window store choice mades that way) */
        UseItem,
        ItemCountChange,
        CappedStat
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
