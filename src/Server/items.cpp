#include "items.h"
#include "berries.h"
#include "../PokemonInfo/pokemoninfo.h"

typedef ItemMechanics IM;
typedef BattleSituation BS;

QTSHash<int, ItemMechanics> ItemEffect::mechanics;
QTSHash<int, QString> ItemEffect::names;
QTSHash<QString, int> ItemEffect::nums;

void ItemEffect::activate(const QString &effect, int num, int source, int target, BattleSituation &b)
{
    QList<ItemInfo::Effect> l = ItemInfo::Effects(num);

    foreach(ItemInfo::Effect e, l) {
	if (!mechanics.contains(e.num) || !mechanics[e.num].functions.contains(effect)) {
	    continue;
	}
	mechanics[e.num].functions[effect](source, target, b);
    }
}

void ItemEffect::setup(int num, int source, BattleSituation &b)
{
    QList<ItemInfo::Effect> effects = ItemInfo::Effects(num);

    foreach(ItemInfo::Effect effect, effects) {

	/* if the effect is invalid or not yet implemented then no need to go further */
	if (!mechanics.contains(effect.num)) {
	    continue;
        }

	//dun remove the test
	if (effect.args.size() > 0) {
	    b.pokelong[source]["ItemArg"] = effect.args;
	}
    }
}

struct IMBlackSludge : public IM
{
    IMBlackSludge() {
        functions["EndTurn63"] = &et;
    }

    static void et(int s, int, BS &b) {
	if(b.koed(s)) {
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
	functions["MovesPossible"] = &mp;
	functions["RegMoveSettings"] = &rms;
    }

    static void mp(int s, int, BS &b) {
	if (!poke(b,s).contains("ChoiceMemory")) {
	    return;
	}
	int mem = poke(b,s)["ChoiceMemory"].toInt();
	for (int i = 0; i < 4; i++) {
	    if (mem != i) {
		turn(b,s)["Move" + QString::number(i) + "Blocked"] = true;
	    }
	}
    }

    static void rms(int s, int, BS &b) {
	poke(b,s)["ChoiceMemory"] = poke(b,s)["MoveSlot"];
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
	}
    }

    static void uodr(int s, int t, BS &b) {
	if (b.koed(s))
	    return;
        if (turn(b,s).contains("CannotBeKoedBy") && turn(b,s)["CannotBeKoedBy"].toInt() == t && b.poke(s).lifePoints() == 1) {
	    b.sendItemMessage(4, s);
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
        int num = b.pokenum(s);
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
	if (turn(b,s)["Category"] == poke(b,s)["ItemArg"]) {
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
	if (turn(b,s)["Type"] == poke(b,s)["ItemArg"]) {
	    turn(b,s)["BasePowerItemModifier"] = 2;
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
	    turn(b,s)["Stat7ItemModifier"] = 4;
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
        b.inflictStatus(s, status, s);
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

	if (turn(b,t).contains("DamageTakenBy") && turn(b,t)["DamageTakenBy"].toInt() == s) {
            turn(b,s)["ActivateLifeOrb"] = true;
	}
    }

    static void atl(int s, int, BS &b) {
        if (turn(b,s).value("ActivateLifeOrb").toBool() && !b.hasWorkingAbility(s, Ability::MagicGuard)) {
            //b.sendItemMessage(21,s);
            b.inflictDamage(s,b.poke(s).totalLifePoints()/10,s);
        }
    }
};

struct IMScopeLens : public IM
{
    IMScopeLens() {
	functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
	inc(turn(b,s)["CriticalRaise"], 1);
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
	if (poke(b,s)["Num"] == poke(b,s)["ItemArg"]) {
	    inc(turn(b,s)["CriticalRaise"], 2);
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
        if (!pokes.contains(QString::number(b.pokenum(s))))
	    return;

	QString type = turn(b,s)["Type"].toString();
	for (int i = 1; i < args.size(); i++) {
	    if (type == args[i])
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
	if (turn(b,s)["Power"].toInt() == 0) {
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
            b.sendItemMessage(17, s);
	}
    }
};

struct IMMentalHerb : public IM
{
    IMMentalHerb() {
	functions["AfterSetup"] = &as;
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
	functions["AfterSetup"] = &as;
	functions["AfterStatChange"] = &as;
    }

    static void as(int s, int, BS &b) {
	bool act = false;
	for (int i = 1; i <= 7; i++) {
	    if (poke(b,s)["Boost" + QString::number(i)].toInt() < 0) {
		act = true;
		poke(b,s)["Boost"+ QString::number(i)] = 0;
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

    initBerries();
}
