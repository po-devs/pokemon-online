namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include <QtXml>
#include <cmath>
#include <ctime>
#include <cassert>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/battlestructs.h"
#include "tier.h"
#include "tiermachine.h"
#include "security.h"
#include "server.h"
#include "waitingobject.h"
#include "loadinsertthread.h"

QString MemberRating::toString() const
{
    return name + "%" + QString::number(matches).rightJustified(5,'0',true) + "%" +
            QString::number(rating).rightJustified(5,0,true) + "%" + QString::number(displayed_rating).rightJustified(5,0,true) + "%" +
            QString::number(last_check_time).rightJustified(10,'0',true) + "%" + QString::number(bonus_time).rightJustified(10,'0',true) + "\n";
}

/* Explanations here: http://pokemon-online.eu/forums/showthread.php?3045-How-to-change-the-rating-system-to-include-auto-decrease
   and here: http://pokemon-online.eu/forums/showthread.php?4189-New-Rating-system&p=40368#post40368 */
void MemberRating::changeRating(int opponent_rating, bool win)
{
    QPair<int,int> change = pointChangeEstimate(opponent_rating);

    if (matches < 9999) {
        matches += 1;
    }

    rating = rating + (win ? change.first : change.second);

    const int hpp = TierMachine::obj()->hours_per_period;
    const int msp = TierMachine::obj()->max_saved_periods;
    const int mpd = TierMachine::obj()->max_percent_decay;
    const int ppp = TierMachine::obj()->percent_per_period;

    bonus_time = bonus_time + (hpp* 3600);

    if (bonus_time > msp * hpp * 3600)
        bonus_time = msp * hpp * 3600;
    else if (bonus_time < 0 && ((-bonus_time)/(hpp*3600)*ppp) > mpd) {
        bonus_time = - ((mpd/ppp)+1) * hpp * 3600;
    }

}

void MemberRating::calculateDisplayedRating()
{
    int cur_time = time(NULL);
    int diff = cur_time - last_check_time;
    last_check_time = cur_time;

    bonus_time = bonus_time - diff;

    const int hpp = TierMachine::obj()->hours_per_period;
    const int msp = TierMachine::obj()->max_saved_periods;
    if (bonus_time > msp * hpp * 3600)
        bonus_time = msp * hpp * 3600;

    /* We don't do a min check on the time, in order to let alts be 3 months old and get deleted.*/

    if (bonus_time > 0)
        displayed_rating = rating;
    else {
        int percent =  ((-bonus_time)/(hpp*3600))*TierMachine::obj()->percent_per_period;

        if (percent > TierMachine::obj()->max_percent_decay) {
            percent = TierMachine::obj()->max_percent_decay;
        }

        displayed_rating = 1000 + (rating-1000) * (100 - percent) / 100;
    }
}

QPair<int, int> MemberRating::pointChangeEstimate(int opponent_rating)
{
    int n = matches;

    int kfactor;
    if (n <= 5) {
        static const int kfactors[] = {100, 90, 80, 70, 60, 50};
        kfactor = kfactors[n];
    } else {
        kfactor = 32;
    }
    double myesp = 1/(1+ pow(10., (float(opponent_rating)-rating)/400));

    /* The +0.5 is a trick to round the value instead of flooring it */
    return QPair<int,int>(int((1. - myesp)*kfactor+0.5),-int(myesp*kfactor +0.5));
}

void Tier::changeName(const QString &name)
{
    this->m_name = name;
}

void Tier::changeId(int id)
{
    m_id = id;
}

int Tier::make_query_number(int type)
{
    /* boss->version is only updated in the main thread, so this call is safe
        as make_query_number is only called in the main thread too */
    return (type << 10) + id() + (boss->version << 16);
}

void Tier::loadFromFile()
{
    ratings.clear();
    rankings = decltype(rankings)();

    delete in;
    in = new QFile("serverdb/tier_" + name() + ".txt");
    in->open(QIODevice::ReadOnly);

    QStringList members = QString::fromUtf8(in->readAll()).split('\n');

    foreach(QString member, members) {
        QStringList mmr = member.split('%');
        if (mmr.size() != 3 && mmr.size() < 6)
            continue;
        if (!SecurityManager::exist(mmr[0]))
            continue;

        MemberRating m;
        m.name = mmr[0];
        m.matches = mmr[1].toInt();
        m.rating = mmr[2].toInt();

        if (mmr.size() == 3) {
            m.displayed_rating = m.rating;
            m.last_check_time = time(nullptr);
            m.bonus_time = 0;
        } else {
            m.displayed_rating = mmr[3].toInt();
            m.last_check_time = mmr[4].toInt();
            m.bonus_time = mmr[5].toInt();
        }

        m.node = rankings.insert(m.displayed_rating, m.name);
        ratings[m.name] = m;
    }

    in->close();
    in->open(QIODevice::WriteOnly);

    int pos = 0;

    for (auto it = ratings.begin(); it != ratings.end(); ++it) {
        it->second.filePos = pos;
        in->write(it->second.toString().toUtf8());
        in->putChar('\n');
        pos = in->pos();
    }
    lastFilePos = pos;
    in->close();
    in->open(QIODevice::ReadWrite);
}

int Tier::count()
{
    return ratings.size();
}

void Tier::addBanParent(Tier *t)
{
    if (!t) {
        parent = NULL;
        return;
    }

    Tier *it = t;

    while (it != NULL) {
        if (it == this) {
            parent = NULL;
            return;
        }

        it = it->parent;
    }

    /* No cyclic tree, so we can assign it */
    parent = t;
}

QString Tier::bannedReason(const PokeBattle &p) const {
    QString errorList = "";
    if(banPokes) {
        if(bannedPokes.contains(PokemonInfo::NonAestheticForme(p.num()))) {
            errorList += QString("Pokemon %1 is banned, ").arg(PokemonInfo::Name(p.num()));
        }
        if(bannedMoves.size() > 0) {
            for(int i = 0; i < 4; i++) {
                if(bannedMoves.contains(p.move(i).num())) {
                    errorList += QString("Move %1 is banned, ").arg(MoveInfo::Name(p.move(i).num()));
                }
            }
        }
        if(bannedItems.contains(p.item())) {
            errorList += QString("Item %1 is banned, ").arg(ItemInfo::Name(p.item()));
        }
    } else {
        if(bannedPokes.size() > 0 && !bannedPokes.contains(PokemonInfo::NonAestheticForme(p.num()))) {
            errorList += QString("Pokemon %1 is banned, ").arg(PokemonInfo::Name(p.num()));
        }
        if(bannedMoves.size() > 0) {
            for(int i = 0; i < 4; i++) {
                if(p.move(i).num() != 0 && !bannedMoves.contains(p.move(i).num())) {
                    errorList += QString("Move %1 is banned, ").arg(MoveInfo::Name(p.move(i).num()));
                }
            }
        }
        if(bannedItems.size() > 0 && p.item() != 0 && !bannedItems.contains(p.item())) {
            errorList += QString("Item %1 is banned, ").arg(ItemInfo::Name(p.item()));
        }
    }
    if(errorList.length() >= 2) {
        errorList.resize(errorList.size()-2);
        errorList += ".";
    }
    return errorList;
}

bool Tier::isBanned(const PokeBattle &p) const {
    if (banPokes) {
        if (bannedPokes.contains(PokemonInfo::NonAestheticForme(p.num())))
            return true;
        if (bannedMoves.size() > 0) {
            for (int i = 0; i < 4; i++) {
                if (bannedMoves.contains(p.move(i).num())) {
                    return true;
                }
            }
        }
        if (bannedItems.contains(p.item())) {
            return true;
        }
//        if (bannedSets.contains(p.num())) {
//            foreach (BannedPoke b, bannedSets.values(p.num())) {
//                if (b.isBanned(p))
//                    return true;
//            }
//        }
    } else {
        /* The mode is the "restrict" mode, so instead we force the pokemons to have
           some characteristics */
        if (bannedPokes.size() > 0 && !bannedPokes.contains(PokemonInfo::NonAestheticForme(p.num())))
            return true;
        if (bannedMoves.size() > 0) {
            for (int i = 0; i < 4; i++) {
                if (p.move(i).num() != 0 && !bannedMoves.contains(p.move(i).num())) {
                    return true;
                }
            }
        }
        if (bannedItems.size() > 0 && p.item() != 0 && !bannedItems.contains(p.item())) {
            return true;
        }
//        if (bannedSets.size() > 0 && bannedSets.contains(p.num())) {
//            foreach (BannedPoke b, bannedSets.values(p.num())) {
//                if (b.isForcedMatch(p))
//                    goto afterloop;
//            }
//            return true;
//            afterloop:;
//        }
    }

    if (parent) {
        return parent->isBanned(p);
    } else {
        return false;
    }
}

bool Tier::isRestricted(const PokeBattle &p) const
{
    if (restrictedPokes.contains(PokemonInfo::NonAestheticForme(p.num())))
        return true;

//    if (restrictedSets.contains(p.num())) {
//        foreach (BannedPoke set, restrictedSets) {
//            if (set.isBanned(p))
//                return true;
//        }
//    }

    return false;
}

bool Tier::exists(const QString &name)
{
    if (!holder.isInMemory(name))
        loadMemberInMemory(name);
    return holder.exists(name);
}

int Tier::ranking(const QString &name)
{
    if (!exists(name))
        return -1;
    return ratings.at(name).node->ranking();
}

bool Tier::isValid(const TeamBattle &t)  const
{
    if (!allowGen(t.gen)) {
        return false;
    }

    int count = 0;
    int restricted = 0;

    for (int i = 0; i< 6; i++) {
        if (t.poke(i).num() != 0) {
            if (isBanned(t.poke(i)))
                return false;
            if (isRestricted(t.poke(i))) {
                restricted += 1;

                if (restricted > maxRestrictedPokes && count < numberOfPokemons) {
                    return false;
                }
            }

            count += 1;
        }
    }

    return true;
}

void Tier::fixTeam(TeamBattle &t) const
{
    for (int i = 0; i < 6; i ++) {
        if (i >= numberOfPokemons) {
            t.poke(i).num() = 0;
            continue;
        }
    }
}

quint8 Tier::restricted(TeamBattle &t) const
{
    int ret = 0;

    for (int i = 0; i < 6; i++) {
        if (isRestricted(t.poke(i))) {
            ret |= 1 << i;
        }
    }

    return ret;
}

void Tier::changeRating(const QString &player, int newRating)
{
    assert(exists(player));

    ratings[player].rating = newRating;
    updateMember(ratings[player]);
}

void Tier::changeRating(const QString &w, const QString &l)
{
    /* Not really necessary, as pointChangeEstimate should always be called
       at the beginning of the battle, but meh maybe it's not a battle */
    bool addw(false), addl(false);
    addw = !exists(w);
    addl = !exists(l);

    MemberRating win = addw ? MemberRating(w) : member(w);
    MemberRating los = addl ? MemberRating(l) : member(l);

    int oldwin = win.rating;
    win.changeRating(los.rating, true);
    los.changeRating(oldwin, false);
    win.calculateDisplayedRating();
    los.calculateDisplayedRating();

    updateMember(win, addw);
    updateMember(los, addl);
}

MemberRating Tier::member(const QString &name)
{
    if (!holder.isInMemory(name))
        loadMemberInMemory(name);

    return holder.member(name);
}

int Tier::rating(const QString &name)
{
    if (!holder.isInMemory(name))
        loadMemberInMemory(name);
    if (exists(name)) {
        return holder.member(name).displayed_rating;
    } else {
        return 1000;
    }
}

int Tier::inner_rating(const QString &name)
{
    if (!holder.isInMemory(name))
        loadMemberInMemory(name);
    if (exists(name)) {
        return holder.member(name).rating;
    } else {
        return 1000;
    }
}

int Tier::ratedBattles(const QString &name)
{
    if (!holder.isInMemory(name))
        loadMemberInMemory(name);
    if (exists(name)) {
        return holder.member(name).matches;
    } else {
        return 0;
    }
}

void Tier::loadMemberInMemory(const QString &name, QObject *o, const char *slot)
{
    if (!o) {
        if (holder.isInMemory(name))
            return;

        processQuery(name, GetInfoOnUser, nullptr);

        return;
    }

    holder.cleanCache();

    WaitingObject *w = WaitingObjects::getObject();

    /* It is important that this connect is done before the connect to freeObject(),
       because then the user at the signal's reception can use the object at will knowing it's not already
       used by another Player or w/e */
    QObject::connect(w, SIGNAL(waitFinished()), o, slot);
    QObject::connect(w, SIGNAL(waitFinished()), WaitingObjects::getInstance(), SLOT(freeObject()));

    if (holder.isInMemory(name)) {
        w->setProperty("tier", this->name());
        w->emitSignal();
    }
    else {
        LoadInsertThread<MemberRating> *t = getThread();

        t->pushQuery(name, w, make_query_number(GetInfoOnUser));
    }
}

void Tier::fetchRankings(const QVariant &data, QObject *o, const char *slot)
{
    WaitingObject *w = WaitingObjects::getObject();

    /* It is important that this connect is done before the connect to freeObject(),
       because then the user at the signal's reception can use the object at will knowing it's not already
       used by another Player or w/e */
    QObject::connect(w, SIGNAL(waitFinished()), o, slot);
    QObject::connect(w, SIGNAL(waitFinished()), WaitingObjects::getInstance(), SLOT(freeObject()));

#if 0
    LoadThread *t = getThread();

    t->pushQuery(data, w, make_query_number(GetRankings));
#else
    processQuery(data, GetRankings, w);
#endif
}

void Tier::fetchRanking(const QString &name, QObject *o, const char *slot)
{
    WaitingObject *w = WaitingObjects::getObject();

    /* It is important that this connect is done before the connect to freeObject(),
       because then the user at the signal's reception can use the object at will knowing it's not already
       used by another Player or w/e */
    QObject::connect(w, SIGNAL(waitFinished()), o, slot);
    QObject::connect(w, SIGNAL(waitFinished()), WaitingObjects::getInstance(), SLOT(freeObject()));

#if 0
    LoadThread *t = getThread();

    t->pushQuery(name.toLower(), w, make_query_number(GetRanking));
#else
    processQuery(name, GetRanking, w);
#endif
}

/* Precondition: name is in lowercase */
void Tier::processQuery(const QVariant &name, int type, WaitingObject *w)
{
    if (w) {
        w->setProperty("tier", this->name());
    }
    if (type == GetInfoOnUser) {
        assert(0);//Should never reach here
    } else if (type == GetRankings) {
        int page;

        if (name.type() == QVariant::String) {
            int r = ranking(name.toString());
            page = (r-1)/TierMachine::playersByPage + 1;
        }
        else {
            page = name.toInt();
        }

        /* A page is 40 players */
        int startingRank = (page-1) * TierMachine::playersByPage + 1;

        RankingTree<QString>::iterator it = rankings.getByRanking(startingRank);

        QVector<QPair<QString, int> > results;
        results.reserve(TierMachine::playersByPage);

        int i = 0;
        while (i < TierMachine::playersByPage && it.p != NULL)
        {
            i++;
            results.push_back(QPair<QString, int> (it->data, it->key));
            --it;
        }

        w->data["rankingpage"] = page;
        w->data["tier"] = this->name();
        w->data["rankingdata"] = QVariant::fromValue(results);
    } else if (type == GetRanking) {
        w->setProperty("ranking", ranking(name.toString()));
    }
}

void Tier::insertMember(void *data, int update)
{
    MemberRating &m = *(MemberRating*) data;

    if (update) {
        in->seek(m.filePos);
        in->write(m.toString().toUtf8());

        ratings[m.name].node = rankings.changeKey(m.node.node(), m.rating);
    } else {
        m.filePos = lastFilePos;
        m.node = rankings.insert(m.rating, m.name);
        in->seek(lastFilePos);
        in->write(m.toString().toUtf8());
        in->putChar('\n');
        lastFilePos = in->pos();
        ratings[m.name] = m;
    }
}

void Tier::updateMember(MemberRating &m, bool add)
{
    holder.addMemberInMemory(m);

    updateMemberInDatabase(m, add);
}

void Tier::updateMemberInDatabase(MemberRating &m, bool add)
{
    /* Can't thread writes since manipulating data directly */
#if 0
    boss->thread->pushMember(m, make_query_number(add ? InsertMember : UpdateMember));
#else
    insertMember(&m, add ? InsertMember : UpdateMember);
#endif
}

void Tier::loadFromXml(const QDomElement &elem)
{
    banPokes = elem.attribute("banMode", "ban") == "ban";
    banParentS = elem.attribute("banParent");
    parent = NULL;
    changeName(elem.attribute("name"));

    m_gen = Pokemon::gen(elem.attribute("gen", QString::number(GenInfo::GenMax())).toInt(),
                         elem.attribute("subgen",QString::number(GenInfo::NumberOfSubgens(elem.attribute("gen", QString::number(GenInfo::GenMax())).toInt())-1)).toInt());
    if (m_gen.num == 0) {
        m_gen.subnum = 0;
    }

    maxLevel = elem.attribute("maxLevel", "100").toInt();
    numberOfPokemons = elem.attribute("numberOfPokemons", "6").toInt();
    maxRestrictedPokes = elem.attribute("numberOfRestricted", "1").toInt();
    mode = elem.attribute("mode", "0").toInt();
    if (mode < ChallengeInfo::Singles || mode > ChallengeInfo::Rotation) {
        mode = ChallengeInfo::Singles;
    }
    displayOrder = elem.attribute("displayOrder", "0").toInt();

    clauses = 0;
//    bannedSets.clear(); subgen="1"
//    restrictedSets.clear();

    importBannedMoves(elem.attribute("moves"));
    importBannedItems(elem.attribute("items"));
    importBannedPokes(elem.attribute("pokemons"));
    importRestrictedPokes(elem.attribute("restrictedPokemons"));

    QStringList clausesL = elem.attribute("clauses").split(",");
    foreach(QString clause, clausesL) {
        int index = ChallengeInfo::clause(clause.trimmed());

        if (index > -1)  {
            clauses |= 1 << index;
        }
    }

//    QDomElement pokElem = elem.firstChildElement("sets");
//    if (!pokElem.isNull()) {
//        pokElem = elem.firstChildElement("pokemon");

//        while (!pokElem.isNull()) {
//            BannedPoke p;
//            p.loadFromXml(elem);

//            bannedSets.insert(p.poke, p);

//            pokElem = elem.nextSiblingElement("pokemon");
//        }
//    }

//    pokElem = elem.firstChildElement("restrictedSets");
//    if (!pokElem.isNull()) {
//        pokElem = elem.firstChildElement("pokemon");

//        while (!pokElem.isNull()) {
//            BannedPoke p;
//            p.loadFromXml(elem);

//            restrictedSets.insert(p.poke, p);

//            pokElem = elem.nextSiblingElement("pokemon");
//        }
//    }
}

void Tier::resetLadder()
{
    ratings.clear();
    rankings = decltype(rankings)();

    in->remove();
    in->open(QIODevice::ReadWrite);
}

void Tier::clearCache()
{
    holder.clearCache();
}

QDomElement & Tier::toXml(QDomElement &dest) const {
    dest.setAttribute("name", name());

    if (banPokes) {
        dest.setAttribute("banMode", "ban");
    } else {
        dest.setAttribute("banMode", "restrict");
    }

    dest.setAttribute("banParent", banParentS);
    dest.setAttribute("gen", m_gen.num);
    dest.setAttribute("subgen", m_gen.subnum);
    dest.setAttribute("maxLevel", maxLevel);
    dest.setAttribute("numberOfPokemons", numberOfPokemons);
    dest.setAttribute("numberOfRestricted", maxRestrictedPokes);
    dest.setAttribute("mode", mode);
    dest.setAttribute("displayOrder", displayOrder);
    dest.setAttribute("moves", getBannedMoves());
    dest.setAttribute("items", getBannedItems());
    dest.setAttribute("pokemons", getBannedPokes());
    dest.setAttribute("restrictedPokemons", getRestrictedPokes());

    if (clauses != 0) {
        QStringList res;
        int clauses = this->clauses;
        int i = 0;
        while (clauses > 0) {
            if (clauses % 2 == 1) {
                res.append(ChallengeInfo::clause(i));
            }
            clauses /= 2;
            i++;
        }
        res.sort();
        dest.setAttribute("clauses", res.join(","));
    }

//    QDomDocument doc;

//    if (bannedSets.size() > 0) {
//        QDomElement elem = doc.createElement("sets");
//        foreach(BannedPoke b, bannedSets) {
//            QDomElement belem = doc.createElement("pokemon");
//            b.toXml(belem);
//            elem.appendChild(belem);
//        }
//        dest.appendChild(elem);
//    }

//    if (restrictedSets.size() > 0) {
//        QDomElement elem = doc.createElement("restrictedSets");
//        foreach(BannedPoke b, restrictedSets) {
//            QDomElement belem = doc.createElement("pokemon");
//            b.toXml(belem);
//            elem.appendChild(belem);
//        }
//        dest.appendChild(elem);
//    }

    return dest;
}

QString Tier::getBannedPokes() const
{
    QStringList bannedPokesS;
    foreach(Pokemon::uniqueId poke, bannedPokes) {
        bannedPokesS.append(PokemonInfo::Name(poke));
    }
    bannedPokesS.sort();
    return bannedPokesS.join(", ");
}

QString Tier::getBannedItems() const
{
    QStringList bannedItemsS;
    foreach(int item, bannedItems) {
        bannedItemsS.append(ItemInfo::Name(item));
    }
    bannedItemsS.sort();
    return bannedItemsS.join(", ");
}

QString Tier::getBannedMoves() const
{
    QStringList bannedMovesS;
    foreach(int move, bannedMoves) {
        bannedMovesS.append(MoveInfo::Name(move));
    }
    bannedMovesS.sort();
    return bannedMovesS.join(", ");
}

QString Tier::getRestrictedPokes() const
{
    QStringList restrictedPokesS;
    foreach(Pokemon::uniqueId poke, restrictedPokes) {
        restrictedPokesS.append(PokemonInfo::Name(poke));
    }
    restrictedPokesS.sort();
    return restrictedPokesS.join(", ");
}

void Tier::importBannedPokes(const QString &s)
{
    bannedPokes.clear();
    if (s.length() == 0)
        return;
    QStringList pokes = s.split(",");
    foreach (QString poke, pokes) {
        Pokemon::uniqueId num = PokemonInfo::Number(poke.trimmed());
        if (num != 0)
            bannedPokes.insert(num);
    }
}

void Tier::importBannedItems(const QString &s)
{
    bannedItems.clear();
    if (s.length() == 0)
        return;
    QStringList items = s.split(",");
    foreach (QString item, items) {
        int num = ItemInfo::Number(item.trimmed());
        if (num != Pokemon::NoPoke)
            bannedItems.insert(num);
    }
}

void Tier::importBannedMoves(const QString &s)
{
    bannedMoves.clear();
    if (s.length() == 0)
        return;
    QStringList moves = s.split(",");
    foreach(QString move, moves) {
        int num = MoveInfo::Number(move.trimmed());

        if (num != 0)
            bannedMoves.insert(num);
    }
}

void Tier::importRestrictedPokes(const QString &s)
{
    restrictedPokes.clear();
    if (s.length() == 0)
        return;
    QStringList rpokes = s.split(",");
    foreach (QString poke, rpokes) {
        Pokemon::uniqueId num = PokemonInfo::Number(poke.trimmed());
        if (num != Pokemon::NoPoke)
            restrictedPokes.insert(num);
    }
}

//void BannedPoke::loadFromXml(const QDomElement &elem) {
//    poke = PokemonInfo::Number(elem.attribute("name"));
//    item = ItemInfo::Number("item");

//    QStringList moves = elem.attribute("moves").split(",");
//    foreach (QString move, moves) {
//        int num = MoveInfo::Number(move.trimmed());

//        if (num != 0) {
//            this->moves.insert(num);
//        }
//    }
//}

//QDomElement &BannedPoke::toXml(QDomElement &dest) const {
//    dest.setAttribute("name", PokemonInfo::Name(poke));
//    if (item > 0)
//        dest.setAttribute("item", ItemInfo::Name(item));
//    if (moves.size() > 0) {
//        QStringList res;
//        foreach(int move, moves) {
//            res.append(MoveInfo::Name(move));
//        }
//        res.sort();
//        dest.setAttribute("moves", res.join(","));
//    }

//    return dest;
//}

//bool BannedPoke::isBanned(const PokeBattle &poke) const
//{
//    if (moves.size() == 0 && item == 0)
//        return true;
//    if (item != 0 && poke.item() == item) {
//        return true;
//    }
//    if (moves.size() > 0) {
//        for (int i = 0; i < 4; i++) {
//            if (moves.contains(poke.move(i).num())) {
//                return true;
//            }
//        }
//    }

//    return false;
//}

//bool BannedPoke::isForcedMatch(const PokeBattle &poke) const
//{
//    if (moves.size() == 0 && item == 0)
//        return true;
//    if (item != 0 && poke.item() != item) {
//        return false;
//    }
//    if (moves.size() > 0) {
//        for (int i = 0; i < 4; i++) {
//            if (!moves.contains(poke.move(i).num())) {
//                return false;
//            }
//        }
//    }
//    return true;
//}

Tier::Tier(TierMachine *boss, TierCategory *cat) : boss(boss), node(cat), holder(1000) {
    in = nullptr;
    banPokes = true;
    parent = nullptr;
    m_gen = Pokemon::gen(GenInfo::GenMax(), GenInfo::NumberOfSubgens(GenInfo::GenMax())-1);
    maxLevel = 100;
    numberOfPokemons = 6;
    maxRestrictedPokes = 1;
    mode = ChallengeInfo::Singles;
    displayOrder = 0;

    clauses = 0;
}

Tier::~Tier()
{
    delete in, in = nullptr;
    qDebug() << "Deleting tier " << name();
}

void Tier::kill() {
    node->kill(this);
}

QPair<int, int> Tier::pointChangeEstimate(const QString &player, const QString &foe)
{
    MemberRating p = exists(player) ? member(player) : MemberRating(player);
    MemberRating f = exists(foe) ? member(foe) : MemberRating(foe);

    return p.pointChangeEstimate(f.rating);
}

LoadInsertThread<MemberRating> * Tier::getThread()
{
    return boss->getThread();
}

int Tier::getMode() const
{
    return mode;
}

bool Tier::allowGen(Pokemon::gen gen) const
{
    if (this->m_gen == 0)
        return true;
    return this->m_gen == gen;
}

int Tier::getClauses() const
{
    return clauses;
}

int Tier::getMaxLevel() const
{
    return maxLevel;
}

Tier *Tier::dataClone() const
{
    Tier *ret = new Tier();
    Tier &t = *ret;

    t.banPokes = banPokes;
//    t.bannedSets = bannedSets;
//    t.restrictedSets = restrictedSets;
    t.maxRestrictedPokes = maxRestrictedPokes;
    t.numberOfPokemons = numberOfPokemons;
    t.maxLevel = maxLevel;
    t.m_gen = m_gen;
    t.banParentS = banParentS;
    t.bannedItems = bannedItems;
    t.bannedMoves = bannedMoves;
    t.bannedPokes = bannedPokes;
    t.restrictedPokes = restrictedPokes;
    t.mode = mode;
    t.displayOrder = displayOrder;
    t.clauses = clauses;

    t.m_name = m_name;

    return ret;
}

void Tier::processDailyRun()
{
    Server::print(QString("Running Daily Run for tier %1").arg(name()));

    clock_t t = clock();

    int count = 0;
    int min_bonus_time = -TierMachine::obj()->alt_expiration * 3600 * 24 * 30;

    for (auto it = ratings.begin(); it != ratings.end(); ) {
        auto &m = it->second;

        if (m.bonus_time < min_bonus_time) {
            m.name[0] = ':';

            in->seek(m.filePos);
            in->write(m.toString().toUtf8());

            it = ratings.erase(it);
        } else {
            m.calculateDisplayedRating();

            in->seek(m.filePos);
            in->write(m.toString().toUtf8());

            ++it;
        }
    }

    Server::print(QString("%1 alts removed from the ladder.").arg(count));

    /* Regenerate rankings & all */
    loadFromFile();

    t = clock() - t;

    Server::print(QString::number(float(t)/CLOCKS_PER_SEC) + " secs");
}
