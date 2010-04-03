#include "battlestructs.h"
#include "pokemoninfo.h"
#include "networkstructs.h"
#include "movesetchecker.h"
#include "../Utilities/otherwidgets.h"

QString ChallengeInfo::clauseText[] =
{
    QObject::tr("Sleep Clause"),
    QObject::tr("Freeze Clause"),
    QObject::tr("Evasion Clause"),
    QObject::tr("OHKO Clause"),
    QObject::tr("Disallow Spects"),
    QObject::tr("Level Balance"),
    QObject::tr("Challenge Cup"),
    QObject::tr("No Time Out"),
    QObject::tr("Species Clause")
};

QString ChallengeInfo::clauseBattleText[] =
{
    QObject::tr("Sleep Clause prevented the sleep inducing effect of the move from working."),
    QObject::tr("Freeze Clause prevented the freezing effect of the move from working."),
    QObject::tr("Evasion Clause prevented the evasion increase of the move."),
    QObject::tr("OHKO Clause prevented the One Hit KO from happening."),
    QObject::tr(""),
    QObject::tr(""),
    QObject::tr(""),
    QObject::tr("The battle ended by timeout."),
    QObject::tr("")
};

QString ChallengeInfo::clauseDescription[] =
{
    QObject::tr("You can not put more than one Pokémon of the opposing team to sleep at the same time."),
    QObject::tr("You can not freeze more than one Pokémon of the opposing team at the same time."),
    QObject::tr("You can't use evasion moves like Double Team."),
    QObject::tr("You can't use One Hit KO moves like Fissure."),
    QObject::tr("Nobody can watch your battle."),
    QObject::tr("Pokémons levels are changed according to their strength."),
    QObject::tr("Random teams are given to trainers."),
    QObject::tr("No time limit for playing."),
    QObject::tr("One player cannot have more than one of the same pokemon per team.")
};

BattleMove::BattleMove()
{
    num() = 0;
    type() = 0;
    power() = 0;
    PP() = 0;
    totalPP() = 0;
}

void BattleMove::load() {
    power() = MoveInfo::Power(num());
    PP() = MoveInfo::PP(num())*8/5; /* 3 PP-ups */
    totalPP() = PP();
    type() = MoveInfo::Type(num());
}

QDataStream & operator >> (QDataStream &in, BattleMove &mo)
{
    in >> mo.num() >> mo.type() >> mo.power() >> mo.PP() >> mo.totalPP();

    return in;
}

QDataStream & operator << (QDataStream &out, const BattleMove &mo)
{
    out << mo.num() << mo.type() << mo.power() << mo.PP() << mo.totalPP();

    return out;
}

PokeBattle::PokeBattle()
{
    num() = 0;
    ability() = 0;
    item() = 0;
    gender() = 0;
    status() = Pokemon::Fine;
    lifePoints() = 0;
    totalLifePoints() = 1;
    level() = 100;
}

const BattleMove & PokeBattle::move(int i) const
{
    return m_moves[i];
}

BattleMove & PokeBattle::move(int i)
{
    return m_moves[i];
}

quint16 PokeBattle::normalStat(int stat) const
{
    return normal_stats[stat-1];
}

void PokeBattle::setNormalStat(int stat, quint16 i)
{
    normal_stats[stat-1] = i;
}

void PokeBattle::init(const PokePersonal &poke)
{
#ifdef SERVER_SIDE
    if (!PokemonInfo::Exist(poke.num())) {
	num() = 0;
	return;
    }

    PokeGeneral p;
    p.num() = poke.num();
    p.load();

    QNickValidator v(NULL);

    num() = poke.num();

    if (ItemInfo::Exist(poke.item())) {
        item() = poke.item();
    } else {
        item() = 0;
    }

    if (item() == Item::GriseousOrb && num() != Pokemon::Giratina_O) {
        item() = 0;
    } else if (num() == Pokemon::Giratina_O && item() != Item::GriseousOrb) {
        num() = Pokemon::Giratina;
    }

    nick() = v.validate(poke.nickname()) == QNickValidator::Acceptable ? poke.nickname() : PokemonInfo::Name(num());

    if (GenderInfo::Possible(poke.gender(), p.genderAvail())) {
	gender() = poke.gender();
    } else {
	gender() = GenderInfo::Default(p.genderAvail());
    }

    if (p.abilities().contains(poke.ability())) {
	ability() = poke.ability();
    } else {
	ability() = p.abilities()[0];
    }

    shiny() = poke.shiny();
    level() = std::min(100, std::max(int(poke.level()), 1));

    nature() = std::min(NatureInfo::NumberOfNatures(), std::max(0, int(poke.nature())));

    int curs = 0;
    QSet<int> invalid_moves;
    QSet<int> taken_moves;
    MoveSetChecker::isValid(num(),poke.move(0),poke.move(1),poke.move(2),poke.move(3), &invalid_moves);

    for (int i = 0; i < 4; i++) {
        if (!invalid_moves.contains(poke.move(i))) {
            if (!taken_moves.contains(poke.move(i))) {
                taken_moves.insert(poke.move(i));
		move(curs).num() = poke.move(i);
		move(curs).load();
		++curs;
	    }
	}
    }
    /* Even by removing the invalid moves indicated, the combination may still
       not be valid, so we check once more */

    if (move(0).num() == 0 || !MoveSetChecker::isValid(num(), move(0).num(), move(1).num(), move(2).num(), move(3).num())) {
	num() = 0;
	return;
    }

    for (int i = 0; i < 6; i++) {
        dvs() << std::min(std::max(poke.DV(i), quint8(0)),quint8(31));
    }

    for (int i = 0; i < 6; i++) {
        evs() << std::min(std::max(poke.EV(i), quint8(0)), quint8(255));
    }

    int sum = 0;
    for (int i = 0; i < 6; i++) {
        //Arceus
        if (num() == Pokemon::Arceus && evs()[i] > 100)
            evs()[i] = 100;
        sum += evs()[i];
	if (sum > 510) {
            evs()[i] -= (sum-510);
	    sum = 510;
	}
    }

    updateStats();
#else
    (void) poke;
#endif
}

void PokeBattle::updateStats()
{
    totalLifePoints() = std::max(PokemonInfo::FullStat(num(), nature(), Hp, level(), dvs()[Hp], evs()[Hp]),1);
    lifePoints() = totalLifePoints();

    for (int i = 0; i < 5; i++) {
        normal_stats[i] = PokemonInfo::FullStat(num(), nature(), i+1, level(), dvs()[i+1], evs()[i+1]);
    }
}

QDataStream & operator >> (QDataStream &in, PokeBattle &po)
{
    in >> po.num() >> po.nick() >> po.totalLifePoints() >> po.lifePoints() >> po.gender() >> po.shiny() >> po.level() >> po.item() >> po.ability();

    for (int i = 0; i < 5; i++) {
	quint16 st;
	in >> st;
	po.setNormalStat(i+1, st);
    }

    for (int i = 0; i < 4; i++) {
	in >> po.move(i);
    }

    return in;
}

QDataStream & operator << (QDataStream &out, const PokeBattle &po)
{
    out << po.num() << po.nick() << po.totalLifePoints() << po.lifePoints() << po.gender() << po.shiny() << po.level() << po.item() << po.ability();

    for (int i = 0; i < 5; i++) {
	out << po.normalStat(i+1);
    }

    for (int i = 0; i < 4; i++) {
	out << po.move(i);
    }

    return out;
}

ShallowBattlePoke::ShallowBattlePoke()
{
}

ShallowBattlePoke::ShallowBattlePoke(const PokeBattle &p)
{
    init(p);
}

void ShallowBattlePoke::init(const PokeBattle &poke)
{
    nick() = poke.nick();
    status() = poke.status();
    num() = poke.num();
    shiny() = poke.shiny();
    gender() = poke.gender();
    lifePercent() = (poke.lifePoints() * 100) / poke.totalLifePoints();
    if (lifePercent() == 0 && poke.lifePoints() > 0) {
	lifePercent() = 1;
    }
    level() = poke.level();
}

QDataStream & operator >> (QDataStream &in, ShallowBattlePoke &po)
{
    in >> po.num() >> po.nick() >> po.lifePercent() >> po.status() >> po.gender() >> po.shiny() >> po.level();

    return in;
}

QDataStream & operator << (QDataStream &out, const ShallowBattlePoke &po)
{
    out << po.num() << po.nick() << po.lifePercent() << po.status() << po.gender() << po.shiny() << po.level();

    return out;
}

TeamBattle::TeamBattle()
{
}

TeamBattle::TeamBattle(const TeamInfo &other)
{
    name = other.name;
    info = other.info;
    int curs = 0;
    for (int i = 0; i < 6; i++) {
	poke(curs).init(other.pokemon(i));
	if (poke(curs).num() != 0) {
	    ++curs;
	}
    }
}

bool TeamBattle::invalid() const
{
    return poke(0).num() == 0;
}

void TeamBattle::generateRandom()
{
    QList<int> pokes;
    for (int i = 0; i < 6; i++) {
        while(1) {
            int num = true_rand() % PokemonInfo::NumberOfPokemons();
            if (pokes.contains(num) || num == 0) {
                continue ;
            }
            pokes.push_back(num);
            break;
        }

        PokeGeneral g;
        PokeBattle &p = poke(i);

        g.num() = pokes[i];
        p.num() = pokes[i];
        g.load();

        p.ability() = g.abilities()[1] == 0 ? g.abilities()[0] : g.abilities()[true_rand()%g.abilities().size()];
        if (g.genderAvail() == Pokemon::MaleAndFemaleAvail) {
            p.gender() = true_rand()%2 ? Pokemon::Female : Pokemon::Male;
        } else {
            p.gender() = g.genderAvail();
        }
        p.nature() = true_rand()%NatureInfo::NumberOfNatures();
        p.level() = PokemonInfo::LevelBalance(p.num());

        PokePersonal p2;

        for (int i = 0; i < 6; i++) {
            p2.setDV(i, true_rand() % 32);
        }
        while (p2.EVSum() < 510) {
            int stat = true_rand() % 6;
            p2.setEV(stat, std::min(int(p2.EV(stat)) + (true_rand()%255), 255));
        }

        p.dvs().clear();
        p.evs().clear();
        p.dvs() << p2.DV(0) << p2.DV(1) << p2.DV(2) << p2.DV(3) << p2.DV(4) << p2.DV(5);
        p.evs() << p2.EV(0) << p2.EV(1) << p2.EV(2) << p2.EV(3) << p2.EV(4) << p2.EV(5);

        QList<int> moves = g.moves().toList();
        QList<int> movesTaken;
        for (int i = 0; i < 4; i++) {
            if (moves.size() <= i) {
                for (int j = i; j < 4; j++) {
                    p.move(j).num() = 0;
                    p.move(j).load();
                }
                break;
            }
            while(1) {
                int movenum = moves[true_rand()%moves.size()];
                if (movesTaken.contains(movenum)) {
                    continue;
                }
                movesTaken.push_back(movenum);
                p.move(i).num() = movenum;
                p.move(i).load();
                break;
            }
        }
        p.item() = ItemInfo::Number(ItemInfo::SortedNames()[true_rand()%ItemInfo::NumberOfItems()]);
        p.updateStats();
        p.nick() = PokemonInfo::Name(p.num());
        p.status() = Pokemon::Fine;
        p.shiny() = !(true_rand() % 50);
    }
}

PokeBattle & TeamBattle::poke(int i)
{
    if (i >= 0 && i < 6)
	return m_pokemons[i];
    else
	return m_pokemons[0];
}

const PokeBattle & TeamBattle::poke(int i) const
{
    if (i >= 0 && i < 6)
	return m_pokemons[i];
    else
	return m_pokemons[0];
}

QDataStream & operator >> (QDataStream &in, TeamBattle &te)
{
    for (int i = 0; i < 6; i++) {
	in >> te.poke(i);
    }

    return in;
}

QDataStream & operator << (QDataStream &out, const TeamBattle &te)
{
    for (int i = 0; i < 6; i++) {
	out << te.poke(i);
    }

    return out;
}

BattleChoices::BattleChoices()
{
    switchAllowed = true;
    attacksAllowed = true;
    std::fill(attackAllowed, attackAllowed+4, true);
}

void BattleChoices::disableSwitch()
{
    switchAllowed = false;
}

void BattleChoices::disableAttack(int attack)
{
    attackAllowed[attack] = false;
}

void BattleChoices::disableAttacks()
{
    std::fill(attackAllowed, attackAllowed+4, false);
    attacksAllowed = false;
}

BattleChoices BattleChoices::SwitchOnly()
{
    BattleChoices ret;
    ret.disableAttacks();

    return ret;
}

QDataStream & operator >> (QDataStream &in, BattleChoices &po)
{
    in >> po.switchAllowed >> po.attacksAllowed >> po.attackAllowed[0] >> po.attackAllowed[1] >> po.attackAllowed[2] >> po.attackAllowed[3];
    return in;
}

QDataStream & operator << (QDataStream &out, const BattleChoices &po)
{
    out << po.switchAllowed << po.attacksAllowed << po.attackAllowed[0] << po.attackAllowed[1] << po.attackAllowed[2] << po.attackAllowed[3];
    return out;
}

BattleChoice::BattleChoice(bool pokeswitch, qint8 numswitch)
	: pokeSwitch(pokeswitch), numSwitch(numswitch)
{
}

/* Tests if the attack chosen is allowed */
bool BattleChoice::match(const BattleChoices &avail) const
{
    if (!avail.attacksAllowed && attack()) {
	return false;
    }
    if (!avail.switchAllowed && poke()) {
	return false;
    }

    if (attack()) {
	if (avail.struggle() != (numSwitch == -1))
	    return false;
	if (!avail.struggle()) {
	    if (numSwitch < 0 || numSwitch > 3) {
		//Crash attempt!!
		return false;
	    }
	    return avail.attackAllowed[numSwitch];
	}
	return true;
    }
    if (poke()) {
	if (numSwitch < 0 || numSwitch > 5) {
	    //Crash attempt!!
	    return false;
	}
	return true;
    }
    //Never reached
    return true; // but avoids warnings anyway
}

QDataStream & operator >> (QDataStream &in, BattleChoice &po)
{
    in >> po.pokeSwitch >> po.numSwitch;
    return in;
}

QDataStream & operator << (QDataStream &out, const BattleChoice &po)
{
    out << po.pokeSwitch << po.numSwitch;
    return out;
}

QDataStream & operator >> (QDataStream &in, ChallengeInfo & c) {
    in >> c.dsc >> c.opp >> c.clauses;
    return in;
}

QDataStream & operator << (QDataStream &out, const ChallengeInfo & c) {
    out << c.dsc <<  c.opp << c.clauses;
    return out;
}

QDataStream & operator >> (QDataStream &in, FindBattleData &f)
{
    quint32 flags;

    in >> flags >> f.range;

    f.rated = flags & 0x01;
    f.sameTier = f.rated || flags & 0x2;
    f.ranged = f.sameTier && flags & 0x4;

    return in;
}

QDataStream & operator << (QDataStream &out, const FindBattleData &f)
{
    quint32 flags = 0;

    flags |= f.rated;
    flags |= f.sameTier << 1;
    flags |= f.ranged << 2;

    out << flags << f.range;

    return out;
}
