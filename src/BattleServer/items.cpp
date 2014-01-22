#include "items.h"
#include "berries.h"
#include <PokemonInfo/pokemoninfo.h>
#include "battlecounterindex.h"

typedef ItemMechanics IM;
typedef BattleSituation BS;
typedef BattleCounterIndex BC;
typedef BattleSituation::TurnMemory TM;

QHash<int, ItemMechanics> ItemEffect::mechanics;
QHash<int, QString> ItemEffect::names;
QHash<QString, int> ItemEffect::nums;

void ItemEffect::activate(const QString &effect, int num, int source, int target, BattleSituation &b)
{
    QList<ItemInfo::Effect> l = ItemInfo::Effects(num, b.gen());

    foreach(ItemInfo::Effect e, l) {
        if (!mechanics.contains(e.num) || !mechanics[e.num].functions.contains(effect)) {
            continue;
        }
        mechanics[e.num].functions[effect](source, target, b);
    }
}

void ItemEffect::setup(int num, int source, BattleSituation &b)
{
    QList<ItemInfo::Effect> effects = ItemInfo::Effects(num, b.gen());

    foreach(ItemInfo::Effect effect, effects) {
        /* if the effect is invalid or not yet implemented then no need to go further */
        if (!mechanics.contains(effect.num)) {
            continue;
        }

        //dun remove the test
        if (effect.args.size() > 0) {
            IM::poke(b,source)["ItemArg"] = effect.args;
        }
    }
}

struct IMBlackSludge : public IM
{
    IMBlackSludge() {
        functions["EndTurn6.3"] = &et; /* Gen 4 */
        functions["EndTurn5.2"] = &et; /* Gen 5 */
    }

    static void et(int s, int, BS &b) {
        if(b.koed(s) || b.hasWorkingAbility(s, Ability::MagicGuard)) {
            return;
        }
        if(b.hasType(s, Pokemon::Poison)) {
            if (b.canHeal(s)) {
                b.sendItemMessage(16,s,0);
                b.healLife(s, b.poke(s).totalLifePoints()/16);
            }
        } else
        {
            b.sendItemMessage(16,s,1);
            b.inflictDamage(s, b.poke(s).totalLifePoints()/8,s);
        }
    }
};

struct IMLeftOvers : public IM
{
    IMLeftOvers() {
        functions["EndTurn5.0"] = &et; /* Gen 2 */
        functions["EndTurn6.3"] = &et; /* Gen 3,4 */
        functions["EndTurn5.2"] = &et; /* Gen 5 */
    }

    static void et(int s, int, BS &b) {
        if (!b.canHeal(s))
            return;

        b.sendItemMessage(12,s);
        b.healLife(s, b.poke(s).totalLifePoints()/16);
    }
};

struct IMChoiceItem : public IM
{
    IMChoiceItem() {
        functions["UponSetup"] = &us;
        functions["MovesPossible"] = &mp;
        functions["BeforeTargetList"] = &btl;
        functions["AfterTargetList"] = &atl;
    }

    static void us(int s, int, BS &b) {
        poke(b,s).remove("ChoiceMemory");
    }

    static void mp(int s, int, BS &b) {
        if (!poke(b,s).contains("ChoiceMemory") || poke(b,s).value("ChoiceMemory").toInt() == 0) {
            return;
        }
        int mem = poke(b,s)["ChoiceMemory"].toInt();
        int index=-1;
        for (int i = 0; i < 4; i++) {
            if (mem  == b.move(s, i)) {
                index = i;
                break;
            }
        }
        /* Happens for example when using metronome + fly, and is correct in regard
           to the ingame behavior */
        if (index == -1)
            return;
        for (int i = 0; i < 4; i++) {
            if (index != i) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }

    static void btl(int s, int, BS &b) {
        if (b.gen() < 5)
            return;
        /* Last move used is here not to take "special occurence" moves */
        poke(b,s)["ChoiceMemory"] = poke(b,s)["LastMoveUsed"];
    }

    static void atl(int s, int, BS &b) {
        if (b.gen() > 4)
            return;
        /* Last move used is here not to take "special occurence" moves */
        poke(b,s)["ChoiceMemory"] = poke(b,s)["LastMoveUsed"];
    }
};

struct IMStatModifier : public IM
{
    IMStatModifier() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        QString args = poke(b,s)["ItemArg"].toString();
        turn(b,s)["Stat" + args.left(1) + "ItemModifier"] = args.mid(2).toInt();
    }
};

struct IMFocusBand : public IM
{
    IMFocusBand() {
        functions["BeforeTakingDamage"] = &btd;
        functions["UponSelfSurvival"] = &uodr;
    }

    static void btd(int s, int t, BS &b) {
        if (b.coinflip(1, 10)) {
            if (b.gen() <= 4)
                turn(b,s)["CannotBeKoedBy"] = t;
            else
                turn(b,s)["CannotBeKoedAt"] = b.attackCount();
        }
    }

    static void uodr(int s, int, BS &b) {
        if (b.koed(s))
            return;

        b.sendItemMessage(4, s);
        turn(b,s)["SurviveReason"] = true;
    }
};

struct IMFocusSash : public IM
{
    IMFocusSash() {
        functions["BeforeTakingDamage"] = &btd;
        functions["UponSelfSurvival"] = &uss;
    }

    static void btd(int s, int t, BS &b) {
        if(b.poke(s).isFull()) {
            if (b.gen() <= 4)
                turn(b,s)["CannotBeKoedBy"] = t;
            else
                turn(b,s)["CannotBeKoedAt"] = b.attackCount();
        }
    }

    static void uss(int s, int, BS &b) {
        if (b.koed(s))
            return;

        b.sendItemMessage(5, s);
        b.disposeItem(s);

        turn(b,s)["SurviveReason"] = true;
    }
};

struct IMLagging : public IM
{
    IMLagging() {
        functions["TurnOrder"] = &tu;
    }
    static void tu (int s, int, BS &b) {
        turn(b,s)["TurnOrder"] = -2;
    }
};

struct IMBoostPokeStat : public IM
{
    IMBoostPokeStat() {
        functions["StatModifier"] = &sm;
    }
    static void sm(int s,int, BS &b) {
        int num = b.pokenum(s).pokenum;
        QStringList args = poke(b,s)["ItemArg"].toString().split('_');
        if(!args[0].split('/').contains(QString::number(num))) {
            return;
        }
        int boost = args[1].toInt();
        for (int i = 2; i < args.size(); i++) {
            turn(b,s)["Stat" + args[i] + "ItemModifier"] = boost;
        }
    }
};

struct IMBoostCategory : public IM
{
    IMBoostCategory() {
        functions["BasePowerModifier"] = &bpm;
    }
    static void bpm(int s, int, BS &b) {
        if (tmove(b,s).category == poke(b,s)["ItemArg"]) {
            b.chainBp(s, 2);
        }
    }
};

struct IMBoostType : public IM
{
    IMBoostType() {
        functions["BasePowerModifier"] = &bpm;
    }
    static void bpm(int s, int, BS &b) {
        if (tmove(b,s).type == poke(b,s)["ItemArg"]) {
            if (b.gen() >= 4)
                b.chainBp(s, 4);
            else
                b.chainBp(s, 2);
        }
    }
};

struct IMZoomLens : public IM
{
    IMZoomLens() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int t, BS &b) {
        if (fturn(b,t).contains(TM::HasMoved)) {
            turn(b,s)["Stat6ItemModifier"] = 4;
        }
    }
};

struct IMStatusOrb : public IM
{
    IMStatusOrb() {
        functions["EndTurn6.7"] = &et; /* Gen 4 */
        functions["EndTurn26.2"] = &et; /* Gen 5 */
    }

    static void et(int s, int, BS &b) {
        if (b.poke(s).status() != Pokemon::Fine) {
            return;
        }
        int status = poke(b,s)["ItemArg"].toInt();
        if (!b.canGetStatus(s, status))
            return;
        if (status == Pokemon::Burnt) {
            b.sendItemMessage(19,s,0);
        } else {
            b.sendItemMessage(19,s,1);
        }
        b.inflictStatus(s, status, s, status == Pokemon::Poisoned ? 15: 0, status == Pokemon::Poisoned ? 15: 0);
    }
};

struct IMLifeOrb : public IM
{
    IMLifeOrb() {
        functions["Mod2Modifier"] = &m2m;
        functions["UponDamageInflicted"] = &udi;
        functions["AfterTargetList"] = &atl;
    }

    static void m2m(int s, int, BS &b) {
        turn(b,s)["ItemMod2Modifier"] = 3;
    }

    static void udi(int s, int t, BS &b) {
        if (s == t)
            return; /* life orb doesn't recoil with self damage */
        if (b.koed(s))
            return;

        /* In gen 4, it does not damage the user if the foe has a substitute. In gen 5, it does */
        if (b.gen() <= 4 && turn(b,t).contains("DamageTakenBy") && turn(b,t)["DamageTakenBy"].toInt() == s) {
            turn(b,s)["ActivateLifeOrb"] = true;
        } else if (b.gen() >= 5 && turn(b,s).contains("DamageInflicted")) {
            turn(b,s)["ActivateLifeOrb"] = true;
            turn(b,s)["LOTarget"] = t;
        }
    }

    static void atl(int s, int, BS &b) {
        if (turn(b,s).value("ActivateLifeOrb").toBool() && !turn(b,s).value("NoLifeOrbActivation").toBool() && !turn(b,s).value("EncourageBug").toBool()
                && !b.hasWorkingAbility(s, Ability::MagicGuard)) {
            if (b.gen() >= 5)
                b.sendItemMessage(21,s);

            b.inflictDamage(s,b.poke(s).totalLifePoints()/10,s);
            turn(b,s)["NoLifeOrbActivation"] = true;

            /* Self KO Clause */
            if (b.koed(s)) {
                /* In VGC 2011 (gen 5), the user of the Life Orb wins instead of losing with the Self KO Clause */
                if (b.gen() <= 4)
                    b.selfKoer() = s;
                else
                    b.selfKoer() = turn(b,s).value("LOTarget").toInt();
            }
        }
    }
};

struct IMScopeLens : public IM
{
    IMScopeLens() {
        functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
        tmove(b, s).critRaise += 1;
    }
};

struct IMShellBell : public IM
{
    IMShellBell() {
        functions["AfterAttackSuccessful"] = &udi;
    }

    static void udi(int s, int t, BS &b) {
        if (s==t)
            return;

        if (!b.canHeal(s) || turn(b,s).value("EncourageBug").toBool())
            return;

        int damage = turn(b,s)["DamageInflicted"].toInt();

        if (damage > 0) {
            b.sendItemMessage(24, s);
            b.healLife(s, damage/8);
        }
    }
};

struct IMCriticalPoke : public IM
{
    IMCriticalPoke() {
        functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
        if (b.pokenum(s).pokenum == poke(b,s)["ItemArg"].toInt()) {
            tmove(b,s).critRaise += 2;
        }
    }
};

struct IMPokeTypeBoost : public IM
{
    IMPokeTypeBoost() {
        functions["BasePowerModifier"] = &bpm;
    }
    static void bpm(int s, int, BS &b) {
        QStringList args = poke(b,s)["ItemArg"].toString().split('_');
        QStringList pokes = args[0].split('/');
        if (!pokes.contains(QString::number(b.pokenum(s).pokenum)))
            return;

        int type = tmove(b,s).type;
        for (int i = 1; i < args.size(); i++) {
            if (type == args[i].toInt())
                b.chainBp(s, 4);
        }
    }
};

struct IMStickyBarb : public IM
{
    IMStickyBarb() {
        functions["EndTurn6.19"] = &et; /* Gen 4 */
        functions["EndTurn26.2"] = &et; /* Gen 5 */
        functions["UponPhysicalAssault"] = &upa;
    }

    static void et(int s, int, BS &b) {
        if (b.hasWorkingAbility(s, Ability::MagicGuard))
            return;

        b.sendItemMessage(29,s,0);
        b.inflictDamage(s, b.poke(s).totalLifePoints()/8,s);
    }

    static void upa(int s, int t, BS &b) {
        if (!b.koed(t) && b.poke(t).item() == 0) {
            b.poke(t).item() = b.poke(s).item();
            b.disposeItem(s);
        }
    }
};

struct IMMetronome : public IM
{
    IMMetronome() {
        functions["BeforeTargetList"] = &btl;
        functions["Mod2Modifier"] = &m2m;
    }

    static void btl(int s, int, BS &b) {
        if (fturn(b,s).contains(TM::NoChoice)) {
            /* multiple turn move */
            return;
        }
        int count = poke(b,s)["IMMetroCount"].toInt();
        int lslot = poke(b,s)["IMLastMoveSlot"].toInt();
        int slot = fpoke(b,s).lastMoveSlot;
        bool act = poke(b,s)["IMMetroActivating"].toBool();
        poke(b,s)["IMLastMoveSlot"] = slot;
        poke(b,s)["IMMetroActivating"] = true;
        if (slot != lslot) {
            poke(b,s)["IMMetroCount"] = 0;
            return;
        }
        if (tmove(b,s).power == 0) {
            return;
        }
        if (act) {
            poke(b,s)["IMMetroCount"] = std::min(10, count+1);
        }
    }

    static void m2m(int s, int, BS &b) {
        turn(b,s)["ItemMod2Modifier"] = poke(b,s)["IMMetroCount"];
    }
};

struct IMQuickClaw : public IM
{
    IMQuickClaw() {
        functions["TurnOrder"] = &tu;
    }
    static void tu(int s, int, BS &b) {
        if (b.coinflip(1, 5)) {
            turn(b,s)["TurnOrder"] = 2;
            turn(b,s)["QuickClawed"] = true;
        }
    }
};

struct IMMentalHerb : public IM
{
    IMMentalHerb() {
        functions["UponSetup"] = &as;
    }

    static void as(int s, int, BS &b) {
        bool used = false;
        if (poke(b,s).contains("AttractedTo")) {
            int seducer = poke(b,s)["AttractedTo"].toInt();
            if (poke(b,seducer).contains("Attracted") && poke(b,seducer)["Attracted"].toInt() == s) {
                removeFunction(poke(b,s), "DetermineAttackPossible", "Attract");
                poke(b,s).remove("AttractedTo");
                used = true;
            }
        }
        if (b.gen() >= 5) {
            if (poke(b,s).contains("Tormented")) {
                removeFunction(poke(b,s), "MovesPossible", "Torment");
                poke(b,s).remove("Tormented");
                used = true;
            }
            if (b.counters(s).hasCounter(BC::Taunt)) {
                removeFunction(poke(b,s), "MovesPossible", "Taunt");
                removeFunction(poke(b,s), "MovePossible", "Taunt");
                b.removeEndTurnEffect(BS::PokeEffect, s, "Taunt");
                used = true;
            }
            if (b.counters(s).hasCounter(BC::Encore)) {
                removeFunction(poke(b,s), "MovesPossible", "Encore");
                b.removeEndTurnEffect(BS::PokeEffect, s, "Encore");
                used = true;
            }
            if (b.counters(s).hasCounter(BC::Disable)) {
                removeFunction(poke(b,s), "MovesPossible", "Disable");
                removeFunction(poke(b,s), "MovePossible", "Disable");
                b.removeEndTurnEffect(BS::PokeEffect, s, "Disable");
                used = true;
            }
            b.counters(s).clear();
        }
        if (used) {
            b.sendItemMessage(7,s);
            b.disposeItem(s);
        }
    }
};

struct IMWhiteHerb : public IM
{
    IMWhiteHerb() {
        functions["UponSetup"] = &as;
        functions["AfterStatChange"] = &as;
    }

    static void as(int s, int, BS &b) {
        bool act = false;
        for (int i = 1; i <= 7; i++) {
            if (fpoke(b,s).boosts[i] < 0) {
                act = true;
                fpoke(b,s).boosts[i] = 0;
            }
        }
        if (act) {
            b.sendItemMessage(3,s);
            b.disposeItem(s);
        }
    }
};

struct IMBerryJuice : public IM
{
    IMBerryJuice() {
        functions["AfterHPChange"] = &ahpc;
    }

    static void ahpc(int s, int, BS &b) {
        if (!b.canHeal(s))
            return;

        if (b.poke(s).lifePercent() <= 50) {
            b.disposeItem(s);
            b.sendItemMessage(18,s,0);
            b.healLife(s, 20);
        }
    }
};

struct IMEviolite : public IM
{
    IMEviolite() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (PokemonInfo::HasEvolutions(b.poke(s).num().pokenum)) {
            turn(b,s)["Stat2ItemModifier"] = 10;
            turn(b,s)["Stat4ItemModifier"] = 10;
        }
    }
};

struct IMRockyHelmet : public IM
{
    IMRockyHelmet() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa( int s, int t, BS &b) {
        if (!b.koed(t) && !b.hasWorkingAbility(t, Ability::MagicGuard)) {
            b.sendItemMessage(34,s,0,t);
            b.inflictDamage(t,b.poke(t).totalLifePoints()/6,s,false);

            /* In VGC 2011, the one with the rugged helmet wins */
            if (b.koed(t)) {
                b.selfKoer() = t;
            }
        }
    }
};

struct IMAirBalloon : public IM
{
    IMAirBalloon() {
        functions["UponSetup"] = &us;
        functions["UponBeingHit"] = &upbi;
    }

    static void us(int s, int, BS &b) {
        b.sendItemMessage(35, s, 1);
    }

    static void upbi(int s, int, BS &b) {
        if (b.koed(s))
            return;
        b.sendItemMessage(35,s,0);
        b.disposeItem(s);
    }
};

struct IMAbsorbBulb : public IM
{
    IMAbsorbBulb() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        int tp = poke(b,s)["ItemArg"].toString().section('_', 0, 0).toInt();
        if (!b.koed(s) && type(b,t) == tp) {
            int stat = poke(b,s)["ItemArg"].toString().section('_', 1).toInt();
            if (b.hasMaximalStatMod(s, stat))
                return;
            b.sendItemMessage(36, s, 0, t, b.poke(s).item(), stat);
            b.disposeItem(s);
            b.inflictStatMod(s, stat, 1, s, false);
        }
    }
};

struct IMGem : public IM
{
    IMGem() {
        functions["BasePowerModifier"] = &bpm;
    }

    static void bpm(int s, int t, BS &b) {
        if (s == t)
            return;

        /* Doom Desire & Future sight don't have their gem attacking right away,
           only when it hits, and then b.attacking() is false */
        if (tmove(b,s).attack == Move::FutureSight || tmove(b,s).attack == Move::DoomDesire) {
            if (b.attacking())
                return;
        }

        if (tmove(b,s).power <= 1) {
            return;
        }
        if (tmove(b,s).type != poke(b,s)["ItemArg"].toInt() || tmove(b,s).attack == Move::FirePledge  || tmove(b,s).attack == Move::GrassPledge  || tmove(b,s).attack == Move::WaterPledge )
            return;
        b.sendItemMessage(37, s, 0, 0, b.poke(s).item(), move(b,s));
        turn(b,s)["GemActivated"] = true;
        b.disposeItem(s);
    }
};

struct IMRedCard : public IM
{
    IMRedCard() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        //Red Card does not trigger if the Pokemon is phazed with Dragon Tail/Circle Throw
        if (b.koed(s) || (b.hasWorkingAbility(t, Ability::SheerForce) && turn(b,t).contains("EncourageBug")) || tmove(b,t).attack == Move::DragonTail || tmove(b,t).attack == Move::CircleThrow || (b.hasSubstitute(s) && !b.canBypassSub(t)))
            return;
        addFunction(turn(b,t), "AfterAttackFinished", "RedCard", &aaf);
        turn(b,t)["RedCardUser"] = s;
        turn(b,t)["RedCardCount"] = slot(b,t)["SwitchCount"];
        turn(b,t)["RedCardGiverCount"] = slot(b,s)["SwitchCount"];

        return;
    }

    static void aaf(int t, int, BS &b) {
        if (turn(b,t)["RedCardCount"] != slot(b,t)["SwitchCount"])
            return;
        int s = turn(b,t)["RedCardUser"].toInt();
        if (b.koed(s) || b.koed(t) || turn(b,t)["RedCardGiverCount"] != slot(b,s)["SwitchCount"])
            return;
        if (!b.hasWorkingItem(s, Item::RedCard))
            return;

        int target = b.player(t);
        if (b.countBackUp(target) == 0) {
            return;
        }

        b.sendItemMessage(38, s, 0, t);
        b.disposeItem(s);

        /* ingrain & suction cups */
        if (poke(b,t).value("Rooted").toBool()) {
            b.sendMoveMessage(107, 1, s, Pokemon::Grass, t);
            return;
        } else if (b.hasWorkingAbility(t,Ability::SuctionCups)) {
            b.sendMoveMessage(107, 0, s, 0, t);
            return;
        }

        QList<int> switches;

        for (int i = 0; i < 6; i++) {
            if (!b.isOut(target, i) && !b.poke(target,i).ko()) {
                switches.push_back(i);
            }
        }
        b.sendBack(t, true);
        b.sendPoke(t, switches[b.randint(switches.size())], true);
        b.sendMoveMessage(107,2,s,0,t);
        b.callEntryEffects(t);

        turn(b,t).remove("RedCardUser");
    }
};

struct IMEscapeButton : public IM
{
    IMEscapeButton() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (b.koed(s) || turn(b,t).value("EncourageBug").toBool() || (b.hasSubstitute(s) && !b.canBypassSub(t)))
            return;
        turn(b,s)["EscapeButtonActivated"] = true;
        turn(b,s)["EscapeButtonCount"] = slot(b,s)["SwitchCount"];

        addFunction(turn(b,t), "AfterAttackFinished", "EscapeButton", &aaf);
    }

    static void aaf(int, int, BS &b) {
        std::vector<int> speeds = b.sortedBySpeed();

        for (unsigned i = 0; i < speeds.size(); i++) {
            int p = speeds[i];
            if (!b.hasWorkingItem(p, Item::EscapeButton))
                continue;
            if (!turn(b,p).contains("EscapeButtonActivated"))
                continue;
            if (turn(b,p)["EscapeButtonCount"] != slot(b,p)["SwitchCount"])
                continue;

            b.sendItemMessage(39, p, 0);
            b.disposeItem(p);
            b.requestSwitch(p);
        }
    }
};

/* Needs a function in order for its Item argument to be registered */
struct IMDrive : public IM {
    IMDrive() {

    }
};

struct IMBerserkGene : public IM {
    IMBerserkGene() {
        functions["UponSetup"] = &os;
    }

    static void os(int s, int, BS &b) {
        b.sendItemMessage(40, s, 0);
        b.sendItemMessage(40, s, 1);
        b.inflictStatMod(s, Attack, 2, s, false);
        b.inflictConfused(s,s);
        b.disposeItem(s);

        /* in GSC cart mechanics, infinite confusion */
        if (b.isConfused(s)) {
            if (b.gen() == Gen::GoldSilver || b.gen() == Gen::Crystal) {
                poke(b,s)["ConfusedCount"] = 255;
            }
        }
    }
};

struct IMStatusHeal : public IM {
    IMStatusHeal() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int p, int s, BS &b) {
        QString sarg = poke(b,p).value("ItemArg").toString();
        int arg = sarg.section("_", 0, 0).toInt();
        bool permanent = sarg.section("_", 1) == "1";

        if (permanent) {
            turn(b,p)["PermanentItem"] = true;
        }

        if (b.koed(s))
            return;

        int status = b.poke(s).status();
        bool conf = b.isConfused(s);

        /* Confusion berry */
        if (arg == -1) {
            if (conf) {
                b.healConfused(s);
                b.sendBerryMessage(1, s, 0);
            }
            return;
        }

        /* Lum berry */
        if (conf && arg == 0) {
            b.healConfused(s);
            goto end;
        }

        if (status == Pokemon::Fine) {
            return;
        }

        /* LumBerry */
        if (arg == 0) {
            if (status == Pokemon::Fine)
                return;
            goto end;
        } else { /* Other Status Berry */
            if (status == arg) {
                goto end;
            }
        }

        return;

end:
        b.healStatus(s, status);
        b.sendBerryMessage(1, s, arg + 1);
    }
};

//*******Trainer Items*******/
struct IMPotion : public IM {
    IMPotion() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int p, int s, BS &b) {
        if (b.poke(s).isFull()) {
            return;
        }
        b.sendBerryMessage(3,s,0);
        int arg = poke(b,p).value("ItemArg").toInt();

        if (arg == 0) {
            arg = b.poke(s).totalLifePoints();
        }

        b.healLife(s, arg);
    }
};

struct IMEther : public IM {
    IMEther() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int p, int s, BS &b) {
        int attack = turn(b,p).value("ItemAttackSlot").toInt();
        int pp = poke(b,p).value("ItemArg").toInt();

        if (pp == 0) {
            pp = 99;
        }

        b.sendBerryMessage(2,s,0,0,0,b.move(s,attack));

        b.gainPP(s, attack, pp);
    }
};

struct IMElixir : public IM {
    IMElixir() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int p, int s, BS &b) {
        int pp = poke(b,p).value("ItemArg").toInt();

        if (pp == 0) {
            pp = 99;
        }

        b.sendBerryMessage(2,s,1);

        for (int i = 0; i < 4; i++) {
            if (b.move(s,i) != Move::NoMove) {
                b.gainPP(s, i, pp);
            }
        }
    }
};

struct IMRevive : public IM {
    IMRevive() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int p, int s, BS &b) {
        if (!b.koed(s)) {
            return;
        }

        int percent = poke(b,p).value("ItemArg").toInt();

        if (percent == 0) {
            percent = 100;
        }

        b.changeHp(s, b.poke(s).totalLifePoints()*percent/100);
        b.changeStatus(b.player(s), b.slotNum(s), Pokemon::Fine);

        b.sendItemMessage(1007, s);
    }
};

struct IMSacredAsh : public IM {
    IMSacredAsh() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int, int p, BS &b) {
        for (int i = 0; i < 6; i++) {
            int s = b.slot(p, i);

            b.changeHp(s, b.poke(s).totalLifePoints());
            if (b.koed(s)) {
                b.sendItemMessage(1999, s);
            }
            b.changeStatus(p, i, Pokemon::Fine);
            for (int j = 0; j < 4; j++) {
                if (b.move(s, j) != Move::NoMove) {
                    b.gainPP(s, j, 99);
                }
            }
        }
    }
};

/* Herbs reduce happiness */
struct IMHerb : public IM {
    IMHerb() {
        functions["TrainerItem"] = &ti;
    }

    static void ti(int, int s, BS &b) {
        b.poke(s).happiness() = std::max(0, int(b.poke(s).happiness())-2);
    }
};

struct IMSafetyGoggles  : public IM {
    IMSafetyGoggles() {
        functions["WeatherSpecial"] = &ws;
        functions["OpponentBlock"] = &uodr;
    }

    static void ws(int s, int, BS &b) {
        turn(b,s)["WeatherSpecialed"] = true;
    }

    static void uodr(int s, int t, BS &b) {
        if (tmove(b,t).flags & Move::PowderFlag) {
            turn(b,s)[QString("Block%1").arg(b.attackCount())] = true;
            //b.sendAbMessage(17, 0, s, t); //add message for Safety Goggles
        }
    }
};

struct IMWeaknessPolicy  : public IM {
    IMWeaknessPolicy() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (fturn(b,t).typeMod > 0 && !b.koed(s)) {
            b.sendItemMessage(43, s);
            b.disposeItem(s);
            b.inflictStatMod(s, Attack, 2, s);
            b.inflictStatMod(s, SpAttack, 2, s);
        }
    }
};

struct IMAssaultVest : public IM {
    IMAssaultVest() {
        functions["MovesPossible"] = &mp;
    }

    static void mp(int s, int, BS &b) {
        for (int i = 0; i < 4; i++) {
            if (MoveInfo::Power(b.move(s,i), b.gen()) == 0) {
                turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
            }
        }
    }
};

#define REGISTER_ITEM(num, name) mechanics[num] = IM##name(); names[num] = #name; nums[#name] = num;

void ItemEffect::init()
{
    REGISTER_ITEM(1, StatModifier);
    REGISTER_ITEM(2, ChoiceItem);
    REGISTER_ITEM(3, WhiteHerb);
    REGISTER_ITEM(4, FocusBand);
    REGISTER_ITEM(5, FocusSash);
    REGISTER_ITEM(6, Lagging);
    REGISTER_ITEM(7, MentalHerb);
    REGISTER_ITEM(8, BoostPokeStat);
    REGISTER_ITEM(9, BoostCategory);
    REGISTER_ITEM(10, BoostType);
    REGISTER_ITEM(12, LeftOvers);
    REGISTER_ITEM(15, ZoomLens);
    REGISTER_ITEM(16, BlackSludge);
    REGISTER_ITEM(17, QuickClaw);
    REGISTER_ITEM(18, BerryJuice);
    REGISTER_ITEM(19, StatusOrb);
    REGISTER_ITEM(21, LifeOrb);
    REGISTER_ITEM(22, Metronome);
    REGISTER_ITEM(23, ScopeLens);
    REGISTER_ITEM(24, ShellBell);
    REGISTER_ITEM(26, CriticalPoke);
    REGISTER_ITEM(27, PokeTypeBoost);
    REGISTER_ITEM(28, StickyBarb);
    REGISTER_ITEM(32, Drive);
    REGISTER_ITEM(33, Eviolite);
    REGISTER_ITEM(34, RockyHelmet);
    REGISTER_ITEM(35, AirBalloon);
    REGISTER_ITEM(36, AbsorbBulb); /* Cell Battery */
    REGISTER_ITEM(37, Gem);
    REGISTER_ITEM(38, RedCard);
    REGISTER_ITEM(39, EscapeButton);
    REGISTER_ITEM(40, BerserkGene);
    REGISTER_ITEM(41, AssaultVest);
    REGISTER_ITEM(42, SafetyGoggles);
    REGISTER_ITEM(43, WeaknessPolicy)
    /* Trainer items */
    REGISTER_ITEM(1000, StatusHeal);
    REGISTER_ITEM(1001, Potion);
    REGISTER_ITEM(1003, Elixir);
    REGISTER_ITEM(1004, Ether);
    REGISTER_ITEM(1006, Herb);
    REGISTER_ITEM(1007, Revive);
    REGISTER_ITEM(1999, SacredAsh);
    initBerries();
}
