#ifndef BATTLEENUM_H
#define BATTLEENUM_H

enum class BattleEnum
{
    NewHp,
    Damaged,
    Ko,
    SendOut,
    SendBack,
    UseAttack,
    Turn,
    Hits,
    EffectiveNess,
    CriticalHit,
    Miss,
    Avoid,
    StatChange,
    ClassicStatusChange,
    AbsoluteStatusChange,
    AlreadyStatusMessage,
    StatusMessage,
    Fail,
    PlayerMessage,
    Spectator,
    SpectatorMessage,
    MoveMessage,
    NoTargetMessage,
    ItemMessage,
    Flinch,
    Recoil,
    Drained,
    WeatherMessage,
    EndWeather,
    WeatherDamage,
    AbilityMessage,
    SubstituteStatus,
    BattleEnd,
    BlankMessage,
    ClauseMessage,
    RatedInfo,
    TierInfo,
    StatBoostsAndField,
    PokemonVanish,
    PokemonReappear,
    SpriteChange,
    DefiniteFormeChange,
    CosmeticFormeChange,
    ClockStart,
    ClockStop,
    ShiftSpots
};

namespace battle {
enum class NetworkBattleCommands
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

}

inline uint qHash(const BattleEnum &b) {
    return int(b);
}

#endif // BATTLEENUM_H
