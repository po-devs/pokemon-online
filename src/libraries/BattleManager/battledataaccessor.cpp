#include <PokemonInfo/battlestructs.h>

#include "battledataaccessor.h"
#include "teamdata.h"
#include "battledata.h"

MoveProxy::MoveProxy(BattleMove *move) :hasOwnerShip(false), moveData(move)
{

}

MoveProxy::MoveProxy() : hasOwnerShip(true), moveData(new BattleMove())
{

}

MoveProxy::~MoveProxy()
{
    if (hasOwnerShip) {
        delete moveData;
    }
}

void MoveProxy::adaptTo(const BattleMove *move)
{
    setNum(move->num());
    changePP(move->PP());
}

Pokemon::gen MoveProxy::gen() const
{
    return master()->gen();
}

BattleMove &MoveProxy::exposedData()
{
    return *moveData;
}

const BattleMove &MoveProxy::exposedData() const
{
    return *moveData;
}

PokeProxy * MoveProxy::master() const
{
    return dynamic_cast<PokeProxy*>(parent());
}

void MoveProxy::setNum(int newnum) {
    if (newnum == num()) {
        return;
    }
    d()->num() = newnum;
    d()->totalPP() = MoveInfo::PP(newnum, gen()) * (newnum == Move::TrumpCard ? 5 :8)/5; /* 3 PP-ups */;
    emit numChanged();
}

void MoveProxy::changePP(int newPP) {
    if (newPP == d()->PP()) {
        return;
    }
    d()->PP() = newPP;
    emit PPChanged();
}

PokeProxy::PokeProxy() : hasOwnerShip(true), pokeData(new ShallowBattlePoke())
{
    if (hasExposedData()) {
        for (int i = 0; i < 4; i++) {
            moves.push_back(new MoveProxy(&dd()->move(i)));
            moves.last()->setParent(this);
        }
    } else {
        for (int i = 0; i < 4; i++) {
            moves.push_back(new MoveProxy());
            moves.last()->setParent(this);
        }
    }
}

PokeProxy::PokeProxy(ShallowBattlePoke *pokemon) : hasOwnerShip(false), pokeData(pokemon)
{
    if (hasExposedData()) {
        for (int i = 0; i < 4; i++) {
            moves.push_back(new MoveProxy(&dd()->move(i)));
            moves.last()->setParent(this);
        }
    } else {
        for (int i = 0; i < 4; i++) {
            moves.push_back(new MoveProxy());
            moves.last()->setParent(this);
        }
    }
}

PokeProxy::~PokeProxy()
{
    if (hasOwnerShip) {
        delete pokeData;
    }
}

void PokeProxy::adaptTo(const ShallowBattlePoke *pokemon, bool soft) {
    const PokeBattle *trans = dynamic_cast<const PokeBattle*>(pokemon);
    if (trans) {
        adaptTo(trans);
    }
    if (*pokemon == *pokeData) {
        return;
    }
    if (soft) {
        pokeData->changeStatus(pokemon->status());
        return;
    }
    /* Could be more granular, change if it matters */
    *pokeData = *pokemon;
    if (pokeData->lifePercent() != pokemon->lifePercent()) {
        pokeData->setLifePercent(pokemon->lifePercent());
    }
    emitReset();
}

void PokeProxy::adaptTo(const PokeBattle *pokemon) {
    PokeBattle *trans = dynamic_cast<PokeBattle*>(pokeData);

    for (int i = 0; i < 4; i++) {
        move(i)->adaptTo(&pokemon->move(i));
    }

    if (trans) {
        *trans = *pokemon;
    } else {
        if (*pokeData == *(static_cast<const ShallowBattlePoke*>(pokemon))) {
            return;
        }
        *pokeData = *pokemon;
    }

    /* Could be more granular, change if it matters */
    emitReset();
}

void PokeProxy::emitReset()
{
    emit numChanged(); emit statusChanged(); emit lifeChanged();
    emit pokemonReset();
}

TeamProxy *PokeProxy::master() const
{
    return dynamic_cast<TeamProxy*>(parent());
}

Pokemon::gen PokeProxy::gen() const
{
    return master()->gen();
}

void PokeProxy::changeStatus(int fullStatus)
{
    if (d()->status() == fullStatus) {
        return;
    }
    d()->changeStatus(fullStatus);
    emit statusChanged();
}

void PokeProxy::setLife(int newLife)
{
    if (d()->life() == newLife) {
        return;
    }
    d()->setLife(newLife);
    emit lifeChanged();
}

void PokeProxy::setNum(Pokemon::uniqueId num){
    if (d()->num() == num) {
        return;
    }
    d()->num() = num;
    emit numChanged();
}

void PokeProxy::setItem(int newItem)
{
    if (!hasExposedData() || dd()->item() == newItem) {
        return;
    }
    dd()->item() = newItem;
    emit itemChanged();
}

int PokeProxy::basestat(int stat) const
{
    if (hasExposedData())
        return dd()->normalStat(stat);
    else
        return 0;
}

int PokeProxy::iv(int stat) const
{
    if (stat >= 0 && stat < dvs().length()) {
        return dvs()[stat];
    } else {
        return 0;
    }
}

int PokeProxy::ev(int stat) const
{
    if (stat >= 0 && stat < evs().length()) {
        return evs()[stat];
    } else {
        return 0;
    }
}

TeamProxy::TeamProxy()
{
    mTime = 300; mTicking = false;
    /* We still require full pokemon, which we will build up */
    teamData = new TeamData(NULL, true);
    for (int i = 0; i < 6; i++) {
        dynamic_cast<PokeBattle*>(teamData->poke(i))->totalLifePoints() = 100;
        pokemons.push_back(new PokeProxy(teamData->poke(i)));
        pokemons.last()->setParent(this);
    }
    hasOwnerShip = true;
}

TeamProxy::TeamProxy(TeamData *teamData) : teamData(teamData), hasOwnerShip(false)
{
    mTime = 300; mTicking = false;
    for (int i = 0; i < 6; i++) {
        pokemons.push_back(new PokeProxy(teamData->poke(i)));
        pokemons.last()->setParent(this);
    }
    hasOwnerShip = false;
}

TeamProxy::~TeamProxy()
{
    if (hasOwnerShip) {
        delete teamData;
    }
}

QString TeamProxy::name() const {
    return teamData->name();
}

void TeamProxy::switchPokemons(int index, int prevIndex)
{
    std::swap(pokemons[index], pokemons[prevIndex]);
    teamData->switchPokemons(index, prevIndex);

    emit pokemonsSwapped(index, prevIndex);
}

bool TeamProxy::ticking() const
{
    return mTicking;
}

int TeamProxy::time() const
{
    return mTime;
}

void TeamProxy::setTimeLeft(int time, bool ticking)
{
    mTicking = ticking;
    if (mTime == time) {
        return;
    }
    mTime = time;
    emit timeChanged();
}

void TeamProxy::setPoke(int index, ShallowBattlePoke *pokemon, bool soft)
{
    poke(index)->adaptTo(pokemon, soft);
}

void TeamProxy::setName(const QString &name)
{
    teamData->name() = name;
}

void TeamProxy::setTeam(const TeamBattle *team)
{
    for (int i = 0; i < 6; i++) {
        poke(i)->adaptTo(&team->poke(i));
    }
    teamData->setGen(team->gen);
    teamData->setItems(team->items);
}

QHash<quint16,quint16> &TeamProxy::items() {
    return teamData->items();
}

Pokemon::gen TeamProxy::gen() const
{
    return teamData->gen();
}

void TeamProxy::removeItem(int item)
{
    teamData->removeItem(item);
}

void TeamProxy::changeItemCount(int item, int count)
{
    teamData->changeItemCount(item, count);
}
