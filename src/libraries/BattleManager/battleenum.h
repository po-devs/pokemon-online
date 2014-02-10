#ifndef BATTLEENUM_H
#define BATTLEENUM_H

enum /* class */ BattleEnum
{
    NewHp, //0
    Damaged,
    Ko,
    SendOut,
    SendBack,
    UseAttack,
    Turn,
    Hits,
    Effectiveness,
    CriticalHit,
    Miss, //10
    Avoid,
    StatChange,
    ClassicStatusChange,
    AbsoluteStatusChange,
    AlreadyStatusMessage,
    StatusFeel,
    StatusFree,
    StatusHurt,
    Fail,
    PlayerMessage, //20
    SpectatorEnter,
    SpectatorLeave,
    SpectatorMessage,
    MoveMessage,
    NoTargetMessage,
    ItemMessage,
    Flinch,
    Recoil,
    Drained,
    StartWeather, //30
    WeatherMessage,
    EndWeather,
    WeatherDamage,
    AbilityMessage,
    SubstituteStatus,
    BattleEnd,
    BlankMessage,
    ClauseMessage,
    RatedInfo,
    TierInfo, //40
    StatBoostsAndField,
    PokemonVanish,
    PokemonReappear,
    SpriteChange,
    DefiniteFormeChange,
    CosmeticFormeChange,
    ClockStart,
    ClockStop,
    ShiftSpots,
    PPChange, //50
    OfferChoice,
    TempPPChange,
    MoveChange,
    RearrangeTeam,
    ChoiceSelection,
    ChoiceCancelled,
    Variation,
    DynamicStats,
    PrintHtml,
    Reconnect, //60
    Disconnect,
    /* Player choices, that the player actually make (as opposed to choice offered by the server) */
    ChooseAttack,
    ChooseSwitch,
    ChooseRearrangeTeam,
    ChooseCancel,
    ChooseShiftToCenter,
    ChooseDraw,
    UseItem,
    ItemCountChange,
    WontGoHigher,
    WontGoLower
};

inline unsigned int qHash(const BattleEnum &b) {
    return int(b);
}

#endif // BATTLEENUM_H
