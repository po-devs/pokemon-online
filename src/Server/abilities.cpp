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
        turn(b,s)["Stat7AbilityModifier"] = 6;
    }
};


struct AMCuteCharm : public AM {
    AMCuteCharm() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (!b.koed(t) && b.isSeductionPossible(s,t) && rand() % 100 < 30 && !poke(b,t).contains("AttractedTo")) {
            poke(b,t)["AttractedTo"] = s;
            poke(b,s)["Attracted"] = t;
            addFunction(poke(b,t), "DetermineAttackPossible", "Attract", &pda);
            b.sendAbMessage(11,0,s,t);
            if (b.hasWorkingItem(s, 17)) /* mental herb*/ {
                b.sendItemMessage(7,s);
                b.disposeItem(t);
                poke(b,t).remove("Attracted");
            }
        }
    }

    static void pda(int s, int, BS &b) {
        if (poke(b,s).contains("AttractedTo")) {
            int seducer = poke(b,s)["AttractedTo"].toInt();
            if (poke(b,seducer).contains("Attracted") && poke(b,seducer)["Attracted"].toInt() == s) {
                b.sendMoveMessage(58,0,s,0,seducer);
                if (rand() % 2 == 0) {
                    turn(b,s)["ImpossibleToMove"] = true;
                    b.sendMoveMessage(58, 2,s);
                }
            }
        }
    }
};

struct AMDownload : public AM {
    AMDownload() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        int t = b.rev(s);
        b.sendAbMessage(12,0,s);
        if (b.koed(t) || b.getStat(t, Defense) > b.getStat(t, SpDefense)) {
            b.gainStatMod(s, SpAttack,1);
        } else {
            b.gainStatMod(s, Attack,1);
        }
    }
};

struct AMDrizzle : public AM {
    AMDrizzle() {
        functions["UponSetup"] = &us;
    }

    static void us (int s, int , BS &b) {
        b.callForth(poke(b,s)["AbilityArg"].toInt(), -1);
    }
};

struct AMDrySkin : public AM {
    AMDrySkin() {
        functions["BasePowerFoeModifier"] = &bpfm;
        functions["WeatherSpecial"] = &ws;
        functions["OpponentBlock"] = &oa;
    }

    static void bpfm(int , int t, BS &b) {
        if (type(b,t) == Pokemon::Fire) {
            turn(b,t)["BasePowerFoeAbilityModifier"] = 5;
        }
    }

    static void oa(int s, int t, BS &b) {
        if (type(b,t) == Pokemon::Water) {
            turn(b,s)[QString("Block%1").arg(t)] = true;
            b.sendAbMessage(15,0,s,s,Pokemon::Water);
            b.healLife(s, b.poke(s).totalLifePoints()/4);
        }
    }

    static void ws (int s, int , BS &b) {
        if (b.isWeatherWorking(BattleSituation::Rain)) {
            b.sendAbMessage(15,1,s,s,Pokemon::Water);
            b.healLife(s, b.poke(s).totalLifePoints()/8);
        } else if (b.isWeatherWorking(BattleSituation::Rain)) {
            b.sendAbMessage(15,2,s,s,Pokemon::Fire);
            b.inflictDamage(s, b.poke(s).totalLifePoints()/8, s, false);
        }
    }
};

struct AMEffectSpore : public AM {
    AMEffectSpore() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (b.poke(t).status() == Pokemon::Fine && rand() % 100 < 30) {
            if (rand() % 3 == 0) {
                if (b.canGetStatus(t,Pokemon::Asleep)) {
                    b.sendAbMessage(16,0,s,t,Pokemon::Grass);
                    b.inflictStatus(t, Pokemon::Asleep,s);
                }
            } else if (rand() % 1 == 0) {
                if (b.canGetStatus(t,Pokemon::Paralysed)) {
                    b.sendAbMessage(16,0,s,t,Pokemon::Electric);
                    b.inflictStatus(t, Pokemon::Paralysed,s);
                }
            } else {
                if (b.canGetStatus(t,Pokemon::Poisoned)) {
                    b.sendAbMessage(16,0,s,t,Pokemon::Poison);
                    b.inflictStatus(t, Pokemon::Poisoned,s);
                }
            }
        }
    }
};

struct AMFlameBody : public AM {
    AMFlameBody() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (b.poke(t).status() == Pokemon::Fine && rand() % 100 < 30) {
            if (b.canGetStatus(t,poke(b,s)["AbilityArg"].toInt())) {
                b.sendAbMessage(18,0,s,t,Pokemon::Curse,b.ability(s));
                b.inflictStatus(t, poke(b,s)["AbilityArg"].toInt(),s);
            }
        }
    }
};

struct AMFlashFire : public AM {
    AMFlashFire() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if (type(b,t) == Pokemon::Fire) {
            turn(b,s)[QString("Block%1").arg(t)] = true;
            b.sendAbMessage(19,0,s,s,Pokemon::Fire);
            poke(b,s)["FlashFired"] = true;
        }
    }
};

struct AMFlowerGift : public AM {
    AMFlowerGift() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Sunny)) {
            turn(b,s)["Stat1AbilityModifier"] = 10;
            turn(b,s)["Stat5AbilityModifier"] = 10;
        }
    }
};

struct AMForeCast : public AM {
    AMForeCast() {
        functions["UponSetup"] = &us;
        functions["WeatherChange"] = &us;
    }

    static void us(int s, int, BS &b) {
        int tp = TypeInfo::TypeForWeather(b.weather());

        if (poke(b,s)["Type2"].toInt() == Pokemon::Curse && tp == poke(b,s)["Type1"].toInt()) {
            return;
        }
        b.sendAbMessage(21,0,s,s,tp);
        poke(b,s)["Type1"] = tp;
        poke(b,s)["Type2"] = Pokemon::Curse;
    }
};

struct AMForeWarn : public AM {
    AMForeWarn() {
        functions["UponSetup"] = &us;
    }

    struct special_moves : public QHash<int,int> {
        special_moves() {
            (*this)[133] = (*this)[166] = (*this)[186] = (*this)[353] = 160;
            (*this)[70] = (*this)[241] = (*this)[251] = 120;
        }
    };

    static special_moves SM;

    static void us(int s, int, BS &b) {
        int t = b.rev(s);

        if (!b.koed(t)) {
            int max = 0;
            std::vector<int> poss;

            for (int i = 0; i < 4; i++) {
                int m = b.move(t,i);
                if (m !=0) {
                    int pow;
                    if (SM.contains(m)) {
                        pow = SM[m];
                    } else if (MoveInfo::Power(m) == 1) {
                        pow = 80;
                    } else {
                        pow = MoveInfo::Power(m);
                    }

                    if (pow > max) {
                        poss.clear();
                        poss.push_back(m);
                    } else if (pow == max) {
                        poss.push_back(m);
                    }
                }
            }

            int m = poss[rand()%poss.size()];

            b.sendAbMessage(22,0,s,t,MoveInfo::Type(m),m);
        }
    }
};

AMForeWarn::special_moves AMForeWarn::SM;

struct AMFrisk : public AM {
    AMFrisk() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        int t = b.rev(s);

        if (b.koed(t))
            return;


        int it = b.poke(t).item();

        if (it != 0) {
            b.sendAbMessage(23,0,s,t,0,it);
        }
    }
};

struct AMGuts : public AM {
    AMGuts() {
        functions["StatModifier"] = &sm;
    }

    static void sm (int s, int, BS &b) {
        if (b.poke(s).status() != Pokemon::Fine) {
            turn(b,s)[QString("Stat%1AbilitiyModifier").arg(poke(b,s)["AbilityArg"].toInt())] = 10;
        }
    }
};

struct AMHeatProof : public AM {
    AMHeatProof() {
        functions["BasePowerFoeModifier"] = &bpfm;
    }

    static void bpfm(int , int t, BS &b) {
        if (type(b,t) == Pokemon::Fire) {
            turn(b,t)["BasePowerFoeAbilityModifier"] = -10;
        }
    }
};

struct AMHugePower : public AM {
    AMHugePower() {
        functions["StatModifier"] = &sm;
    }

    static void sm (int s, int, BS &b) {
        turn(b,s)["Stat1AbilitiyModifier"] = 20;
    }
};

struct AMHustle : public AM {
    AMHustle() {
        functions["StatModifier"] = &sm;
    }

    static void sm (int s, int, BS &b) {
        turn(b,s)["Stat1AbilitiyModifier"] = 10;
        if (turn(b,s)["Category"].toInt() == Move::Physical) {
            turn(b,s)["Stat7AbilityModifier"] = -4;
        }
    }
};

struct AMHydration : public AM {
    AMHydration() {
        functions["WeatherSpecial"] = &ws;
    }

    static void ws(int s, int, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Rain) && b.poke(s).status() != Pokemon::Fine) {
            b.sendAbMessage(29,0,s,s,Pokemon::Water);
            b.healStatus(s, b.poke(s).status());
        }
    }
};

struct AMHyperCutter : public AM {
    AMHyperCutter() {
        functions["PreventStatChange"] = &psc;
    }

    static void psc(int s, int t, BS &b) {
        if (turn(b,s)["StatModType"].toString() == "Stat" && turn(b,s)["StatModded"].toInt() == Attack && turn(b,s)["StatModification"].toInt() < 0) {
            if (b.canSendPreventMessage(s,t))
                b.sendAbMessage(30,0,s,s,0,b.ability(s));
            b.preventStatMod(s,t);
        }
    }
};

struct AMClearBody : public AM {
    AMClearBody() {
        functions["PreventStatChange"] = &psc;
    }

    static void psc(int s, int t, BS &b) {
        if (turn(b,s)["StatModType"].toString() == "Stat" && turn(b,s)["StatModification"].toInt() < 0) {
            if (b.canSendPreventMessage(s,t))
                b.sendAbMessage(31,0,s,s,0,b.poke(s).ability());
            b.preventStatMod(s,t);
        }
    }
};

struct AMIceBody : public AM {
    AMIceBody() {
        functions["WeatherSpecial"] = &ws;
    }

    static void ws(int s, int, BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["WeatherSpecialed"] = true; //to prevent being hit by the weather
            b.sendAbMessage(32,0,s,s,TypeInfo::TypeForWeather(poke(b,s)["AbilityArg"].toInt()),b.ability(s));
            b.healLife(s, b.poke(s).totalLifePoints()/16);
        }
    }
};

struct AMInsomnia : public AM {
    AMInsomnia() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        if (b.poke(s).status() == poke(b,s)["AbilityArg"].toInt()) {
            b.sendAbMessage(33,0,s,s,Pokemon::Dark,b.ability(s));
            b.healStatus(s, b.poke(s).status());
        }
    }
};

struct AMIntimidate : public AM {
    AMIntimidate() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        int t = b.rev(s);
        if (b.koed(t))
            return;

        b.sendAbMessage(34,0,s,t);
        b.loseStatMod(t,Attack,1,s);
    }
};

struct AMIronFist : public AM {
    AMIronFist() {
        functions["BasePowerModifier"] = &bpm;
    }

    struct PunchingMoves : public QSet<int> {
        PunchingMoves() {
            (*this) << 50 << 61 << 91 << 105 << 108 << 131 << 145 << 171 << 197 << 226 << 238 << 244 << 350 << 362 << 426;
        }
    };

    static PunchingMoves PM;

    static void bpm (int s, int , BS &b) {
        if (PM.contains(move(b,s))) {
            turn(b,s)["BasePowerAbilityModifier"] = 2;
        }
    }
};

AMIronFist::PunchingMoves AMIronFist::PM;

struct AMLeafGuard  : public AM {
    AMLeafGuard() {
        functions["PreventStatChange"]= &psc;
    }

    static void psc(int s, int t, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Sunny) && turn(b,s)["StatModType"].toString() == "Status") {
            if (b.canSendPreventMessage(s,t))
                b.sendAbMessage(37,0,s,s,0,b.poke(s).ability());
            b.preventStatMod(s,t);
        }
    }
};

struct AMMagnetPull : public AM {
    AMMagnetPull() {
        functions["IsItTrapped"] = &iit;
    }

    static void iit(int, int t, BS &b) {
        if (b.hasType(t, Pokemon::Steel)) {
            turn(b,t)["Trapped"] = true;
        }
    }
};

struct AMMoldBreaker : public AM {
    AMMoldBreaker() {
        functions["UponSetup"] = &us;
    }

    static void us (int s, int, BS &b) {
        b.sendAbMessage(40,0,s);
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
    BasePowerFoeModifier
    StatModifier
    WeatherSpecial
    WeatherChange
    OpponentBlock
    PreventStatChange
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
    REGISTER_AB(11, CuteCharm);
    REGISTER_AB(13, Download);
    REGISTER_AB(14, Drizzle);
    REGISTER_AB(15, DrySkin);
    REGISTER_AB(16, EffectSpore);
    REGISTER_AB(18, FlameBody);
    REGISTER_AB(19, FlashFire);
    REGISTER_AB(20, FlowerGift);
    REGISTER_AB(21, ForeCast);
    REGISTER_AB(22, ForeWarn);
    REGISTER_AB(23, Frisk);
    //Gluttony
    REGISTER_AB(25, Guts);
    REGISTER_AB(26, HeatProof);
    REGISTER_AB(27, HugePower);
    REGISTER_AB(28, Hustle);
    REGISTER_AB(29, Hydration);
    REGISTER_AB(30, HyperCutter);
    REGISTER_AB(31, ClearBody);
    REGISTER_AB(32, IceBody);
    REGISTER_AB(33, Insomnia);
    REGISTER_AB(34, Intimidate);
    REGISTER_AB(35, IronFist);
    REGISTER_AB(37, LeafGuard);
    REGISTER_AB(39, MagnetPull);
    REGISTER_AB(40, MoldBreaker);
}
