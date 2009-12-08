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
    activate("ItemSetup", num, source, source, b);
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
	if (turn(b,s).contains("CannotBeKoedBy") && turn(b,s)["CannotBeKoedBy"].toInt() == t) {
	    b.sendItemMessage(5, s);
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
    REGISTER_ITEM(12, LeftOvers);
    REGISTER_ITEM(16, BlackSludge);
}
