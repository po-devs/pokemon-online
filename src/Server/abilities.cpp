#include "abilities.h"
#include "../PokemonInfo/pokemoninfo.h"

typedef AbilityMechanics AM;
typedef BattleSituation BS;

QTSHash<int, AbilityMechanics> AbilityEffect::mechanics;
QTSHash<int, QString> AbilityEffect::names;
QTSHash<QString, int> AbilityEffect::nums;

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
    AbilityInfo::Effect effect = AbilityInfo::Effects(num);

    /* if the effect is invalid or not yet implemented then no need to go further */
    if (!mechanics.contains(effect.num)) {
        return;
    }

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
            if (TypeInfo::Eff(MoveInfo::Type(b.move(t, i)), b.getType(s,1)) * TypeInfo::Eff(MoveInfo::Type(b.move(t, i)), b.getType(s,2)) > 4) {
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
            turn(b,s)["BasePowerAbilityModifier"] = 10;
        }
    }
};

struct AMChlorophyll : public AM {
    AMChlorophyll() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["Stat3AbilityModifier"] = 20;
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
        if (!b.koed(t) && b.isSeductionPossible(s,t) && true_rand() % 100 < 30 && !poke(b,t).contains("AttractedTo")) {
            poke(b,t)["AttractedTo"] = s;
            poke(b,s)["Attracted"] = t;
            addFunction(poke(b,t), "DetermineAttackPossible", "Attract", &pda);
            b.sendAbMessage(11,0,s,t);
            if (b.hasWorkingItem(s, Item::MentalHerb)) /* mental herb*/ {
                b.sendItemMessage(7,s);
                b.disposeItem(t);
                poke(b,t).remove("Attracted");
            }
        }
    }

    static void pda(int s, int, BS &b) {
        if (turn(b,s).value("HasPassedStatus").toBool())
            return;
        if (poke(b,s).contains("AttractedTo")) {
            int seducer = poke(b,s)["AttractedTo"].toInt();
            if (poke(b,seducer).contains("Attracted") && poke(b,seducer)["Attracted"].toInt() == s) {
                b.sendMoveMessage(58,0,s,0,seducer);
                if (true_rand() % 2 == 0) {
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
        int w = poke(b,s)["AbilityArg"].toInt();
        if (w != b.weather()) {
            int type = (w == BS::Hail ? Type::Ice : (w == BS::Sunny ? Type::Fire : (w == BS::SandStorm ? Type::Rock : Type::Water)));
            b.sendAbMessage(14,w-1,s,s,type);
        }
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
            b.sendAbMessage(15,0,s,s,Pokemon::Water);
            b.healLife(s, b.poke(s).totalLifePoints()/8);
        } else if (b.isWeatherWorking(BattleSituation::Sunny)) {
            b.sendAbMessage(15,1,s,s,Pokemon::Fire);
            b.inflictDamage(s, b.poke(s).totalLifePoints()/8, s, false);
        }
    }
};

struct AMEffectSpore : public AM {
    AMEffectSpore() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (b.poke(t).status() == Pokemon::Fine && true_rand() % 100 < 30) {
            if (true_rand() % 3 == 0) {
                if (b.canGetStatus(t,Pokemon::Asleep)) {
                    b.sendAbMessage(16,0,s,t,Pokemon::Grass);
                    b.inflictStatus(t, Pokemon::Asleep,s);
                }
            } else if (true_rand() % 1 == 0) {
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

            int m = poss[true_rand()%poss.size()];

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
        /* Guts doesn't activate on a sleeping poke that used Rest (but other ways of sleeping
            are activated */
        if (b.poke(s).status() != Pokemon::Fine && !(b.poke(s).status() == Pokemon::Asleep && poke(b,s).value("Rested").toBool())) {
            turn(b,s)[QString("Stat%1AbilityModifier").arg(poke(b,s)["AbilityArg"].toInt())] = 10;
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
        turn(b,s)["Stat1AbilityModifier"] = 20;
    }
};

struct AMHustle : public AM {
    AMHustle() {
        functions["StatModifier"] = &sm;
    }

    static void sm (int s, int, BS &b) {
        turn(b,s)["Stat1AbilityModifier"] = 10;
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
        if (turn(b,s)["StatModType"].toString() == "Stat" && turn(b,s)["StatModded"].toInt() == poke(b,s)["AbilityArg"].toInt() && turn(b,s)["StatModification"].toInt() < 0) {
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
        if (b.hasSubstitute(t)) {
            b.sendAbMessage(34,1,s,t);
        } else {
            b.loseStatMod(t,Attack,1,s);
        }
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
            turn(b,s)["BasePowerAbilityModifier"] = 4;
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

struct AMMotorDrive : public AM {
    AMMotorDrive() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if (type(b,t) == Type::Electric) {
            turn(b,s)[QString("Block%1").arg(t)] = true;
            b.sendAbMessage(41,0,s,s,Pokemon::Electric);
            b.gainStatMod(s,Speed,1);
        }
    }
};

struct AMNaturalCure : public AM {
    AMNaturalCure () {
        functions["UponSwitchOut"] = &uso;
    }

    static void uso (int s, int, BS &b) {
        b.healStatus(s, b.poke(s).status());
    }
};

struct AMNormalize : public AM {
    AMNormalize() {
        functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
        if (turn(b,s)["Type"].toInt() != Type::Curse)
            turn(b,s)["Type"] = Type::Normal;
    }
};

struct AMOwnTempo : public AM {
    AMOwnTempo() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int, BS &b) {
        if (b.isConfused(s)) {
            b.sendAbMessage(44,0,s);
            b.healConfused(s);
        }
    }
};

struct AMPressure : public AM {
    AMPressure() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int, BS &b) {
        b.sendAbMessage(46,0,s);
    }
};

struct AMReckless : public AM {
    AMReckless() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm (int s, int , BS &b) {
        int mv = move(b,s);
        //Jump kicks
        if (turn(b,s).value("Recoil").toInt() > 0 || mv == 183 || mv == 207) {
            turn(b,s)["BasePowerAbilityModifier"] = 4;
        }
    }
};

struct AMRivalry : public AM {
    AMRivalry() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm (int s, int t, BS &b) {
        if (b.poke(s).gender() == Pokemon::Neutral || b.poke(t).gender() == Pokemon::Neutral)
            return;
        if (b.poke(s).gender() == b.poke(t).gender())
            turn(b,s)["BasePowerAbilityModifier"] = 5;
        else
            turn(b,s)["BasePowerAbilityModifier"] = -5;
    }
};

struct AMRoughSkin : public AM {
    AMRoughSkin() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa( int s, int t, BS &b) {
        if (!b.koed(t)) {
            b.sendAbMessage(50,0,s,t);
            b.inflictDamage(t,b.poke(t).totalLifePoints()/8,s,false);
        }
    }
};

struct AMSandVeil : public AM {
    AMSandVeil() {
        functions["StatModifier"] = &sm;
        functions["WeatherSpecial"] = &ws;
    }

    static void sm (int s, int , BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["Stat6AbilityModifier"] = 4;
        }
    }

    static void ws(int s, int, BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["WeatherSpecialed"] = true;
        }
    }
};

struct AMShadowTag : public AM {
    AMShadowTag() {
        functions["IsItTrapped"] = &iit;
    }

    static void iit(int, int t, BS &b) {
        //Shadow Tag
        if (!b.hasWorkingAbility(t, Ability::ShadowTag)) turn(b,t)["Trapped"] = true;
    }
};

struct AMShedSkin : public AM {
    AMShedSkin() {
        functions["EndTurn"] = &et;
    }

    static void et(int s, int, BS &b) {
        if (true_rand() % 100 < 30 && b.poke(s).status() != Pokemon::Fine) {
            b.sendAbMessage(54,0,s,s,Pokemon::Bug);
            b.healStatus(s, b.poke(s).status());
        }
    }
};

struct AMSlowStart : public AM {
    AMSlowStart() {
        functions["UponSetup"] = &us;
        functions["EndTurn"] = &et;
        functions["StatModifier"] = &sm;
    }

    static void us(int s, int t, BS &b) {
        poke(b,s)["SlowStartTurns"] = b.turn() + 4;
        b.sendAbMessage(55,0,s);
    }

    static void et(int s, int, BS &b) {
        if (!b.koed(s) && b.turn() == poke(b,s)["SlowStartTurns"].toInt()) {
            b.sendAbMessage(55,1,s);
        }
    }

    static void sm(int s, int, BS &b) {
        if (b.turn() <= poke(b,s)["SlowStartTurns"].toInt()) {
            turn(b,s)["Stat1AbilityModifier"] = -10;
            turn(b,s)["Stat3AbilityModifier"] = -10;
        }
    }
};

struct AMSolarPower : public AM {
    AMSolarPower() {
        functions["WeatherSpecial"] = &ws;
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Sunny)) {
            turn(b,s)["Stat4AbilityModifier"] = 10;
        }
    }

    static void ws(int s, int, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Sunny)) {
            b.sendAbMessage(56,0,s,s,Pokemon::Fire);
            b.inflictDamage(s,b.poke(s).totalLifePoints()/8,s,false);
        }
    }
};

struct AMSoundProof : public AM {
    AMSoundProof() {
        functions["OpponentBlock"] = &ob;
    }

    struct SoundMoves : public QSet<int> {
        SoundMoves() {
            /* Grasswhistle, Growl, Hyper Voice, Metal Sound, Perish Song, Roar, Sing, Sonicboom, Supersonic, Screech, Snore, Uproar, Roar Of Time, Bug Buzz, Chatter, and Heal Bell */
            (*this) << 48 << 58 << 160 << 162 << 192 << 243 << 320 << 321 << 341 << 357 << 374 << 377 << 402 << 441 ;
        }
    };

    static SoundMoves SM;

    static void ob(int s, int t, BS &b) {
        int mv = move(b,t);

        if (SM.contains(mv)) {
            turn(b,s)[QString("Block%1").arg(t)] = true;
            b.sendAbMessage(57,0,s);
        }
    }
};

AMSoundProof::SoundMoves AMSoundProof::SM;

struct AMSpeedBoost : public AM {
    AMSpeedBoost() {
        functions["EndTurn"] = &et;
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;
        b.sendAbMessage(58,0,s);
        b.gainStatMod(s, Speed, 1);
    }
};

struct AMStall : public AM {
    AMStall() {
        functions["TurnOrder"] = &tu;
    }
    static void tu (int s, int, BS &b) {
        turn(b,s)["TurnOrder"] = -1;
    }
};

struct AMTangledFeet : public AM {
    AMTangledFeet() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int,  BS &b) {
        if (b.isConfused(s)) {
            turn(b,s)["Stat6AbilityModifier"] = 10;
        }
    }
};

struct AMTechnician : public AM {
    AMTechnician() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s, int , BS &b) {
        if (turn(b,s)["Power"].toInt() <= 60) {
            turn(b,s)["BasePowerAbilityModifier"] = 10;
        }
    }
};

struct AMThickFat : public AM {
    AMThickFat() {
        functions["BasePowerFoeModifier"] = &bpfm;
    }

    static void bpfm (int , int t, BS &b) {
        int tp = turn(b,t)["Type"].toInt();

        if (tp == Type::Ice || tp == Type::Fire) {
            turn(b,t)["BasePowerFoeAbilityModifier"] = -10;
        }
    }
};

struct AMTintedLens : public AM {
    AMTintedLens() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s, int , BS &b) {
        if (turn(b,s)["TypeMod"].toInt() < 4) {
            turn(b,s)["BasePowerAbilityModifier"] = 20;
        }
    }
};

struct AMTrace : public AM {
    AMTrace() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int, BS &b) {
        int t = b.rev(s);

        //Multitype
        if (!b.koed(t) && !b.hasWorkingAbility(t,Ability::Multitype)) {
            b.sendAbMessage(66,0,s,t,0,b.ability(t));
            b.acquireAbility(s, b.ability(t));
        }
    }
};

struct AMTruant : public AM {
    AMTruant() {
        functions["UponSetup"] = &us;
        functions["DetermineAttackPossible"] = &dap;
    }

    static void us(int s, int, BS &b) {
        poke(b,s)["TruantActiveTurn"] = (b.turn()+1)%2;
    }

    static void dap(int s, int, BS &b) {
        if (b.turn()%2 != poke(b,s)["TruantActiveTurn"].toInt()) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendAbMessage(67,0,s);
        }
    }
};

struct AMUnburden : public AM {
    AMUnburden() {
        functions["UponSetup"] = &us;
        functions["StatModifier"] = &sm;
    }

    static void us(int s, int, BS &b) {
        poke(b,s)["UnburdenToStartWith"] = b.poke(s).item() != 0;
    }

    static void sm(int s, int, BS &b) {
        if (b.poke(s).item() == 0 && poke(b,s)["UnburdenToStartWith"].toBool()) {
            turn(b,s)["Stat3AbilityModifier"] = 20;
        }
    }
};

struct AMVoltAbsorb : public AM {
    AMVoltAbsorb() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if (type(b,t) == poke(b,s)["AbilityArg"].toInt()) {
            turn(b,s)[QString("Block%1").arg(t)] = true;
            b.sendAbMessage(70,0,s,s,type(b,t), b.ability(s));
            b.healLife(s, b.poke(s).totalLifePoints()/4);
        }
    }
};

struct AMWonderGuard : public AM {
    AMWonderGuard() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        int tp = type(b,t);
        if (turn(b,t)["Power"].toInt() > 0 && tp != Pokemon::Curse) {
            int mod = TypeInfo::Eff(tp, b.getType(s,1)) * TypeInfo::Eff(tp, b.getType(s,2));

            if (mod <= 4) {
                b.sendAbMessage(71,0,s);
                turn(b,s)[QString("Block%1").arg(t)] = true;
            }
        }
    }
};

/* Events:
    UponPhysicalAssault
    DamageFormulaStart
    UponOffensiveDamageReceived
    UponSetup
    UponSwitchOut
    IsItTrapped
    EndTurn
    BasePowerModifier
    BasePowerFoeModifier
    StatModifier
    WeatherSpecial
    WeatherChange
    OpponentBlock
    PreventStatChange
    BeforeTargetList
    TurnOrder
    DetermineAttackPossible
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
    //Gluttony, but done with berries already
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
    REGISTER_AB(41, MotorDrive);
    REGISTER_AB(42, NaturalCure);
    REGISTER_AB(43, Normalize);
    REGISTER_AB(44, OwnTempo);
    REGISTER_AB(46, Pressure);
    REGISTER_AB(48, Reckless);
    REGISTER_AB(49, Rivalry);
    REGISTER_AB(50, RoughSkin);
    REGISTER_AB(51, SandVeil);
    REGISTER_AB(53, ShadowTag);
    REGISTER_AB(54, ShedSkin);
    REGISTER_AB(55, SlowStart);
    REGISTER_AB(56, SolarPower);
    REGISTER_AB(57, SoundProof);
    REGISTER_AB(58, SpeedBoost);
    REGISTER_AB(59, Stall);
    REGISTER_AB(62, TangledFeet);
    REGISTER_AB(63, Technician);
    REGISTER_AB(64, ThickFat);
    REGISTER_AB(65, TintedLens);
    REGISTER_AB(66, Trace);
    REGISTER_AB(67, Truant);
    REGISTER_AB(69, Unburden);
    REGISTER_AB(70, VoltAbsorb);
    REGISTER_AB(71, WonderGuard);
}
