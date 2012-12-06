#ifndef POKEMONTOJSON_H
#define POKEMONTOJSON_H

#include <QVariantMap>

namespace Pokemon {class gen;}
class BattleConfiguration;

QVariantMap toJson(const Pokemon::gen &gen);
QVariantMap toJson(const BattleConfiguration & conf);

#endif // POKEMONTOJSON_H
