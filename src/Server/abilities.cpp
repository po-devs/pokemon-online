#include "abilities.h"
#include "../PokemonInfo/pokemoninfo.h"

typedef AbilityMechanics AM;
typedef BattleSituation BS;

QHash<int, AbilityMechanics> AbilityEffect::mechanics;
QHash<int, QString> AbilityEffect::names;
QHash<QString, int> AbilityEffect::nums;

void AbilityEffect::activate(const QString &effect, int num, int source, int target, BattleSituation &b)
{
    QList<ItemInfo::Effect> l = ItemInfo::Effects(num);

    foreach(ItemInfo::Effect e, l) {
        if (!mechanics.contains(e.num) || !mechanics[e.num].functions.contains(effect)) {
            continue;
        }
        mechanics[e.num].functions[effect](source, target, b);
    }
}

void AbilityEffect::setup(int num, int source, BattleSituation &b)
{
    qDebug() << "Setup required for " << num;
    QList<ItemInfo::Effect> effects = ItemInfo::Effects(num);

    foreach(ItemInfo::Effect effect, effects) {
        qDebug() << "Setting up " << effect.num;

        /* if the effect is invalid or not yet implemented then no need to go further */
        if (!mechanics.contains(effect.num)) {
            continue;
        }

        qDebug() << "Setup confirmed";

        //dun remove the test
        if (effect.args.size() > 0) {
            b.pokelong[source]["ItemArg"] = effect.args;
        }
    }
}

#define REGISTER_AB(num, name) mechanics[num] = AM##name(); names[num] = #name; nums[#name] = num;

void AbilityEffect::init()
{
}
