#include "items.h"
#include "berries.h"
#include "../PokemonInfo/pokemoninfo.h"

typedef ItemMechanics IM;
typedef BattleSituation BS;

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

    activate("UponSetup", num, source, source, b);
}

struct IMBlackSludge : public IM
{
    IMBlackSludge() {
        functions["EndTurn63"] = &et;
    }

    static void et(int s, int, BS &b) {
        if(b.koed(s) || b.hasWorkingAbility(s, Ability::MagicGuard)) {
	    return;
	}
	if(b.hasType(s, Pokemon::Poison)) {
	    if (!b.poke(s).isFull()) {
		b.sendItemMessage(16,s,0);
		b.healLife(s, b.poke(s).totalLifePoints()/16);
	    }
	} else if (!b.hasType(s, Pokemon::Steel)) {
	    b.sendItemMessage(16,s,1);
	    b.inflictDamage(s, b.poke(s).totalLifePoints()/8,s);
	}
    }
};

struct IMLeftOvers : public IM
{
    IMLeftOvers() {
        functions["EndTurn63"] = &et;
    }

    static void et(int s, int, BS &b) {
	if (!b.poke(s).isFull()) {
	    b.sendItemMessage(12,s);
	    b.healLife(s, b.poke(s).totalLifePoints()/16);
	}
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
        /* In gens after 5, the user is not locked anymore
           even after tricking with a foe */
        if (b.gen() >= 5) {
            poke(b,s).remove("ChoiceMemory");
        }
    }

    static void mp(int s, int, BS &b) {
	if (!poke(b,s).contains("ChoiceMemory")) {
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
        if (b.true_rand() % 10 == 0) {
            turn(b,s)["CannotBeKoedBy"] = t;
        } else if (b.gen() >= 5){
            turn(b,s).remove("CannotBeKoedBy");
        }
    }

    static void uodr(int s, int t, BS &b) {
	if (b.koed(s))
	    return;
        if (turn(b,s).contains("CannotBeKoedBy") && turn(b,s)["CannotBeKoedBy"].toInt() == t && b.poke(s).lifePoints() == 1) {
	    b.sendItemMessage(4, s);
	}

        /* future: if another thing touches CannotBeKoedBy, make sure focus sash doesn't remove it when it shouldn't.
           Focus band is ok because you can't have 2 items at once */
        /* list of other effects: false swipe, focus band */
        /* In gen 5, focus sash doesn't block all the hits of a multi hit attack */
        if (b.gen() >= 5) {
            turn(b,s).remove("CannotBeKoedBy");
        }
    }
};

struct IMFocusSash : public IM
{
    IMFocusSash() {
	functions["BeforeTakingDamage"] = &btd;
        functions["UponSelfSurvival"] = &uodr;
    }

    static void btd(int s, int t, BS &b) {
	if(b.poke(s).isFull()) {
            turn(b,s)["CannotBeKoedBy"] = t;
	}
    }

    static void uodr(int s, int t, BS &b) {
        if (b.koed(s))
	    return;
        if (turn(b,s).contains("CannotBeKoedBy") && turn(b,s)["CannotBeKoedBy"].toInt() == t && b.poke(s).lifePoints() == 1) {
	    b.sendItemMessage(5, s);
	    b.disposeItem(s);
	}
        /* future: if another thing touches CannotBeKoedBy, make sure focus sash doesn't remove it when it shouldn't.
           Focus band is ok because you can't have 2 items at once */
        /* list of other effects: false swipe, focus band */
        /* In gen 5, focus sash doesn't block all the hits of a multi hit attack */
        if (b.gen() >= 5) {
            turn(b,s).remove("CannotBeKoedBy");
        }
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
	    turn(b,s)["BasePowerItemModifier"] = 1;
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
                turn(b,s)["BasePowerItemModifier"] = 2;
            else
                turn(b,s)["BasePowerItemModifier"] = 1;
	}
    }
};

struct IMZoomLens : public IM
{
    IMZoomLens() {
	functions["StatModifier"] = &sm;
    }

    static void sm(int s, int t, BS &b) {
	if (turn(b,t)["HasMoved"].toBool() == true) {
            turn(b,s)["Stat6ItemModifier"] = 4;
	}
    }
};

struct IMStatusOrb : public IM
{
    IMStatusOrb() {
        functions["EndTurn66"] = &et;
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
        }
    }

    static void atl(int s, int, BS &b) {
        if (turn(b,s).value("ActivateLifeOrb").toBool() && !b.hasWorkingAbility(s, Ability::MagicGuard)) {
            //b.sendItemMessage(21,s);
            b.inflictDamage(s,b.poke(s).totalLifePoints()/10,s);

            /* Self KO Clause */
            if (b.koed(s)) {
                b.selfKoer() = s;
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
	functions["UponDamageInflicted"] = &udi;
    }

    static void udi(int s, int t, BS &b) {
	if (s==t)
	    return;
	if (b.koed(s))
	    return;

	b.sendItemMessage(24, s);
	b.healLife(s, turn(b,s)["DamageInflicted"].toInt()/8);
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
		turn(b,s)["BasePowerItemModifier"] = 2;
	}
    }
};

struct IMStickyBarb : public IM
{
    IMStickyBarb() {
        functions["EndTurn618"] = &et;
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
	if (turn(b,s).contains("NoChoice")) {
	    /* multiple turn move */
	    return;
	}
	int count = poke(b,s)["IMMetroCount"].toInt();
	int lslot = poke(b,s)["IMLastMoveSlot"].toInt();
	int slot = poke(b,s)["MoveSlot"].toInt();
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
        if (b.true_rand() % 5 == 0) {
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
	if (poke(b,s).contains("AttractedTo")) {
	    int seducer = poke(b,s)["AttractedTo"].toInt();
	    if (poke(b,seducer).contains("Attracted") && poke(b,seducer)["Attracted"].toInt() == s) {
		b.sendItemMessage(7,s);
		removeFunction(poke(b,s), "DetermineAttackPossible", "Attract");
		poke(b,s).remove("AttractedTo");
		b.disposeItem(s);
	    }
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
        if (!b.koed(s) && b.poke(s).lifePercent() <= 50) {
            b.disposeItem(s);
            b.sendItemMessage(18,s,0);
            b.healLife(s, 20);
        }
    }
};

struct IMEvolutionStone : public IM
{
    IMEvolutionStone() {
        functions["StatModifier"] = &sm;
    }

    static void sm(int s, int, BS &b) {
        if (PokemonInfo::HasEvolutions(b.poke(s).num().pokenum)) {
            turn(b,s)["Stat2ItemModifier"] = 10;
            turn(b,s)["Stat4ItemModifier"] = 10;
        }
    }
};

struct IMRuggedHelmet : public IM
{
    IMRuggedHelmet() {
        functions["UponPhysicalAssault"] = &upa;
    }

    static void upa( int s, int t, BS &b) {
        if (!b.koed(t)) {
            b.sendItemMessage(34,s,0,t);
            b.inflictDamage(t,b.poke(t).totalLifePoints()/6,s,false);
        }
    }
};

struct IMBalloon : public IM
{
    IMBalloon() {
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

struct IMBulb : public IM
{
    IMBulb() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (!b.koed(s) && type(b,t) == poke(b,s)["ItemArg"].toInt()) {
            int stat;
            if (b.poke(s).item() == Item::RechargeableBattery) {
                if (b.hasMaximalStatMod(s, Attack))
                    return;
                stat = Attack;
            } else {
                if (b.hasMaximalStatMod(s, SpAttack))
                    return;
                stat = SpAttack;
            }
            b.sendItemMessage(36, s, 0, t, b.poke(s).item(), stat);
            b.disposeItem(s);
            b.inflictStatMod(s, SpAttack, 1, s, false);
        }
    }
};

struct IMJewel : public IM
{
    IMJewel() {
        functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
        if (tmove(b,s).power <= 1) {
            return;
        }
        if (tmove(b,s).type != poke(b,s)["ItemArg"].toInt())
            return;
        b.sendItemMessage(37, s, 0, 0, b.poke(s).item(), move(b,s));
        tmove(b,s).power = tmove(b,s).power * 3 / 2;
        b.disposeItem(s);
    }
};

struct IMRedCard : public IM
{
    IMRedCard() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (b.koed(s))
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
        if (b.koed(s) || turn(b,t)["RedCardGiverCount"] != slot(b,s)["SwitchCount"])
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
            b.sendMoveMessage(107, 1, s, Pokemon::Grass,t);
            return;
        } else if (b.hasWorkingAbility(t,Ability::SuctionCups)) {
            b.sendMoveMessage(107, 0, s, 0,t);
            return;
        }

        QList<int> switches;

        for (int i = 0; i < 6; i++) {
            if (!b.isOut(target, i) && !b.poke(target,i).ko()) {
                switches.push_back(i);
            }
        }
        b.sendBack(t, true);
        b.sendPoke(t, switches[b.true_rand()%switches.size()], true);
        b.sendMoveMessage(107,2,s,0,t);
        b.callEntryEffects(t);
    }
};

struct IMEscapeButton : public IM
{
    IMEscapeButton() {
        functions["UponBeingHit"] = &ubh;
    }

    static void ubh(int s, int t, BS &b) {
        if (b.koed(s) || b.hasSubstitute(s))
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
struct IMCassette : public IM {
    IMCassette() {

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
    REGISTER_ITEM(10,BoostType);
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
    REGISTER_ITEM(32, Cassette);
    REGISTER_ITEM(33, EvolutionStone);
    REGISTER_ITEM(34, RuggedHelmet);
    REGISTER_ITEM(35, Balloon);
    REGISTER_ITEM(36, Bulb);
    REGISTER_ITEM(37, Jewel);
    REGISTER_ITEM(38, RedCard);
    REGISTER_ITEM(39, EscapeButton);

    initBerries();
}
