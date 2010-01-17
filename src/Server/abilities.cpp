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

    qDebug() << "Activating " << effect << " for ability " << num << " (effect " << e.num << ").";
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

struct AMAdaptability : public AM {
    AMAdaptability() {
        functions["DamageFormulaStart"] = &dfs;
    }

    static void dfs(int s, int, BS &b) {
        /* So the regular stab (3) will become 4 and no stab (2) will stay 2 */
        turn(b,s)["Stab"] = turn(b,s)["Stab"].toInt() * 4 / 3;
    }
};

struct AMAftermath : public AM {
    AMAftermath() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (b.koed(s) && !b.koed(t)) {
            b.sendAbMessage(2,0,s,t);
            b.inflictPercentDamage(t,25,s,false);
        }
    }
};

#define REGISTER_AB(num, name) mechanics[num] = AM##name(); names[num] = #name; nums[#name] = num;

void AbilityEffect::init()
{
    REGISTER_AB(1, Adaptability);
    REGISTER_AB(2, Aftermath);
}
