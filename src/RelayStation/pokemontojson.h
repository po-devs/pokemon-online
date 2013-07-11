#ifndef POKEMONTOJSON_H
#define POKEMONTOJSON_H

#include <QVariantMap>

namespace Pokemon {class gen;}
class BattleConfiguration;
class ShallowBattlePoke;
class PokeBattle;
class TeamBattle;
class BattleMove;
class BattleChoice;
class BattleChoices;

QVariantMap toJson(const Pokemon::gen &gen);
QVariantMap toJson(const BattleConfiguration & conf);
QVariantMap toJson(const BattleChoices &choices);
QVariantMap toJson(const ShallowBattlePoke &poke);
QVariant toJson(const TeamBattle &team);
QVariantMap toJson(const PokeBattle &poke);
QVariantMap toJson(const BattleMove &move);
BattleChoice&& fromJson(const QVariantMap &map);
#endif // POKEMONTOJSON_H
