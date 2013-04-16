#include "abilities.h"
#include "miscmoves.h"
#include "moves.h" //For magic mirror.
#include "../PokemonInfo/pokemoninfo.h"
#include "miscabilities.h"
#include "../Shared/battlecommands.h"

QHash<int, AbilityMechanics> AbilityEffect::mechanics;
QHash<int, QString> AbilityEffect::names;
QHash<QString, int> AbilityEffect::nums;

void AbilityEffect::activate(const QString &effect, int num, int source, int target, BattleSituation &b)
{
    AbilityInfo::Effect e = AbilityInfo::Effects(num, b.gen());

    if (!mechanics.contains(e.num) || !mechanics[e.num].functions.contains(effect)) {
        return;
    }
    mechanics[e.num].functions[effect](source, target, b);
}

void AbilityEffect::setup(int num, int source, BattleSituation &b, bool firstAct)
{
    AbilityInfo::Effect effect = AbilityInfo::Effects(num, b.gen());

    /* if the effect is invalid or not yet implemented then no need to go further */
    if (!mechanics.contains(effect.num)) {
        return;
    }

    AM::poke(b, source)["AbilityArg"] = effect.arg;

    QString activationkey = QString("Ability%1SetUp").arg(effect.num);

    /* In gen 3, intimidate/insomnia/... aren't triggered by Trace */
    if (AM::poke(b, source).value(activationkey) != AM::poke(b, source)["SwitchCount"].toInt() && (b.gen() >= 4 || firstAct)) {
        AM::poke(b, source)[activationkey] = AM::poke(b, source)["SwitchCount"].toInt();
        activate("UponSetup", num, source, source, b);
     }
}

struct AMAdaptability : public AM {
    AMAdaptability() {
        functions["DamageFormulaStart"] = &dfs;
    }

    static void dfs(int s, int, BS &b) {
        /* So the regular stab (3) will become 4 and no stab (2) will stay 2 */
        fturn(b,s).stab = fturn(b,s).stab * 4 / 3;
    }
};

struct AMAftermath : public AM {
    AMAftermath() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (b.koed(s) && !b.koed(t)) {
            if (!b.hasWorkingAbility(t,Ability::Damp)){
                b.sendAbMessage(2,0,s,t);
                b.inflictPercentDamage(t,25,s,false);
                if (b.koed(t) && b.gen() >= 5)
                    b.selfKoer() = t;
            }
            else
                b.sendAbMessage(2,1,s,t);
        }
    }
};

struct AMAngerPoint : public AM {
    AMAngerPoint() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (!b.koed(s) && s != t && fturn(b,t).contains(TM::CriticalHit) && (b.gen() <= 4 || !b.hasSubstitute(s))) {
            b.sendAbMessage(3,0,s);
            b.inflictStatMod(s,Attack,12,s);
        }
    }
};

struct AMAnticipation : public AM {
    AMAnticipation() {
        functions["UponSetup"] = &us;
    }

    static void us (int s, int, BS &b) {
        static QList<int> cool_moves = QList<int> () << Move::Counter << Move::MetalBurst << Move::MirrorCoat;

        QList<int> tars = b.revs(s);
        bool frightening_truth = false;
        foreach(int t, tars) {
            for (int i = 0; i < 4; i++) {
                int move = b.move(t, i);

                if (cool_moves.contains(move) || MoveInfo::Power(move, b.gen()) == 0)
                    continue;

                if (move == Move::Explosion || move == Move::Selfdestruct || MoveInfo::isOHKO(move, b.gen()) ||
                    TypeInfo::Eff(MoveInfo::Type(b.move(t, i), b.gen()), b.getType(s,1))
                    * TypeInfo::Eff(MoveInfo::Type(b.move(t, i), b.gen()), b.getType(s,2)) > 4) {
                    frightening_truth = true;
                    break;
                }
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
        functions["EndTurn6.10"] = &et; /* Gen 4 */
        functions["EndTurn26.1"] = &et; /* Gen 5 */
    }

    static void et (int s, int, BS &b) {
        QList<int> tars = b.revs(s);
        foreach(int t, tars) {
            if (b.poke(t).status() == Pokemon::Asleep && !b.hasWorkingAbility(t, Ability::MagicGuard)) {
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
        functions["WeatherSpecial"] = &ws;
    }

    static void sm(int s, int, BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["Stat5AbilityModifier"] = 20;
        }
    }

    static void ws(int s, int, BS &b) {
        if (b.isWeatherWorking(poke(b,s)["AbilityArg"].toInt())) {
            turn(b,s)["WeatherSpecialed"] = true;
        }
    }
};

struct AMColorChange : public AM {
    AMColorChange() {
        functions["UponBeingHit"] = &ubh;
        functions["AfterBeingPlumetted"] = &abp;
    }

    /* gen 3 & 4 event */
    static void ubh(int s, int t, BS &b) {
        if (b.gen() > 4)
            return;
        if (b.koed(s))
            return;
        if ((s!=t) && type(b,t) != Pokemon::Curse) {
            int tp = type(b,t);
            if (fpoke(b,s).type2 == Pokemon::Curse && tp == fpoke(b,s).type1) {
                return;
            }
            b.sendAbMessage(9,0,s,t,tp,tp);
            fpoke(b, s).type1 = tp;
            fpoke(b, s).type2 = Pokemon::Curse;
        }
    }

    /* gen 5 event */
    static void abp(int s, int t, BS &b) {
        if ((s!=t) && type(b,t) != Pokemon::Curse) {
            int tp = type(b,t);
            if (fpoke(b,s).type2 == Pokemon::Curse && tp == fpoke(b,s).type1) {
                return;
            }
            b.sendAbMessage(9,0,s,t,tp,tp);
            fpoke(b, s).type1 = tp;
            fpoke(b, s).type2 = Pokemon::Curse;
        }
    }
};

struct AMCompoundEyes : public AM {
    AMCompoundEyes() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int , BS &b) {
        turn(b,s)["Stat6AbilityModifier"] = 6;
    }
};


struct AMCuteCharm : public AM {
    AMCuteCharm() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (!b.koed(s) && !b.koed(t) && b.isSeductionPossible(s,t) && b.coinflip(30, 100)
            && !b.linked(t, "Attract"))
        {
            b.sendMoveMessage(58,1,s,0,t);
            if (b.hasWorkingItem(t, Item::MentalHerb)) /* mental herb*/ {
                b.sendItemMessage(7,t);
                b.disposeItem(t);
            } else {
                b.link(s, t, "Attract");
                addFunction(poke(b,t), "DetermineAttackPossible", "Attract", &pda);

                if (b.hasWorkingItem(t, Item::DestinyKnot) && b.isSeductionPossible(t, s) && !b.linked(s, "Attract")) {
                    b.link(t, s, "Attract");
                    addFunction(poke(b,s), "DetermineAttackPossible", "Attract", &pda);
                    b.sendItemMessage(7,t,0,s);
                }
            }
        }
    }

    static void pda(int s, int, BS &b) {
        if (fturn(b,s).contains(TM::HasPassedStatus))
            return;
        if (b.linked(s, "Attract")) {
            int seducer = b.linker(s, "Attract");

            b.sendMoveMessage(58,0,s,0,seducer);
            if (b.coinflip(1, 2)) {
                turn(b,s)["ImpossibleToMove"] = true;
                b.sendMoveMessage(58, 2,s);
            }
        }
    }
};

struct AMDownload : public AM {
    AMDownload() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        int t = b.randomOpponent(s);

        b.sendAbMessage(13,0,s);

        if (t==-1|| b.getStat(t, Defense) >= b.getStat(t, SpDefense)) {
            b.inflictStatMod(s, SpAttack,1,s);
        } else {
            b.inflictStatMod(s, Attack,1, s);
        }
    }
};

struct AMDrizzle : public AM {
    AMDrizzle() {
        functions["UponSetup"] = &us;
    }

    static void us (int s, int , BS &b) {
        int w = poke(b,s)["AbilityArg"].toInt();
        if (w != b.weather) {
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
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            b.sendAbMessage(15,0,s,s,Pokemon::Water);
            b.healLife(s, b.poke(s).totalLifePoints()/4);
        }
    }

    static void ws (int s, int , BS &b) {
        if (b.isWeatherWorking(BattleSituation::Rain) && !b.poke(s).isFull()) {
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
        if (b.poke(t).status() == Pokemon::Fine && b.coinflip(30, 100)) {
            switch (b.randint(3)) {
            case 0:
                if (b.canGetStatus(t,Pokemon::Asleep)) {
                    b.sendAbMessage(16,0,s,t,Pokemon::Grass);
                    b.inflictStatus(t, Pokemon::Asleep,s);
                } break;
            case 1:
                if (b.canGetStatus(t,Pokemon::Paralysed)) {
                    b.sendAbMessage(16,0,s,t,Pokemon::Electric);
                    b.inflictStatus(t, Pokemon::Paralysed,s);
                } break;
            default:
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
        if (b.poke(t).status() == Pokemon::Fine && b.coinflip(3, 10)) {
            if (b.canGetStatus(t,poke(b,s)["AbilityArg"].toInt())) {
                b.sendAbMessage(18,0,s,t,Pokemon::Curse,b.ability(s));
                b.inflictStatus(t, poke(b,s)["AbilityArg"].toInt(),s);
            }
        }
    }
};

struct AMFlowerGift : public AM {
    AMFlowerGift() {
        functions["StatModifier"] = &sm;
        functions["PartnerStatModifier"] = &sm2;
        functions["UponSetup"] = &us;
        functions["WeatherChange"] = &us;
    }

    static void us(int s, int, BS &b) {
        if (b.pokenum(s).pokenum != Pokemon::Cherrim)
            return;
        if (b.weather == BS::Sunny) {
            if (b.pokenum(s).subnum != 1) b.changeAForme(s, 1);
        } else {
            if (b.pokenum(s).subnum != 0) b.changeAForme(s, 0);
        }
    }

    static void sm(int s, int, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Sunny)) {
            turn(b,s)["Stat1AbilityModifier"] = 10;
            turn(b,s)["Stat4AbilityModifier"] = 10;
        }
    }

    static void sm2(int , int t, BS &b) {
        /* FlowerGift doesn't stack */
        if (b.isWeatherWorking(BattleSituation::Sunny) && !b.hasWorkingAbility(t, Ability::FlowerGift)) {
            turn(b,t)["Stat1PartnerAbilityModifier"] = 10;
            turn(b,t)["Stat4PartnerAbilityModifier"] = 10;
        }
    }
};

struct AMForeCast : public AM {
    AMForeCast() {
        functions["UponSetup"] = &us;
        functions["WeatherChange"] = &us;
    }

    static void us(int s, int, BS &b) {
        if (PokemonInfo::OriginalForme(b.poke(s).num()) != Pokemon::Castform)
            return;

        int weather = b.weather;
        if (weather != BS::Hail && weather != BS::Rain && weather != BS::Sunny) {
            weather = BS::NormalWeather;
        }

        if (weather == b.poke(s).num().subnum)
            return;

        b.changePokeForme(s, Pokemon::uniqueId(b.poke(s).num().pokenum, weather));
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
        QList<int> tars = b.revs(s);

        if (tars.size() == 0) {
            return;
        }

        int max = 0;
        std::vector<int> poss;

        foreach(int t, tars) {
            for (int i = 0; i < 4; i++) {
                int m = b.move(t,i);
                if (m !=0) {
                    int pow;
                    if (SM.contains(m)) {
                        pow = SM[m];
                    } else if (MoveInfo::Power(m, b.gen()) == 1) {
                        pow = 80;
                    } else {
                        pow = MoveInfo::Power(m, b.gen());
                    }

                    if (pow > max) {
                        poss.clear();
                        poss.push_back(m);
                        max = pow;
                    } else if (pow == max) {
                        poss.push_back(m);
                    }
                }
            }
        }

        int m = poss[b.randint(poss.size())];

        b.sendAbMessage(22,0,s,s,MoveInfo::Type(m, b.gen()),m);
    }
};

AMForeWarn::special_moves AMForeWarn::SM;

struct AMFrisk : public AM {
    AMFrisk() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        int t = b.randomOpponent(s);

        if (t == -1)
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
            make it activated) */
        if (b.poke(s).status() != Pokemon::Fine) {
            if (b.gen() > 3 || b.ability(s) == Ability::MarvelScale || b.poke(s).status() != Pokemon::Asleep || !poke(b,s).value("Rested").toBool()) {
                int arg = poke(b,s)["AbilityArg"].toInt();
                turn(b,s)[QString("Stat%1AbilityModifier").arg(arg)] = 10;
            }
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
        if (tmove(b,s).category == Move::Physical) {
            turn(b,s)["Stat6AbilityModifier"] = -4;
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
                b.sendAbMessage(31,0,s,s,0,b.ability(s));
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
            if (!b.poke(s).isFull()) {
                b.sendAbMessage(32,0,s,s,TypeInfo::TypeForWeather(poke(b,s)["AbilityArg"].toInt()),b.ability(s));
                b.healLife(s, b.poke(s).totalLifePoints()/16);
            }
        }
    }
};

struct AMInsomnia : public AM {
    AMInsomnia() {
        functions["UponSetup"] = &us;
        functions["PreventStatChange"] = &psc;
    }

    static void us(int s, int, BS &b) {
        if (b.poke(s).status() == poke(b,s)["AbilityArg"].toInt()) {
            b.sendAbMessage(33,0,s,s,Pokemon::Dark,b.ability(s));
            b.healStatus(s, b.poke(s).status());
        }
    }

    static void psc(int s, int t, BS &b) {
        if (turn(b,s)["StatModType"].toString() == "Status" && turn(b,s)["StatusInflicted"].toInt() == poke(b,s)["AbilityArg"].toInt()) {
            if (b.canSendPreventSMessage(s,t))
                b.sendAbMessage(33,turn(b,s)["StatusInflicted"].toInt(),s,s,0,b.ability(s));
            b.preventStatMod(s,t);
        }
    }
};

struct AMOwnTempo : public AM {
    AMOwnTempo() {
        functions["UponSetup"] = &us;
        functions["PreventStatChange"] = &psc;
    }

    static void us(int s, int, BS &b) {
        if (b.isConfused(s)) {
            b.sendAbMessage(44,0,s);
            b.healConfused(s);
        }
    }

    static void psc(int s, int t, BS &b) {
        if (turn(b,s)["StatModType"].toString() == "Status" && turn(b,s)["StatusInflicted"].toInt() == Pokemon::Confused) {
            if (b.canSendPreventSMessage(s,t))
                b.sendAbMessage(44,1,s,s,0,b.ability(s));
            b.preventStatMod(s,t);
        }
    }
};

struct AMIntimidate : public AM {
    AMIntimidate() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        QList<int> tars = b.revs(s);

        foreach(int t, tars) {
            if (!b.areAdjacent(s, t)) {
                continue;
            }
            if (b.hasSubstitute(t)) {
                b.sendAbMessage(34,1,s,t);
            } else {
                b.sendAbMessage(34,0,s,t);
                b.inflictStatMod(t,Attack,-1,s);
            }
        }
    }
};

struct AMIronFist : public AM {
    AMIronFist() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm (int s, int t, BS &b) {
        if (s != t && tmove(b,s).flags & Move::PunchFlag) {
            turn(b,s)["BasePowerAbilityModifier"] = 4;
        }
    }
};

struct AMLeafGuard  : public AM {
    AMLeafGuard() {
        functions["PreventStatChange"]= &psc;
    }

    static void psc(int s, int t, BS &b) {
        if (b.isWeatherWorking(BattleSituation::Sunny) && turn(b,s)["StatModType"].toString() == "Status") {
            if (b.canSendPreventSMessage(s,t))
                b.sendAbMessage(37,0,s,s,0,b.ability(s));
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
        b.sendAbMessage(40,0,s,s,0,b.ability(s));
    }
};

struct AMMotorDrive : public AM {
    AMMotorDrive() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if (type(b,t) == Type::Electric) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            b.sendAbMessage(41,0,s,s,Pokemon::Electric);
            b.inflictStatMod(s,Speed,1,s);
        }
    }
};

struct AMNormalize : public AM {
    AMNormalize() {
        functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
        if (tmove(b,s).type != Type::Curse)
            tmove(b,s).type = Type::Normal;
    }
};

struct AMPoisonTouch : public AM {
    AMPoisonTouch() {
        functions["OnPhysicalAssault"] = &opa;
    }

    static void opa(int s, int t, BS &b) {
        if (tmove(b,s).classification == Move::OffensiveStatChangingMove || tmove(b,s).flinchRate > 0 || b.hasSubstitute(t))
            return;
        if (b.poke(t).status() == Pokemon::Fine && b.coinflip(2, 10)) {
            if (b.canGetStatus(t,poke(b,s)["AbilityArg"].toInt())) {
                b.sendAbMessage(18,0,s,t,Pokemon::Curse,b.ability(s));
                b.inflictStatus(t, poke(b,s)["AbilityArg"].toInt(),s);
            }
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
        if (tmove(b,s).recoil < 0 || mv == Move::HiJumpKick || mv == Move::JumpKick) {
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
        if (!b.koed(t) && !b.hasWorkingAbility(t, Ability::MagicGuard)) {
            b.sendAbMessage(50,0,s,t,0,b.ability(s));
            b.inflictDamage(t,b.poke(t).totalLifePoints()/8,s,false);

            /* In VGC 2011, the one with the rugged helmet wins */
            if (b.koed(t)) {
                b.selfKoer() = t;
            }
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
            turn(b,s)["Stat7AbilityModifier"] = 4;
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
        if (!b.hasWorkingAbility(t, Ability::ShadowTag) || b.gen().num == 3) turn(b,t)["Trapped"] = true;
    }
};

struct AMShedSkin : public AM {
    AMShedSkin() {
        functions["EndTurn6.2"] = &et; /* Gen 4 */
        functions["EndTurn5.1"] = &et; /* Gen 5 */
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;
        if (b.coinflip(30, 100) && b.poke(s).status() != Pokemon::Fine) {
            b.sendAbMessage(54,0,s,s,Pokemon::Bug);
            b.healStatus(s, b.poke(s).status());
        }
    }
};

struct AMSlowStart : public AM {
    AMSlowStart() {
        functions["UponSetup"] = &us;
        functions["EndTurn12.0"] = &et; /* gen 4 */
        functions["EndTurn29.0"] = &et; /* gen 5 */
        functions["StatModifier"] = &sm;
    }

    static void us(int s, int, BS &b) {
        poke(b,s)["SlowStartTurns"] = b.turn() + 5;
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
            turn(b,s)["Stat5AbilityModifier"] = -10;
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
            turn(b,s)["Stat3AbilityModifier"] = 10;
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

    static void ob(int s, int t, BS &b) {
        if (tmove(b,t).flags & Move::SoundFlag) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            b.sendAbMessage(57,0,s);
        }
    }
};

struct AMSpeedBoost : public AM {
    AMSpeedBoost() {
        functions["UponSetup"] = &os;
        functions["EndTurn6.2"] = &et; /* Gen 4 */
        functions["EndTurn26.1"] = &et; /* Gen 5 */
    }

    static void os(int s, int, BS &b) {
        poke(b,s)["SpeedBoostSetupTurn"] = b.turn();
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s) || b.turn() == poke(b,s).value("SpeedBoostSetupTurn").toInt() ||
                b.hasMaximalStatMod(s, poke(b,s).value("AbilityArg").toInt()))
            return;
        b.sendAbMessage(58,b.ability(s) == Ability::SpeedBoost ? 0 : 1,s);
        b.inflictStatMod(s, poke(b,s)["AbilityArg"].toInt(), 1, s);
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
            turn(b,s)["Stat7AbilityModifier"] = 10;
        }
    }
};

struct AMTechnician : public AM {
    AMTechnician() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s, int , BS &b) {
        /* Move::NoMove is for confusion damage, Struggle is affected by technician in gen 5 but not gen 4 */
        if (tmove(b,s).power <= 60 && ( (b.gen() >= 5 && move(b,s) != Move::NoMove) || (b.gen() <= 4 && type(b,s) != Type::Curse) ) ) {
            turn(b,s)["BasePowerAbilityModifier"] = 10;
        }
    }
};

struct AMThickFat : public AM {
    AMThickFat() {
        functions["BasePowerFoeModifier"] = &bpfm;
    }

    static void bpfm (int , int t, BS &b) {
        int tp = tmove(b,t).type;

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
        if (fturn(b,s).typeMod < 4) {
            turn(b,s)["BasePowerAbilityModifier"] = 20;
        }
    }
};

struct AMTrace : public AM {
    AMTrace() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int, BS &b) {
        int t = b.randomOpponent(s);

        if (t == - 1)
            return;

        int ab = b.ability(t);
        //Multitype
        if (b.hasWorkingAbility(t, ab) && ab != Ability::Multitype && ab !=  Ability::Trace
            && !(ab == Ability::Illusion && poke(b,t).contains("IllusionTarget"))) {
            b.sendAbMessage(66,0,s,t,0,ab);
            b.loseAbility(s);
            b.acquireAbility(s, ab);
        }
    }
};

struct AMTruant : public AM {
    AMTruant() {
        functions["DetermineAttackPossible"] = &dap;
    }

    static void dap(int s, int, BS &b) {
        if (!poke(b, s).contains("TruantActiveTurn")) {
            poke(b,s)["TruantActiveTurn"] = b.turn()%2;
        }
        if (b.turn()%2 != poke(b,s)["TruantActiveTurn"].toInt()) {
            turn(b,s)["ImpossibleToMove"] = true;
            b.sendAbMessage(67,0,s);
        }
    }
};

struct AMUnburden : public AM {
    AMUnburden() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (b.poke(s).item() == 0 && poke(b,s).value("Unburdened").toBool()) {
            turn(b,s)["Stat5AbilityModifier"] = 20;
        }
    }
};

struct AMVoltAbsorb : public AM {
    AMVoltAbsorb() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if (type(b,t) == poke(b,s)["AbilityArg"].toInt() && (b.gen() >= 4 || tmove(b,t).power > 0) ) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;

            if (b.poke(s).lifePoints() == b.poke(s).totalLifePoints()) {
                b.sendAbMessage(70,1,s,s,type(b,t), b.ability(s));
            } else {
                b.sendAbMessage(70,0,s,s,type(b,t), b.ability(s));
                b.healLife(s, b.poke(s).totalLifePoints()/4);
            }
        }
    }
};

struct AMWonderGuard : public AM {
    AMWonderGuard() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        int tp = type(b,t);
        /* Fire fang always hits through Wonder Guard, at least in 4th gen... */
        if (tmove(b,t).power > 0 && tp != Pokemon::Curse && (b.gen() >= 5 || move(b, t) != Move::FireFang)) {
            int mod = TypeInfo::Eff(tp, b.getType(s,1)) * TypeInfo::Eff(tp, b.getType(s,2));

            if (mod <= 4) {
                b.sendAbMessage(71,0,s);
                turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            }
        }
    }
};

struct AMLightningRod : public AM {
    AMLightningRod() {
        functions["GeneralTargetChange"] = &gtc;
        functions["OpponentBlock"] = &ob;
    }

    static void gtc(int s, int t, BS &b) {
        if (type(b,t) != poke(b,s)["AbilityArg"].toInt()) {
            return;
        }

        int tarChoice = tmove(b,t).targets;
        bool muliTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

        if (muliTar) {
            return;
        }

        /* So, we make the move hit with 100 % accuracy */
        tmove(b,t).accuracy = 0;

        turn(b,t)["TargetChanged"] = true;

        if (turn(b,t)["Target"].toInt() == s) {
            return;
        } else {
            b.sendAbMessage(38,0,s,t,0,b.ability(s));
            turn(b,t)["Target"] = s;
        }
    }

    static void ob(int s, int t, BS &b) {
        if (b.gen() <= 4)
            return;

        int tp = type(b,t);

        if (tp == poke(b,s)["AbilityArg"].toInt()) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            if (b.hasMaximalStatMod(s, SpAttack)) {
                b.sendAbMessage(38, 2, s, 0, tp, b.ability(s));
            } else {
                b.inflictStatMod(s, SpAttack, 1, s, false);
                b.sendAbMessage(38, 1, s, 0, tp, b.ability(s));
            }
        }
    }
};

struct AMPlus : public AM {
    AMPlus() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (!b.multiples()) {
            return;
        }
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (!b.arePartners(i, s) || i==s || b.koed(i)) {
                continue;
            }
            if (b.hasWorkingAbility(i, Ability::Minus)) {
                turn(b,s)["Stat3AbilityModifier"] = 10;
                return;
            }
        }
    }
};

struct AMMinus : public AM {
    AMMinus() {
        functions["StatModifier"] = &sm;
    }


    static void sm(int s, int, BS &b) {
        if (!b.multiples()) {
            return;
        }
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (!b.arePartners(i, s) || i==s || b.koed(i)) {
                continue;
            }
            if (b.hasWorkingAbility(i, Ability::Minus)) {
                turn(b,s)["Stat3AbilityModifier"] = 10;
                return;
            }
        }
    }
};

/* 5th gen abilities */
struct AMDustProof : public AM {
    AMDustProof() {
        functions["WeatherSpecial"] = &ws;
    }

    static void ws(int s, int, BS &b) {
        turn(b,s)["WeatherSpecialed"] = true;
    }
};

struct AMMummy : public AM {
    AMMummy() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if ( (b.countBackUp(b.player(s)) > 0 || !b.koed(s)) && b.ability(t) != Ability::Mummy) {
            b.sendAbMessage(47, 0, t);
            b.loseAbility(t);
            b.acquireAbility(t, Ability::Mummy);
        }
    }
};

struct AMEarthquakeSpiral : public AM {
    AMEarthquakeSpiral() {
        functions["AfterKoing"] = &ak;
    }

    static void ak(int s, int, BS &b) {
        b.inflictStatMod(s, Attack, 1, s);
    }
};

struct AMHerbivore : public AM {
    AMHerbivore() {
        functions["OpponentBlock"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        int tp = type(b,t);

        if (tp == poke(b,s)["AbilityArg"].toInt()) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            if (!b.hasMaximalStatMod(s, Attack)) {
                b.sendAbMessage(68, 0, s, 0, tp, b.ability(s));
                b.inflictStatMod(s, Attack, 1, s, false);
            } else {
                b.sendAbMessage(68, 1, s, 0, tp, b.ability(s));
            }
        }
    }
};

struct AMSandPower : public AM {
    AMSandPower() {
        functions["BasePowerModifier"] = &bpam;
        functions["WeatherSpecial"] = &ws;
    }

    static void bpam(int s, int, BS &b) {
        if (b.isWeatherWorking(BS::SandStorm)) {
            int t = type(b,s);

            if (t == Type::Rock || t == Type::Steel || t == Type::Ground) {
                turn(b,s)["BasePowerAbilityModifier"] = 6;
            }
        }
    }

    static void ws(int s, int, BS &b) {
        if (b.isWeatherWorking(BS::SandStorm)) {
            turn(b,s)["WeatherSpecialed"] = true;
        }
    }
};

//struct AMJackOfAllTrades : public AM {
//    AMJackOfAllTrades() {
//        functions["DamageFormulaStart"] = &dfs;
//    }

//    static void dfs(int s, int, BS &b) {
//        turn(b,s)["Stab"] = 3;
//    }
//};

struct AMBrokenArmour : public AM {
    AMBrokenArmour() {
        functions["UponBeingHit"] = &upa;
    }

    static void upa(int s, int t, BS &b) {
        if (b.koed(s) || tmove(b,t).category != Move::Physical)
            return;

        b.sendAbMessage(74, 0, s, 0);
        if (!b.hasMinimalStatMod(s, Defense)) {
            b.inflictStatMod(s, Defense, -1, s);
        }
        if (!b.hasMaximalStatMod(s, Speed)) {
            b.inflictStatMod(s, Speed, 1, s);
        }
    }
};

struct AMVictoryStar : public AM {
    AMVictoryStar() {
        functions["StatModifier"] = &sm;
        functions["PartnerStatModifier"] = &sm2;
    }

    static void sm(int s, int, BS &b) {
        turn(b,s)["Stat6AbilityModifier"] = 2;
    }

    static void sm2(int , int t, BS &b) {
        /* FlowerGift doesn't stack */
        if (!b.hasWorkingAbility(t, Ability::VictoryStar)) {
            turn(b,t)["Stat6PartnerAbilityModifier"] = 2;
        }
    }
};

struct AMWeakKneed : public AM {
    AMWeakKneed() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (b.poke(s).lifePoints() * 2 > b.poke(s).totalLifePoints())
            return;

        turn(b,s)["Stat1AbilityModifier"] = -10;
        turn(b,s)["Stat3AbilityModifier"] = -10;
    }
};

struct AMDarumaMode : public AM {
    AMDarumaMode() {
        functions["AfterHPChange"] = &ahpc;
    }

    static void ahpc(int s, int, BS &b) {
        Pokemon::uniqueId num = fpoke(b,s).id;

        if (PokemonInfo::OriginalForme(num) != Pokemon::Hihidaruma) {
            return;
        }

        bool daruma = b.poke(s).lifePoints() * 2 <= b.poke(s).totalLifePoints();

        if (daruma == num.subnum)
            return;

        b.changePokeForme(s, Pokemon::uniqueId(num.pokenum, daruma));
    }
};

struct AMWickedThief : public AM
{
    AMWickedThief() {
        functions["UponPhysicalAssault"] = &upa;
    }

    /* Ripped off from Covet */
    static void upa(int s, int t, BS &b) {
        if (!b.koed(t) && b.poke(t).item() != 0 && !b.koed(s) && !b.hasWorkingAbility(t, Ability::StickyHold)
                    && b.ability(t) != Ability::Multitype && !b.hasWorkingAbility(s, Ability::Multitype)
                    && b.pokenum(s).pokenum != Pokemon::Giratina && b.poke(s).item() == 0
                            && b.pokenum(t).pokenum != Pokemon::Giratina && !ItemInfo::isMail(b.poke(t).item()))
            {
            b.sendAbMessage(78, 0,s,t,0,b.poke(t).item());
            b.acqItem(s, b.poke(t).item());
            b.loseItem(t);
        }
    }
};

struct AMEncourage : public AM
{
    AMEncourage() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s, int, BS &b) {
        int cl = tmove(b,s).classification;

        /* Self stat changing moves like nitro charge/ancient power are boosted, but not moves like close combat/super power */
        if (cl != Move::OffensiveStatChangingMove && cl != Move::OffensiveStatusInducingMove && tmove(b,s).flinchRate == 0
            && (cl != Move::OffensiveSelfStatChangingMove || ((signed char)(tmove(b,s).boostOfStat >> 16)) < 0))
            return;

        tmove(b,s).classification = Move::StandardMove;
        tmove(b,s).flinchRate = 0;
        turn(b,s)["BasePowerAbilityModifier"] = 6;

        /* Ugly, to tell life orb not to activate =/ */
        turn(b,s)["EncourageBug"] = true;
    }
};

struct AMCompetitiveSpirit : public AM
{
    AMCompetitiveSpirit() {
        functions["AfterNegativeStatChange"] = &ansc;
    }

    static void ansc(int s, int ts, BS &b) {
        if (b.hasMaximalStatMod(s, Attack))
            return;
        if (ts != -1 && b.player(ts) == b.player(s))
            return;
        /* Fix me : ability message */
        b.sendAbMessage(80, 0, s);
        b.inflictStatMod(s, Attack, 2, s, false);
    }
};

struct AMImposter : public AM
{
    AMImposter() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int , BS &b) {
        int t = b.randomOpponent(s);

        if (t == -1)
            return;

        if (fpoke(b,t).flags & BS::BasicPokeInfo::Transformed || b.hasSubstitute(t))
            return;

        if (b.hasWorkingAbility(t,  Ability::Illusion) && poke(b,t).contains("IllusionTarget"))
            return;

        fpoke(b,s).flags |= BS::BasicPokeInfo::Transformed;
        /* Ripped off from Transform */
        /* Give new values to what needed */
        Pokemon::uniqueId num = b.pokenum(t);

        if (b.gen() <= 4) {
            if (num.toPokeRef() == Pokemon::Giratina_O && b.poke(s).item() != Item::GriseousOrb)
                num = Pokemon::Giratina;
            if (PokemonInfo::OriginalForme(num) == Pokemon::Arceus) {
                num.subnum = ItemInfo::PlateType(b.poke(s).item());
            }
            if (PokemonInfo::OriginalForme(num) == Pokemon::Genesect)
                num.subnum = ItemInfo::DriveForme(b.poke(s).item());
        }

        b.sendAbMessage(81,0,s,s,0,num.pokenum);

        BS::BasicPokeInfo &po = fpoke(b,s);
        BS::BasicPokeInfo &pt = fpoke(b,t);

        po.id = num;
        po.weight = PokemonInfo::Weight(num);
        po.type1 = PokemonInfo::Type1(num, b.gen());
        po.type2 = PokemonInfo::Type2(num, b.gen());

        b.changeSprite(s, num);

        for (int i = 0; i < 4; i++) {
            b.changeTempMove(s,i,b.move(t,i));
        }

        for (int i = 1; i < 6; i++)
            po.stats[i] = pt.stats[i];

//        for (int i = 0; i < 6; i++) {
//            po.dvs[i] = pt.dvs[i];
//        }

        for (int i = 0; i < 8; i++) {
            po.boosts[i] = pt.boosts[i];
        }

        b.loseAbility(s);
        b.acquireAbility(s, b.ability(t));
    }
};

struct AMMischievousHeart : public AM
{
    AMMischievousHeart() {
        functions["PriorityChoice"] = &pc;
    }

    static void pc(int s, int, BS &b) {
        if (tmove(b,s).power == 0 && tmove(b,s).priority == 0)
            tmove(b,s).priority += 1;
    }
};

struct AMMultiScale : public AM
{
    AMMultiScale() {
        functions["BasePowerFoeModifier"] = &bpfm;
    }

    static void bpfm(int s, int t, BS &b) {
        if (b.poke(s).isFull()) {
            turn(b,t)["BasePowerFoeAbilityModifier"] = -10;
        }
    }
};

struct AMHeatRampage : public AM
{
    AMHeatRampage() {
        functions["StatModifier"] = &sm;
    }

    static void sm (int s, int, BS &b) {
        int st = poke(b,s)["AbilityArg"].toInt();

        if (b.poke(s).status() == st) {
            if (st == Pokemon::Burnt) {
                turn(b,s)[QString("Stat%1AbilityModifier").arg(SpAttack)] = 10;
            } else {
                turn(b,s)[QString("Stat%1AbilityModifier").arg(Attack)] = 10;
            }
        }
    }
};

struct AMTelepathy : public AM {
    AMTelepathy() {
        functions["OpponentBlock"] = &op;
    }

    static void op(int s, int t, BS &b) {
        if (tmove(b,t).power > 0 && b.player(t) == b.player(s)) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;

            b.sendAbMessage(85,0,s,t,Type::Psychic);
        }
    }
};

struct AMMagicBounce : public AM
{
    AMMagicBounce() {
        functions["UponSetup"] = &uas;
    }

    static void uas (int, int, BS &b) {
        addFunction(b.battleMemory(), "DetermineGeneralAttackFailure2", "MagicBounce", &dgaf);
    }

    static void dgaf(int s, int t, BS &b) {
        bool bounced = tmove(b, s).flags & Move::MagicCoatableFlag;
        if (!bounced)
            return;
        /* Don't double bounce something */
        if (b.battleMemory().contains("CoatingAttackNow")) {
            return;
        }

        int target = -1;

        if (t != s && b.hasWorkingAbility(t, Ability::MagicBounce) ) {
            target = t;
        } else {
            /* Entry hazards */
            if (tmove(b,s).targets == Move::OpposingTeam) {
                foreach(int t, b.revs(s)) {
                    if (b.koed(t)) {
                        continue;
                    }
                    if (b.hasWorkingAbility(t, Ability::MagicBounce) && !b.hasWorkingAbility(s, Ability::MoldBreaker)) {
                        target = t;
                        break;
                    }
                }
            }
        }

        if (target == -1)
            return;

        int move = AM::move(b,s);

        b.fail(s,76, 2, Pokemon::Psychic);
        /* Now Bouncing back ... */
        BS::context ctx = turn(b,target);
        BS::BasicMoveInfo info = tmove(b,target);
        BS::TurnMemory turnMem = fturn(b, target);
        int lastMove = fpoke(b,target).lastMoveUsed;

        turn(b,target).clear();
        MoveEffect::setup(move,target,s,b);

        turn(b,target)["Target"] = s;
        b.battleMemory()["CoatingAttackNow"] = true;
        b.useAttack(target,move,true,false);
        b.battleMemory().remove("CoatingAttackNow");

        /* Restoring previous state. Only works because moves reflected don't store useful data in the turn memory,
            and don't cause any such data to be stored in that memory */
        turn(b,target) = ctx;
        tmove(b,target) = info;
        fturn(b,target) = turnMem;
        fpoke(b,target).lastMoveUsed = lastMove;
    }
};

struct AMHarvest : public AM
{
    AMHarvest() {
        functions["EndTurn26.1"] = &et;
    }

    static void et(int s, int, BS &b) {
        if (b.poke(s).item() == 0 && b.poke(s).itemUsed() != 0 && ItemInfo::isBerry(b.poke(s).itemUsed())) {
            if (!b.isWeatherWorking(BattleSituation::Sunny)) {
                if (b.coinflip(1, 2))
                     return; // 50 % change when not sunny
            }
            int item = b.poke(s).itemUsed();
            b.poke(s).itemUsed() = 0;
            b.sendAbMessage(88, 0, s, 0, 0, item);
            b.acqItem(s, item);
        }
    }
};

struct AMCloudNine : public AM {
    AMCloudNine() {
        functions["UponSetup"] = &us;
    }

    static void us (int s, int, BS &b) {
        if (b.gen() >= 5)
            b.sendAbMessage(40,0,s,s,0,b.ability(s));
    }
};

/**
  Miracle skin evades all "status" moves (including taunt, encore, ...) half the time
  */
struct AMMiracleSkin : public AM {
    AMMiracleSkin() {
        functions["TestEvasion"] = &psc;
    }

    static void psc(int s, int t, BS &b) {
        if (tmove(b,t).power == 0 && tmove(b,t).accuracy != 0) {
            if (b.coinflip(1,2)) {
                turn(b,s)["EvadeAttack"] = true;
            } else {
                tmove(b, s).accuracy = 0;
            }
        }
    }
};

/* 5th gen sturdy only */
struct AMSturdy : public AM {
    AMSturdy() {
        functions["BeforeTakingDamage"] = &btd;
        functions["UponSelfSurvival"] = &uss;
    }

    static void btd(int s, int, BS &b) {
        if (b.poke(s).isFull()) {
            turn(b,s)["CannotBeKoedAt"] = b.attackCount();
            turn(b,s)["SturdyActivated"] = true;
        }
    }

    static void uss(int s, int , BS &b) {
        /*  It may be possible for both Sturdy and Focus Band to activate,
          so we make sure sturdy activated before sending the sturdy message,
          otherwise we let focus band sending its message */
        if (turn(b,s)["SturdyActivated"].toBool()) {
            b.sendAbMessage(91, 0, s);
            turn(b,s)["SurviveReason"] = true;
        }
    }
};

struct AMIllusion : public AM {
    AMIllusion() {
        functions["UponBeingHit"] = &ubh;
        functions["BeforeBeingKoed"] = &ubh;
    }

    static void ubh(int s, int, BS &b) {
        if (!poke(b,s).contains("IllusionTarget"))
            return;
        poke(b,s).remove("IllusionTarget");
        /* Bad!! But this is such a peculiar ability, I'll allow this. */
        b.notify(BS::All, BattleCommands::SendOut, s, true, quint8(b.slotNum(s)), b.opoke(s, b.player(s), b.slotNum(s)));
    }
};

struct AMJusticeHeart : public AM {
    AMJusticeHeart() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (b.koed(s))
            return;

        int tp = type(b,t);

        if (tp == Pokemon::Dark) {
            if (!b.hasMaximalStatMod(s, Attack)) {
                b.sendAbMessage(68, 0, s, 0, tp, b.ability(s));
                b.inflictStatMod(s, Attack, 1, s, false);
            }
        }
    }
};

struct AMInconsistent : public AM {
    AMInconsistent() {
        functions["EndTurn26.1"] = &et;
    }

    static void et(int s, int, BS &b) {
        if (b.koed(s))
            return;
        QList<int> raisableStats = QList<int>();
        for (int i = Attack; i <= Evasion; i++) {
            if (!b.hasMaximalStatMod(s, i))
                raisableStats.push_back(i);
        }
        if (raisableStats.empty())
            return;
        int randomStat = raisableStats[b.randint(raisableStats.size())];

        b.sendAbMessage(95,0,s,0, 0, randomStat);
        b.inflictStatMod(s, randomStat, 2, s, false);

        raisableStats.clear();
        for (int i = Attack; i <= Evasion; i++) {
            if (!b.hasMinimalStatMod(s, i) && i != randomStat)
                raisableStats.push_back(i);
        }
        if (raisableStats.empty())
            return;
        randomStat = raisableStats[b.randint(raisableStats.size())];

        b.sendAbMessage(95,1,s,0, 0, randomStat);
        b.inflictStatMod(s, randomStat, -1, s, false);
    }
};

struct AMCursedBody : public AM {
    AMCursedBody() {
        functions["UponBeingHit"] = ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (b.koed(t) || MMDisable::failOn(t, b) || !b.coinflip(30, 100))
            return;

        b.sendAbMessage(96, 0, s);
        MMDisable::uas(s, t, b);
    }
};

struct AMSelfConscious : public AM {
    AMSelfConscious() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (b.koed(s)) {
            return;
        }
        int tp = type(b,t);

        if ((tp == Type::Bug || tp == Type::Ghost || tp == Type::Dark) && !b.hasMaximalStatMod(s, Speed)) {
            b.sendAbMessage(97,0,s);
            b.inflictStatMod(s, Speed, 1, s, false);
        }
    }
};

struct AMAnalyze : public AM {
    AMAnalyze() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s,int,BS&b) {
        if (b.speedsVector.back() == s) {
            turn(b,s)["BasePowerAbilityModifier"] = 6;
        }
    }
};

struct AMHealingHeart : public AM {
    AMHealingHeart() {
        functions["EndTurn5.1"] = &et;
    }

    static void et(int s, int, BS &b) {
        if (!b.multiples()) {
            return;
        }

        if (!b.coinflip(30, 100)) {
            return;
        }

        std::vector<int> partners;
        for (int i = 0; i < b.numberOfSlots();i++) {
            if (b.areAdjacent(i, s) && i!=s && b.arePartners(i, s) && !b.koed(i) && b.poke(i).status() != Pokemon::Fine) {
                partners.push_back(i);
            }
        }

        if (partners.size() == 0)
            return;

        int p = partners[b.randint(partners.size())];

        b.sendAbMessage(99, 0, s, p);
        b.healStatus(p, b.poke(p).status());
    }
};

struct AMFriendGuard : public AM
{
    AMFriendGuard() {
        functions["BasePowerAbilityModifier"] = &bpm;
    }

    static void bpm(int s, int t, BS &b) {
        if (b.arePartners(s, t)) {
            turn(b,s)["BasePowerModifier"] = -5;
        }
    }
};

struct AMNaturalCure : public AM {
    AMNaturalCure () {
        functions["UponSwitchOut"] = &uso;
    }

    static void uso (int s, int, BS &b) {
        if (b.gen() <= 4)
            return;
        b.healStatus(s, b.poke(s).status());
    }
};

struct AMRegeneration : public AM {
    AMRegeneration() {
        functions["UponSwitchOut"] = &us;
    }

    static void us(int s, int, BS &b) {
        if (!b.poke(s).isFull()) {
            b.healLife(s, b.poke(s).totalLifePoints() / 3);
        }
    }
};

struct AMPickUp : public AM {
    AMPickUp() {
        functions["EndTurn26.1"] = &et;
    }

    static void et(int s, int, BS &b) {
        if (b.poke(s).item() != 0) {
            return;
        }
        std::vector<int> possibilities;
        for (int i = 0; i < b.numberOfSlots(); i++) {
            if (!b.koed(i) && b.areAdjacent(s, i) && s != i && b.poke(i).itemUsed() != 0
                    && b.poke(i).itemUsedTurn() == b.turn()) {
                possibilities.push_back(i);
            }
        }

        if (possibilities.size() == 0) {
            return;
        }

        int i = possibilities[b.randint(possibilities.size())];
        int item = b.poke(i).itemUsed();
        b.sendAbMessage(93, 0, s, 0, 0, item);
        b.poke(i).itemUsed() = 0;
        b.acqItem(s, item);
    }
};

struct AMUnnerve : public AM {
    AMUnnerve() {
        functions["UponSetup"] = &us;
    }

    static void us(int s, int, BS &b) {
        b.sendAbMessage(102,0,s);
    }
};



/* Events:
    PriorityChoice
    AfterNegativeStatChange
    UponPhysicalAssault
    OnPhysicalAssault
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
    BeforeTargetList
    TurnOrder
    DetermineAttackPossible
    GeneralTargetChange
    PartnerStatModifier
    AfterKoing
    UponSwitchOut
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
    //Inner Focus Message
    REGISTER_AB(13, Download);
    REGISTER_AB(14, Drizzle);
    REGISTER_AB(15, DrySkin);
    REGISTER_AB(16, EffectSpore);
    REGISTER_AB(17, DustProof);
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
    REGISTER_AB(36, Minus);
    REGISTER_AB(37, LeafGuard);
    REGISTER_AB(38, LightningRod)
    REGISTER_AB(39, MagnetPull);
    REGISTER_AB(40, MoldBreaker);
    REGISTER_AB(41, MotorDrive);
    REGISTER_AB(42, NaturalCure);
    REGISTER_AB(43, Normalize);
    REGISTER_AB(44, OwnTempo);
    REGISTER_AB(45, Plus);
    REGISTER_AB(46, Pressure);
    REGISTER_AB(47, Mummy);
    REGISTER_AB(48, Reckless);
    REGISTER_AB(49, Rivalry);
    REGISTER_AB(50, RoughSkin);
    REGISTER_AB(51, SandVeil);
    REGISTER_AB(52, EarthquakeSpiral);
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
    REGISTER_AB(68, Herbivore);
    REGISTER_AB(69, Unburden);
    REGISTER_AB(70, VoltAbsorb);
    REGISTER_AB(71, WonderGuard);
    REGISTER_AB(72, SandPower);
//    REGISTER_AB(73, JackOfAllTrades);
    REGISTER_AB(74, BrokenArmour);
    REGISTER_AB(75, VictoryStar);
    REGISTER_AB(76, WeakKneed);
    REGISTER_AB(77, DarumaMode);
    REGISTER_AB(78, WickedThief);
    REGISTER_AB(79, Encourage);
    REGISTER_AB(80, CompetitiveSpirit);
    REGISTER_AB(81, Imposter);
    REGISTER_AB(82, MischievousHeart);
    REGISTER_AB(83, MultiScale);
    REGISTER_AB(84, HeatRampage);
    REGISTER_AB(85, Telepathy);
    REGISTER_AB(86, Regeneration);
    REGISTER_AB(87, MagicBounce);
    REGISTER_AB(88, Harvest);
    REGISTER_AB(89, CloudNine);
    REGISTER_AB(90, MiracleSkin);
    REGISTER_AB(91, Sturdy);
    REGISTER_AB(92, Illusion);
    REGISTER_AB(93, PickUp);
    REGISTER_AB(94, JusticeHeart);
    REGISTER_AB(95, Inconsistent);
    REGISTER_AB(96, CursedBody);
    REGISTER_AB(97, SelfConscious);
    REGISTER_AB(98, Analyze);
    REGISTER_AB(99, HealingHeart);
    REGISTER_AB(100, FriendGuard);
    REGISTER_AB(101, PoisonTouch);
    REGISTER_AB(102, Unnerve);
}
