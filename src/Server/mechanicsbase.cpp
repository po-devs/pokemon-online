#include "mechanicsbase.h"

BattleBase::TurnMemory & PureMechanicsBase::fturn(BattleBase &b, int player)
{
    return b.turnMem(player);
}

BattleBase::context & PureMechanicsBase::poke(BattleBase &b, int player)
{
    return b.pokeMemory(player);
}

BattleBase::BasicPokeInfo & PureMechanicsBase::fpoke(BattleBase &b, int player)
{
    return b.fpoke(player);
}

BattleBase::context & PureMechanicsBase::turn(BattleBase &b, int player)
{
    return b.turnMemory(player);
}

int PureMechanicsBase::type(BattleBase &b, int source)
{
    return tmove(b, source).type;
}

int PureMechanicsBase::move(BattleBase &b, int source)
{
    return tmove(b, source).attack;
}

BattleBase::BasicMoveInfo & PureMechanicsBase::tmove(BattleBase &b, int source)
{
    return b.tmove(source);
}

void PureMechanicsBase::initMove(int num, Pokemon::gen gen, BattleBase::BasicMoveInfo &data)
{

    /* Different steps: critical raise, number of times, ... */
    data.critRaise = MoveInfo::CriticalRaise(num, gen);
    data.repeatMin = MoveInfo::RepeatMin(num, gen);
    data.repeatMax = MoveInfo::RepeatMax(num, gen);
    data.priority = MoveInfo::SpeedPriority(num, gen);
    data.flags = MoveInfo::Flags(num, gen);
    data.power = MoveInfo::Power(num, gen);
    data.accuracy = MoveInfo::Acc(num, gen);
    data.type = MoveInfo::Type(num, gen);
    data.category = MoveInfo::Category(num, gen);
    data.rate = MoveInfo::EffectRate(num, gen);
    //(*this)["StatEffect"] = MoveInfo::Effect(num, gen);
    data.flinchRate = MoveInfo::FlinchRate(num, gen);
    data.recoil = MoveInfo::Recoil(num, gen);
    data.attack = num;
    data.targets = MoveInfo::Target(num, gen);
    data.healing = MoveInfo::Healing(num, gen);
    data.classification = MoveInfo::Classification(num, gen);
    data.status = MoveInfo::Status(num, gen);
    data.statusKind = MoveInfo::StatusKind(num, gen);
    data.minTurns = MoveInfo::MinTurns(num, gen);
    data.maxTurns = MoveInfo::MaxTurns(num, gen);
    data.statAffected = MoveInfo::StatAffected(num, gen);
    data.boostOfStat = MoveInfo::BoostOfStat(num, gen);
    data.rateOfStat = MoveInfo::RateOfStat(num, gen);
    data.kingRock = MoveInfo::FlinchByKingRock(num, gen);
}
