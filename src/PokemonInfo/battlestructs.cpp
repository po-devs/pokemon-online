#include <algorithm>
#include <cassert>

#include "battlestructs.h"
#include "networkstructs.h"
#include "movesetchecker.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/coreclasses.h"

QString ChallengeInfo::clauseText[] =
{
    QObject::tr("Sleep Clause"),
    QObject::tr("Freeze Clause"),
    QObject::tr("Disallow Spects"),
    QObject::tr("Item Clause"),
    QObject::tr("Challenge Cup"),
    QObject::tr("No Timeout"),
    QObject::tr("Species Clause"),
    QObject::tr("Wifi Battle"),
    QObject::tr("Self-KO Clause"),
    QObject::tr("Inverted Battle")
};

QString ChallengeInfo::clauseBattleText[] =
{
    QObject::tr("Sleep Clause prevented the sleep inducing effect of the move from working."),
    QObject::tr("Freeze Clause prevented the freezing effect of the move from working."),
    QObject::tr(""),
    QObject::tr(""),
    QObject::tr(""),
    QObject::tr("The battle ended by timeout."),
    QObject::tr(""),
    QObject::tr(""),
    QObject::tr("The Self-KO Clause acted as a tiebreaker."),
    QObject::tr("")
};

QString ChallengeInfo::clauseDescription[] =
{
    QObject::tr("You can not put more than one Pokemon of the opposing team to sleep at the same time."),
    QObject::tr("You can not freeze more than one Pokemon of the opposing team at the same time."),
    QObject::tr("Nobody can watch your battle."),
    QObject::tr("No more than one of the same items is allowed per team."),
    QObject::tr("Random teams are given to trainers."),
    QObject::tr("No time limit for playing."),
    QObject::tr("One player cannot have more than one of the same pokemon per team."),
    QObject::tr("At the beginning of the battle, you can see the opponent's team and rearrange yours accordingly."),
    QObject::tr("The one who causes a tie (Recoil, Explosion, Destinybond, ...) loses the battle."),
    QObject::tr("All Type Effectivenesses are inverted (Ex: Water is weak to Fire)")
};

QString ChallengeInfo::modeText[] =
{
    QObject::tr("Singles", "Mode"),
    QObject::tr("Doubles", "Mode"),
    QObject::tr("Triples", "Mode"),
    QObject::tr("Rotation", "Mode")
};

BattleMove::BattleMove()
{
    num() = 0;
    PP() = 0;
    totalPP() = 0;
}

void BattleMove::load(Pokemon::gen gen) {
    PP() = MoveInfo::PP(num(), gen)*(num() == Move::TrumpCard ? 5 :8)/5; /* 3 PP-ups */
    totalPP() = PP();
}

DataStream & operator >> (DataStream &in, BattleMove &mo)
{
    in >> mo.num() >> mo.PP() >> mo.totalPP();

    return in;
}

DataStream & operator << (DataStream &out, const BattleMove &mo)
{
    out << mo.num() << mo.PP() << mo.totalPP();

    return out;
}

PokeBattle::PokeBattle()
{
    num() = Pokemon::NoPoke;
    ability() = 0;
    item() = 0;
    gender() = 0;
    fullStatus() = 1;
    lifePoints() = 0;
    totalLifePoints() = 1;
    level() = 100;
    happiness() = 255;
    itemUsed() = 0;
    itemUsedTurn() = 0;

    for (int i = 0; i < 6; i++) {
        dvs() << 31;
    }
    for (int i = 0; i < 6; i++) {
        evs() << 80;
    }
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

void PokeBattle::init(PokePersonal &poke)
{
    /* Checks num, ability, moves, item */
    poke.runCheck();

    num() = poke.num();

    if (num() == Pokemon::NoPoke)
        return;


    PokeGeneral p;
    p.gen() = poke.gen();
    p.num() = poke.num();
    p.load();

    QNickValidator v(NULL);

    happiness() = poke.happiness();

    item() = poke.item();
    ability() = poke.ability();

    if (item() == Item::GriseousOrb && num() != Pokemon::Giratina_O && p.gen() <= 4) {
        item() = 0;
    } else if (num() == Pokemon::Giratina_O && item() != Item::GriseousOrb) {
        num() = Pokemon::Giratina;
    }

    if (PokemonInfo::OriginalForme(num()) == Pokemon::Arceus) {
        if (ItemInfo::isPlate(item())) {
            num().subnum = ItemInfo::PlateType(item());
        } else {
            num().subnum = 0;
        }
    }
    if (PokemonInfo::OriginalForme(num()) == Pokemon::Genesect) {
        num().subnum = ItemInfo::DriveForme(item());
    }

	if (num() == Pokemon::Keldeo_R && !poke.hasMove(Move::SecretSword)) {
        num() = Pokemon::Keldeo;
	}

    Pokemon::uniqueId ori = PokemonInfo::OriginalForme(num());

    if (ori == Pokemon::Castform || ori == Pokemon::Cherrim || ori == Pokemon::Darmanitan || ori == Pokemon::Meloetta) {
        num().subnum = 0;
    }

    nick() = (v.validate(poke.nickname()) == QNickValidator::Acceptable && poke.nickname().length() <= 12) ? poke.nickname() : PokemonInfo::Name(num());

    if (GenderInfo::Possible(poke.gender(), p.genderAvail())) {
        gender() = poke.gender();
    } else {
        gender() = GenderInfo::Default(p.genderAvail());
    }

    shiny() = poke.shiny();
    level() = std::min(100, std::max(int(poke.level()), 1));

    nature() = std::min(NatureInfo::NumberOfNatures() - 1, std::max(0, int(poke.nature())));

    int curs = 0;
    QSet<int> taken_moves;
    
    for (int i = 0; i < 4; i++) {
        if (!taken_moves.contains(poke.move(i)) && poke.move(i) != 0) {
            taken_moves.insert(poke.move(i));
            move(curs).num() = poke.move(i);
            move(curs).load(poke.gen());
            ++curs;
        }
    }

    if (move(0).num() == 0) {
		num() = 0;
		return;
    }

    dvs().clear();
    for (int i = 0; i < 6; i++) {
        dvs() << std::min(std::max(poke.DV(i), quint8(0)),quint8(p.gen() <= 2? 15: 31));
    }

    evs().clear();
    for (int i = 0; i < 6; i++) {
        evs() << std::min(std::max(poke.EV(i), quint8(0)), quint8(255));
    }

    if (p.gen() >= 3) {
        int sum = 0;
        for (int i = 0; i < 6; i++) {
            //Arceus
            if (PokemonInfo::OriginalForme(num()) == Pokemon::Arceus && evs()[i] > 100 && p.gen() < 5) evs()[i] = 100;
            sum += evs()[i];
            if (sum > 510) {
                evs()[i] -= (sum-510);
                sum = 510;
            }
        }
    }

    updateStats(p.gen().num);
}

void PokeBattle::updateStats(Pokemon::gen gen)
{
    totalLifePoints() = std::max(PokemonInfo::FullStat(num(), gen.num, nature(), Hp, level(), dvs()[Hp], evs()[Hp]),1);
    setLife(totalLifePoints());

    for (int i = 0; i < 5; i++) {
        normal_stats[i] = PokemonInfo::FullStat(num(), gen.num, nature(), i+1, level(), dvs()[i+1], evs()[i+1]);
    }
}

DataStream & operator >> (DataStream &in, PokeBattle &po)
{
    in >> po.num() >> po.nick() >> po.totalLifePoints() >> po.lifePoints() >> po.gender() >> po.shiny() >> po.level() >> po.item() >> po.ability()
       >> po.happiness();

    for (int i = 0; i < 5; i++) {
        quint16 st;
        in >> st;
        po.setNormalStat(i+1, st);
    }

    for (int i = 0; i < 4; i++) {
        in >> po.move(i);
    }

    for (int i = 0; i < 6; i++) {
        in >> po.evs()[i];
    }

    for (int i = 0; i < 6; i++) {
        in >> po.dvs()[i];
    }

    return in;
}

DataStream & operator << (DataStream &out, const PokeBattle &po)
{
    out << po.num() << po.nick() << po.totalLifePoints() << po.lifePoints() << po.gender() << po.shiny() << po.level() << po.item() << po.ability()
        << po.happiness();

    for (int i = 0; i < 5; i++) {
        out << po.normalStat(i+1);
    }

    for (int i = 0; i < 4; i++) {
        out << po.move(i);
    }

    for (int i = 0; i < 6; i++) {
        out << po.evs()[i];
    }

    for (int i = 0; i < 6; i++) {
        out << po.dvs()[i];
    }

    return out;
}

ShallowBattlePoke::ShallowBattlePoke()
{
    setLife(0);
    gender() = 0;
    fullStatus() = 1;
    level() = 100;
}

ShallowBattlePoke::ShallowBattlePoke(const PokeBattle &p)
{
    init(p);
}
ShallowBattlePoke::~ShallowBattlePoke() {}

void ShallowBattlePoke::init(const PokeBattle &poke)
{
    nick() = poke.nick();
    fullStatus() = poke.fullStatus();
    num() = poke.num();
    shiny() = poke.shiny();
    gender() = poke.gender();
    setLifePercent( (poke.lifePoints() * 100) / poke.totalLifePoints() );
    if (lifePercent() == 0 && poke.lifePoints() > 0) {
        setLifePercent(1);
    }
    level() = poke.level();
}

int ShallowBattlePoke::status() const
{
    if (fullStatus() & (1 << Pokemon::Koed))
        return Pokemon::Koed;
    return intlog2(fullStatus() & (0x3F));
}

void ShallowBattlePoke::changeStatus(int status)
{
    /* Clears past status */
    fullStatus() = fullStatus() & ~( (1 << Pokemon::Koed) | 0x3F);
    /* Adds new status */
    fullStatus() = fullStatus() | (1 << status);
}

bool ShallowBattlePoke::hasStatus(int status) const
{
    return fullStatus() & (1 << status);
}

void ShallowBattlePoke::addStatus(int status)
{
    if (status <= Pokemon::Poisoned) {
        changeStatus(status);
        return;
    }

    fullStatus() = fullStatus() | (1 << status);
}

void ShallowBattlePoke::removeStatus(int status)
{
    fullStatus() = fullStatus() & ~(1 << status);
}


DataStream & operator >> (DataStream &in, ShallowBattlePoke &po)
{
    in >> po.num() >> po.nick();
    quint8 percent;
    in >> percent;
    in >> po.fullStatus() >> po.gender() >> po.shiny() >> po.level();

    po.setLifePercent(percent);

    return in;
}

DataStream & operator << (DataStream &out, const ShallowBattlePoke &po)
{
    out << po.num() << po.nick() << po.lifePercent() << po.fullStatus() << po.gender() << po.shiny() << po.level();

    return out;
}

TeamBattle::TeamBattle()
{
    for (int i = 0; i < 6; i++) {
        m_indexes[i] = i;
    }
}

TeamBattle::TeamBattle(PersonalTeam &other)
{
    resetIndexes();

    gen = other.gen();
    tier = other.defaultTier();

    if (gen < GEN_MIN || gen > GenInfo::GenMax()) {
        gen = GenInfo::GenMax();
    }

    int curs = 0;
    for (int i = 0; i < 6; i++) {
        poke(curs).init(other.poke(i));
        if (poke(curs).num() != 0) {
            ++curs;
        }
    }
}

void TeamBattle::resetIndexes()
{
    for (int i = 0; i < 6; i++) {
        m_indexes[i] = i;
    }
}

void TeamBattle::switchPokemon(int pok1, int pok2)
{
    std::swap(m_indexes[pok1],m_indexes[pok2]);
}

bool TeamBattle::invalid() const
{
    return poke(0).ko() && poke(1).ko() && poke(2).ko() && poke(3).ko() && poke(4).ko() && poke(5).ko();
}

void TeamBattle::generateRandom(Pokemon::gen gen)
{
    QList<Pokemon::uniqueId> pokes;
    for (int i = 0; i < 6; i++) {
        while(1) {
            Pokemon::uniqueId num = PokemonInfo::getRandomPokemon(gen);
            if (pokes.contains(num)) {
                continue ;
            }
            pokes.push_back(num);
            break;
        }

        PokeGeneral g;
        PokeBattle &p = poke(i);

        g.num() = pokes[i];
        p.num() = pokes[i];
        g.gen() = gen;
        g.load();

        if (gen >= GEN_MIN_ABILITIES) {
            p.ability() = g.abilities().ab(true_rand()%3);
            /* In case the pokemon has less than 3 abilities, ability 1 has 2/3 of being chosen. Fix it. */
            if (p.ability() == 0)
                p.ability() = g.abilities().ab(0);
        }


        if (g.genderAvail() == Pokemon::MaleAndFemaleAvail) {
            p.gender() = true_rand()%2 ? Pokemon::Female : Pokemon::Male;
        } else {
            p.gender() = g.genderAvail();
        }

        if (gen > 2) {
            p.nature() = true_rand()%NatureInfo::NumberOfNatures();
        }

        p.level() = PokemonInfo::LevelBalance(p.num());

        PokePersonal p2;

        for (int i = 0; i < 6; i++) {
            p2.setDV(i, true_rand() % (gen <= 2 ? 16 : 32));
        }

        if (gen <= 2) {
            for (int i = 0; i < 6; i++) {
                p2.setEV(i, 255);
            }
        } else {
            while (p2.EVSum() < 510) {
                int stat = true_rand() % 6;
                p2.setEV(stat, std::min(int(p2.EV(stat)) + (true_rand()%255), long(255)));
            }
        }

        p.dvs().clear();
        p.evs().clear();
        p.dvs() << p2.DV(0) << p2.DV(1) << p2.DV(2) << p2.DV(3) << p2.DV(4) << p2.DV(5);
        p.evs() << p2.EV(0) << p2.EV(1) << p2.EV(2) << p2.EV(3) << p2.EV(4) << p2.EV(5);

        QList<int> moves = g.moves().toList();
        QList<int> movesTaken;
        int off = false;
        for (int i = 0; i < 4; i++) {
            /* If there are no other moves possible,
               sets the remaining moves to 0 and exit the loop
               (like for weedle) */
            if (moves.size() <= i) {
                for (int j = i; j < 4; j++) {
                    p.move(j).num() = 0;
                    p.move(j).load(gen);
                }
                break;
            }
            int count = 0;
            while(++count) {
                int movenum = moves[true_rand()%moves.size()];
                if (movesTaken.contains(movenum)) {
                    continue;
                }
                if (MoveInfo::Power(movenum, gen) > 0 && movenum != Move::NaturalGift && movenum != Move::Snore && movenum != Move::Fling
                        && !MoveInfo::isOHKO(movenum, gen) && movenum != Move::DreamEater && movenum != Move::Synchronoise && movenum != Move::FalseSwipe
                        && movenum != Move::Feint) {
                    if (count > 4 || MoveInfo::Power(movenum, gen) > 50) {
                        off++;
                    }
                }
                if (i == 3 && ((count <= 6 && off < 2) || (count > 6 && count < 10 && off == 0))) {
                    continue;
                }
                movesTaken.push_back(movenum);
                p.move(i).num() = movenum;
                p.move(i).load(gen);
                break;
            }
        }

        if (movesTaken.contains(Move::Return))
            p.happiness() = 255;
        else if (movesTaken.contains(Move::Frustration))
            p.happiness() = 0;
        else
            p.happiness() = true_rand() % 256;

        if (gen >= GEN_MIN_ITEMS)
            p.item() = ItemInfo::Number(ItemInfo::SortedUsefulNames(gen)[true_rand()%ItemInfo::SortedUsefulNames(gen).size()]);

        if (ItemInfo::isPlate(p.item()) && p.num() == Pokemon::Arceus) {
            p.num() = Pokemon::uniqueId(Pokemon::Arceus, ItemInfo::PlateType(p.item()));
        }

        p.updateStats(gen);
        p.nick() = PokemonInfo::Name(p.num());
        p.fullStatus() = 0;
        p.shiny() = !(true_rand() % 50);
    }
}

PokeBattle & TeamBattle::poke(int i)
{
    return m_pokemons[m_indexes[i]];
}

const PokeBattle & TeamBattle::poke(int i) const
{
    return m_pokemons[m_indexes[i]];
}

int TeamBattle::internalId(const PokeBattle &p) const
{
    for (int i = 0; i < 6; i++) {
        if (m_pokemons + i == &p)
            return i;
    }

    return 0;
}

const PokeBattle &TeamBattle::getByInternalId(int i) const
{
    return m_pokemons[i];
}

DataStream & operator >> (DataStream &in, TeamBattle &te)
{
    for (int i = 0; i < 6; i++) {
        in >> te.poke(i);
    }

    return in;
}

DataStream & operator << (DataStream &out, const TeamBattle &te)
{
    for (int i = 0; i < 6; i++) {
        out << te.poke(i);
    }

    return out;
}

DataStream & operator >> (DataStream &in, TeamBattle::FullSerializer f) {
    in >> f.team->tier >> f.team->gen >> f.team;

    assert(f.team->gen.isValid());

    return in;
}

DataStream & operator << (DataStream &out, const TeamBattle::FullSerializer &f) {
    out << f.team->tier << f.team->gen << f.team;

    return out;
}

ShallowShownPoke::ShallowShownPoke()
{

}

void ShallowShownPoke::init(const PokeBattle &b)
{
    item = b.item() != 0;
    num = b.num();
    level = b.level();
    gender = b.gender();

    /* All arceus formes have the same icon */
    if (PokemonInfo::OriginalForme(num) == Pokemon::Arceus) {
        num = Pokemon::Arceus;
    }
}

DataStream & operator >> (DataStream &in, ShallowShownPoke &po) {
    in >> po.num >> po.level >> po.gender >> po.item;

    return in;
}

DataStream & operator << (DataStream &out, const ShallowShownPoke &po) {
    out << po.num << po.level << po.gender << po.item;

    return out;
}

ShallowShownTeam::ShallowShownTeam(const TeamBattle &t)
{
    for (int i = 0; i < 6; i++) {
        pokemons[i].init(t.poke(i));
    }
}

DataStream & operator >> (DataStream &in, ShallowShownTeam &po) {
    for (int i = 0; i < 6; i++) {
        in >> po.poke(i);
    }

    return in;
}

DataStream & operator << (DataStream &out, const ShallowShownTeam &po) {
    for (int i = 0; i < 6; i++) {
        out << po.poke(i);
    }

    return out;
}

BattleConfiguration::BattleConfiguration(const BattleConfiguration &other)
{
   gen = other.gen;
   mode = other.mode;
   clauses = other.clauses;
   ids[0] = other.ids[0];
   ids[1] = other.ids[1];
   avatar[0] = other.avatar[0];
   avatar[1] = other.avatar[1];
   teams[0] = other.teams[0];
   teams[1] = other.teams[1];
   receivingMode[0] = other.receivingMode[0];
   receivingMode[1] = other.receivingMode[1];
   teamOwnership = false;
}

BattleConfiguration &BattleConfiguration::operator =(const BattleConfiguration &other)
{
   gen = other.gen;
   mode = other.mode;
   clauses = other.clauses;
   ids[0] = other.ids[0];
   ids[1] = other.ids[1];
   avatar[0] = other.avatar[0];
   avatar[1] = other.avatar[1];
   teams[0] = other.teams[0];
   teams[1] = other.teams[1];
   receivingMode[0] = other.receivingMode[0];
   receivingMode[1] = other.receivingMode[1];
   teamOwnership = false;
   oldconf = other.oldconf;
   flags = other.flags;
   m_prop_tier = other.m_prop_tier;
   battleId = other.battleId;

   return* this;
}

BattleConfiguration::~BattleConfiguration()
{
    if (teamOwnership) {
        delete teams[0];
        delete teams[1];
    }
}

FullBattleConfiguration::FullBattleConfiguration(int battleid, int p1, int p2, const QString &tier, const ChallengeInfo &c)
{
    this->battleId = battleid;
    this->ids[0] = p1;
    this->ids[1] = p2;
    this->tier() = tier;
    this->clauses = c.clauses;
    this->flags.setFlag(Rated, c.rated);
    this->finished() = false;
}

bool FullBattleConfiguration::acceptSpectator(int player, bool authed) const
{
    if (spectators.contains(player) || this->id(0) == player || this->id(1) == player)
        return false;
    if (authed)
        return true;
    return !(clauses & ChallengeInfo::DisallowSpectator);
}

DataStream & operator >> (DataStream &in, FullBattleConfiguration &c)
{
    VersionControl v;
    in >> v;

    if (v.versionNumber != 0) {
        return in;
    }

    Flags network;

    v.stream >> network >> c.flags >> c.gen >> c.mode >> c.clauses >> c.ids[0] >> c.ids[1];
    v.stream >> c.receivingMode[0] >> c.name[0] >> c.avatar[0];

    if (c.receivingMode[0] == BattleConfiguration::Player) {
        c.teams[0] = new TeamBattle();
        v.stream >> *c.teams[0];
        c.teams[0]->name = c.name[0];
    } else {
        c.teams[0] = NULL;
    }

    v.stream >> c.receivingMode[1] >> c.name[1] >> c.avatar[1];

    if (c.receivingMode[1] == BattleConfiguration::Player) {
        c.teams[1] = new TeamBattle();
        v.stream >> *c.teams[1];
        c.teams[1]->name = c.name[1];
    } else {
        c.teams[1] = NULL;
    }

    c.teamOwnership = true;

    return in;
}

DataStream & operator << (DataStream &out, const FullBattleConfiguration &c)
{
    VersionControl v;
    v.stream << Flags() << c.flags << c.gen << c.mode << c.clauses << c.ids[0] << c.ids[1];

    v.stream << c.receivingMode[0] << c.getName(0) << c.avatar[0];

    if (c.receivingMode[0] == BattleConfiguration::Player) {
        v.stream << *c.teams[0];
    }

    v.stream << c.receivingMode[1] << c.getName(1) << c.avatar[1];

    if (c.receivingMode[1] == BattleConfiguration::Player) {
        v.stream << *c.teams[1];
    }

    out << v;
    return out;
}

BattleChoices::BattleChoices()
{
    mega = false;
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

BattleChoices BattleChoices::SwitchOnly(quint8 slot)
{
    BattleChoices ret;
    ret.disableAttacks();
    ret.numSlot = slot;

    return ret;
}

DataStream & operator >> (DataStream &in, BattleChoices &po)
{
    in >> po.numSlot >> po.switchAllowed >> po.attacksAllowed >> po.attackAllowed[0] >> po.attackAllowed[1] >> po.attackAllowed[2] >> po.attackAllowed[3] >> po.mega;
    return in;
}

DataStream & operator << (DataStream &out, const BattleChoices &po)
{
    out << po.numSlot << po.switchAllowed << po.attacksAllowed << po.attackAllowed[0] << po.attackAllowed[1] << po.attackAllowed[2] << po.attackAllowed[3] << po.mega;
    return out;
}

/* Tests if the attack chosen is allowed */
bool BattleChoice::match(const BattleChoices &avail) const
{
    if (!avail.attacksAllowed && (attackingChoice() || moveToCenterChoice() || itemChoice())) {
        return false;
    }
    if (!avail.switchAllowed && switchChoice()) {
        return false;
    }
    if (rearrangeChoice()) {
        return false;
    }

    if (attackingChoice()) {
        if (avail.struggle() != (attackSlot() == -1))
            return false;
        if (mega() && !avail.mega)
            return false;
        if (!avail.struggle()) {
            if (attackSlot() < 0 || attackSlot() > 3) {
                //Crash attempt!!
                return false;
            }
            return avail.attackAllowed[attackSlot()];
        }
        return true;
    }

    if (switchChoice()) {
        if (pokeSlot() < 0 || pokeSlot() > 5) {
            //Crash attempt!!
            return false;
        }
        return true;
    }

    if (moveToCenterChoice()) {
        return true;
    }

    if (itemChoice()) {
        return true;
    }

    //Reached if the type is not known
    return false;
}

DataStream & operator >> (DataStream &in, BattleChoice &po)
{
    in >> po.playerSlot >> po.type;

    switch (po.type) {
    case CancelType:
    case DrawType:
    case CenterMoveType:
        break;
    case SwitchType:
        in >> po.choice.switching.pokeSlot;
        break;
    case AttackType:
        in >> po.choice.attack.attackSlot >> po.choice.attack.attackTarget >> po.choice.attack.mega;
        break;
    case RearrangeType:
        for (int i = 0; i < 6; i++) {
            in >> po.choice.rearrange.pokeIndexes[i];
        }
        break;
    case ItemType:
        in >> po.choice.item.item >> po.choice.item.target >> po.choice.item.attack;
        break;
    }

    return in;
}

DataStream & operator << (DataStream &out, const BattleChoice &po)
{
    out << po.playerSlot << po.type;

    switch (po.type) {
    case CancelType:
    case CenterMoveType:
    case DrawType:
        break;
    case SwitchType:
        out << po.choice.switching.pokeSlot;
        break;
    case AttackType:
        out << po.choice.attack.attackSlot << po.choice.attack.attackTarget << po.choice.attack.mega;
        break;
    case RearrangeType:
        for (int i = 0; i < 6; i++) {
            out << po.choice.rearrange.pokeIndexes[i];
        }
        break;
    case ItemType:
        out << po.choice.item.item << po.choice.item.target << po.choice.item.attack;
        break;
    }

    return out;
}

DataStream & operator >> (DataStream &in, ChallengeInfo & c) {
    in >> c.dsc >> c.opp >> c.clauses >> c.mode >> c.team >> c.gen >> c.srctier >> c.desttier;
    return in;
}

DataStream & operator << (DataStream &out, const ChallengeInfo & c) {
    out << c.dsc <<  c.opp << c.clauses << c.mode << c.team << c.gen << c.srctier << c.desttier;
    return out;
}

DataStream & operator >> (DataStream &in, BattlePlayer & c) {
    in >> c.id >> c.name >> c.avatar >> c.rating >> c.win >> c.lose >> c.tie >> c.restrictedCount >> c.restrictedPokes >> c.teamCount >> c.maxlevel;
    return in;
}

DataStream & operator << (DataStream &out, const BattlePlayer & c) {
    out << c.id << c.name << c.avatar << c.rating << c.win << c.lose << c.tie << c.restrictedCount << c.restrictedPokes << c.teamCount << c.maxlevel;
    return out;
}

DataStream & operator >> (DataStream &in, FindBattleData &f)
{
    Flags network, data;

    in >> network >> data;

    f.rated = data[0];
    f.sameTier = data[1] || f.rated;
    f.ranged = network[0] && f.sameTier;

    if (network[0]) {
        in >> f.range;
    }

    if (network[1]) {
        in >> f.teams;
    } else {
        f.teams = 0;
    }

    if (f.range < 100)
        f.range = 100;

    return in;
}

DataStream & operator << (DataStream &out, const FindBattleData &f)
{
    Flags data, network;

    data.setFlag(0, f.rated);
    data.setFlag(1, f.sameTier);
    network.setFlag(0, f.ranged);
    network.setFlag(1, true);

    out << network << data;

    if (f.ranged) {
        out << f.range;
    }

    out << f.teams;

    return out;
}

void FindBattleDataAdv::shuffle(int total)
{
    shuffled.clear();

    for (int i = 0; i < total; i++) {
        if (teams == 0 || ((teams >> i )& 1)) {
            shuffled.push_back(i);
        }
    }

    std::random_shuffle(shuffled.begin(), shuffled.end());
}
