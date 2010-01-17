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

    b.pokelong[source]["AbilityArg"] = effect.arg;

    QString activationkey = QString("Ability%1SetUp").arg(effect.num);

    if (!b.pokelong[source].contains(activationkey)) {
        activate("UponSetup", num, source, source, b);
        b.pokelong[source][activationkey] = true;
    }
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

struct AMAngerPoint : public AM {
    AMAngerPoint() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (s != t && turn(b,t)["CriticalHit"].toBool()) {
            b.sendAbMessage(3,0,s);
            b.gainStatMod(s,Attack,12);
        }
    }
};

struct AMAnticipation : public AM {
    AMAnticipation() {
        functions["UponSetup"] = &us;
    }

    static void us (int s, int, BS &b) {
        int t = b.rev(s);
        if (b.koed(t)) {
            return;
        }
        bool frightening_truth = false;
        for (int i = 0; i < 4; i++) {
            if (TypeInfo::Eff(b.move(t, i), b.getType(s,1)) * TypeInfo::Eff(b.move(t, i), b.getType(s,2))) {
                frightening_truth = true;
            }
        }
        if (frightening_truth) {
            b.sendAbMessage(4,0,s);
        }
    }
};

struct AMArenaTrap : public AM {
    AMArenaTrap() {
        functions["IsItTrapped"] = &iit;
    }

    static void iit(int, int t, BS &b) {
        if (!b.isFlying(t)) {
            turn(b,t)["Trapped"] = true;
        }
    }
};

struct AMBadDreams : public AM {
    AMBadDreams() {
        functions["EndTurn"] = &et;
    }

    static void et (int s, int, BS &b) {
        int t = b.rev(s);
        if (!b.koed(t)) {
            if (b.poke(t).status() == Pokemon::Asleep) {
                b.sendAbMessage(6,0,s,t,Pokemon::Ghost);
                b.inflictDamage(t, b.poke(t).totalLifePoints()/8,s,false);
            }
        }
    }
};

struct AMBlaze : public AM {
    AMBlaze() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s, int, BS &b) {
        if (b.poke(s).lifePoints() <= b.poke(s).totalLifePoints()/3 && type(b,s) == poke(b,s)["AbilityArg"].toInt()) {
            turn(b,s)["BasePowerAbilityModifier"] = 5;
        }
    }
};

struct AMChlorophyll : public AM {
    AMChlorophyll() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["Stat3Modifier"] = 20;
        }
    }
};

struct AMColorChange : public AM {
    AMColorChange() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if ((s!=t) && type(b,t) != Pokemon::Curse) {
            int tp = type(b,t);
            if (poke(b,s)["Type2"].toInt() == Pokemon::Curse && tp == poke(b,s)["Type1"].toInt()) {
                return;
            }
            b.sendAbMessage(9,0,s,t,tp,tp);
            poke(b,s)["Type1"] = tp;
            poke(b,s)["Type2"] = Pokemon::Curse;
        }
    }
};

struct AMCompoundEyes : public AM {
    AMCompoundEyes() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int , BS &b) {
        turn(b,s)["Stat7AbilityModifier"] = 3;
    }
};

/* Events:
    UponPhysicalAssault
    DamageFormulaStart
    UponOffensiveDamageReceived
    UponSetup
    IsItTrapped
    EndTurn
    BasePowerModifier
    StatModifier
*/

#define REGISTER_AB(num, name) mechanics[num] = AM##name(); names[num] = #name; nums[#name] = num;

void AbilityEffect::init()
{
    REGISTER_AB(1, Adaptability);
    REGISTER_AB(2, Aftermath);
    REGISTER_AB(3, AngerPoint);
    REGISTER_AB(4, Anticipation);
    REGISTER_AB(5, ArenaTrap);
    REGISTER_AB(6, BadDreams);
    REGISTER_AB(7, Blaze);
    REGISTER_AB(8, Chlorophyll);
    REGISTER_AB(9, ColorChange);
    REGISTER_AB(10, CompoundEyes);
}
