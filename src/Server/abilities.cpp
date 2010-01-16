#include "abilities.h"
#include "../PokemonInfo/pokemoninfo.h"

typedef AbilityMechanics AM;
typedef BattleSituation BS;

QHash<int, AbilityMechanics> AbilityEffect::mechanics;
QHash<int, QString> AbilityEffect::names;
QHash<QString, int> AbilityEffect::nums;

void AbilityEffect::activate(const QString &effect, int num, int source, int target, BattleSituation &b)
{
    AbilityInfo::Effect e = AbilityInfo::Effects(num);

    if (!mechanics.contains(e.num) || !mechanics[e.num].functions.contains(effect)) {
        return;
    }
    mechanics[e.num].functions[effect](source, target, b);
}

void AbilityEffect::setup(int num, int source, BattleSituation &b)
{
    qDebug() << "Setup required for " << num;
    AbilityInfo::Effect effect = AbilityInfo::Effects(num);

    qDebug() << "Setting up " << effect.num;

    /* if the effect is invalid or not yet implemented then no need to go further */
    if (!mechanics.contains(effect.num)) {
        return;
    }

    qDebug() << "Setup confirmed";

    //dun remove the test
    b.pokelong[source]["AbilityArg"] = effect.arg;
}

#define REGISTER_AB(num, name) mechanics[num] = AM##name(); names[num] = #name; nums[#name] = num;

void AbilityEffect::init()
{
}
