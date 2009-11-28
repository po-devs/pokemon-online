#include "moves.h"
#include "../PokemonInfo/pokemoninfo.h"

MoveEffect::MoveEffect(int num)
{
    QString eff = MoveInfo::Effect(num);

    /* Different steps: critical raise, number of times, ... */
    (*this)["CriticalRaise"] = MoveInfo::CriticalRaise(num);
    (*this)["RepeatMin"] = MoveInfo::RepeatMin(num);
    (*this)["RepeatMax"] = MoveInfo::RepeatMax(num);
    (*this)["SpeedPriority"] = MoveInfo::SpeedPriority(num);
    (*this)["PhysicalContact"] = MoveInfo::PhysicalContact(num);
    (*this)["KingRock"] = MoveInfo::KingRock(num);
    (*this)["Power"] = MoveInfo::Power(num);
    (*this)["Accuracy"] = MoveInfo::Acc(num);
    (*this)["Type"] = MoveInfo::Type(num);
    (*this)["Category"] = MoveInfo::Category(num);
    (*this)["EffectRate"] = MoveInfo::EffectRate(num);
    (*this)["StatEffect"] = MoveInfo::Effect(num);
    (*this)["FlinchRate"] = MoveInfo::FlinchRate(num);
    (*this)["Recoil"] = MoveInfo::Recoil(num);
}
