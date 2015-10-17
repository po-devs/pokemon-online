#ifndef POKEMONTOJSON_H
#define POKEMONTOJSON_H

#include <QVariantMap>

#include <PokemonInfo/battlestructs.h>

namespace Pokemon {class gen;}
class BattleConfiguration;
class ShallowBattlePoke;
class PokeBattle;
class TeamBattle;
class BattleMove;
class BattleChoice;
class BattleChoices;
class ShallowShownTeam;
class ShallowShownPoke;
class TrainerInfo;

QVariantMap toJson(const Pokemon::gen &gen);
QVariantMap toJson(const BattleConfiguration & conf);
QVariantMap toJson(const BattleChoices &choices);
QVariantMap toJson(const ShallowBattlePoke &poke);
QVariant toJson(const ShallowShownTeam &team);
QVariantMap toJson(const ShallowShownPoke &poke);
QVariant toJson(const TeamBattle &team);
QVariantMap toJson(const PokeBattle &poke);
QVariantMap toJson(const BattleMove &move);

template <class T>
T fromJson(const QVariantMap &map);

template <>
BattleChoice fromJson<BattleChoice>(const QVariantMap &v);

template <>
TrainerInfo fromJson<TrainerInfo>(const QVariantMap &map);

template <>
PersonalTeam fromJson<PersonalTeam>(const QVariantMap &map);

template <>
PokePersonal fromJson<PokePersonal>(const QVariantMap &map);

template <>
Pokemon::gen fromJson<Pokemon::gen>(const QVariantMap &map);

#endif // POKEMONTOJSON_H
