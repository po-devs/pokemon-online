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
    StatusFeel,
    StatusFree,
    StatusHurt,
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

inline uint qHash(const BattleEnum &b) {
    return int(b);
}

#endif // BATTLEENUM_H
