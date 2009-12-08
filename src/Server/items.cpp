#include "items.h"
#include "../PokemonInfo/pokemoninfo.h"

typedef ItemMechanics IM;
typedef BattleSituation BS;

QHash<int, ItemMechanics> ItemEffect::mechanics;
QHash<int, QString> ItemEffect::names;
QHash<QString, int> ItemEffect::nums;

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
    qDebug() << "Setup required for " << num;
    QList<ItemInfo::Effect> effects = ItemInfo::Effects(num);

    foreach(ItemInfo::Effect effect, effects) {
	qDebug() << "Setting up " << effect.num;

	/* if the effect is invalid or not yet implemented then no need to go further */
	if (!mechanics.contains(effect.num)) {
	    continue;
	}

	qDebug() << "Setup confirmed";

	if (effect.args.size() > 0) {
	    b.pokelong[source]["ItemArg"] = effect.args;
	}
    }
}

struct IMBlackSludge : public IM
{
    IMBlackSludge() {
	functions["EndTurn"] = &et;
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
	functions["EndTurn"] = &et;
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
	poke(b,s)["ChoiceMemory"] = turn(b,s)["MoveSlot"];
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
	qDebug() << "there it is: " << args;
    }
};

struct IMFocusBand : public IM
{
    IMFocusBand() {
	functions["BeforeTakingDamage"] = &btd;
	functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void btd(int s, int t, BS &b) {
	if (rand() % 10 == 0) {
	    turn(b,s)["CannotBeKoedBy"] = t;
	}
    }

    static void uodr(int s, int t, BS &b) {
	if (b.koed(s))
	    return;
	if (turn(b,s).contains("CannotBeKoedBy") && turn(b,s)["CannotBeKoedBy"].toInt() == t) {
	    b.sendItemMessage(4, s);
	}
    }
};

struct IMFocusSash : public IM
{
    IMFocusSash() {
	functions["BeforeTakingDamage"] = &btd;
	functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void btd(int s, int t, BS &b) {
	if(b.poke(s).isFull()) {
	    turn(b,s)["CannotBeKoedBy"] = t;
	}
    }

    static void uodr(int s, int t, BS &b) {
	if (b.koed(s))
	    return;
	if (turn(b,s).contains("CannotBeKoedBy") && turn(b,s)["CannotBeKoedBy"].toInt() == t) {
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
	inc(turn(b,s)["TurnOrder"], -2);
    }
};

struct IMBoostPokeStat : public IM
{
    IMBoostPokeStat() {
	functions["StatModifier"] = &sm;
    }
    static void sm(int s,int, BS &b) {
	int num = poke(b,s)["Num"].toInt();
	QStringList args = poke(b,s)["ItemArg"].toString().split('_');
	if(!args[1].split('_').contains(QString::number(num))) {
	    return;
	}
	int boost = args[0].toInt();
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
	functions["EndTurn"] = &et;
    }

    static void et(int s, int, BS &b) {
	if (b.poke(s).status() != Pokemon::Fine) {
	    return;
	}
	int status = poke(b,s)["ItemArg"].toInt();
	if (status == Pokemon::Burnt) {
	    b.sendItemMessage(19,s,0);
	} else {
	    b.sendItemMessage(19,s,1);
	}
	b.inflictStatus(s, poke(b,s)["ItemArg"].toInt());
    }
};

struct IMLifeOrb : public IM
{
    IMLifeOrb() {
	functions["Mod2Modifier"] = &m2m;
	functions["UponDamageInflicted"] = &udi;
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
	    b.sendItemMessage(21,s);
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
	if (poke(b,s)["Num"].toInt() != args[0].toInt())
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
	functions["EndTurn"] = &et;
	functions["UponPhysicalAssault"] = &upa;
    }

    static void et(int s, int, BS &b) {
	b.sendItemMessage(29,s,0);
	b.inflictDamage(s, b.poke(s).totalLifePoints()/8,s);
    }

    static void upa(int s, int t, BS &b) {
	b.sendItemMessage(29,t,1,s);
	b.inflictDamage(t,b.poke(s).totalLifePoints()/8,s);
	if (!b.koed(t) && b.poke(t).item() == 0) {
	    b.sendItemMessage(29,t,2);
	    b.poke(t).item() = b.poke(s).item();
	    b.disposeItem(s);
	}
    }
};

#define REGISTER_ITEM(num, name) mechanics[num] = IM##name(); names[num] = #name; nums[#name] = num;

void ItemEffect::init()
{
    REGISTER_ITEM(1, StatModifier);
    REGISTER_ITEM(2, ChoiceItem);
    REGISTER_ITEM(4, FocusBand);
    REGISTER_ITEM(5, FocusSash);
    REGISTER_ITEM(6, Lagging);
    REGISTER_ITEM(8, BoostPokeStat);
    REGISTER_ITEM(9, BoostCategory);
    REGISTER_ITEM(10,BoostType);
    REGISTER_ITEM(12, LeftOvers);
    REGISTER_ITEM(15, ZoomLens);
    REGISTER_ITEM(16, BlackSludge);
    REGISTER_ITEM(19, StatusOrb);
    REGISTER_ITEM(21, LifeOrb);
    REGISTER_ITEM(23, ScopeLens);
    REGISTER_ITEM(26, CriticalPoke);
    REGISTER_ITEM(27, PokeTypeBoost);
    REGISTER_ITEM(28, StickyBarb);
}
