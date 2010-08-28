#include <QtXml>
#include <cmath>
#include <ctime>
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
    return name + "%" + QString::number(matches) + "%" +
            QString::number(rating) + "\n";
}

void MemberRating::changeRating(int opponent_rating, bool win)
{
    QPair<int,int> change = pointChangeEstimate(opponent_rating);

    matches += 1;
    rating = rating + (win ? change.first : change.second);
}

QPair<int, int> MemberRating::pointChangeEstimate(int opponent_rating)
{
    int n = matches;

    int kfactor;
    if (n <= 5) {
        static const int kfactors[] = {200, 100, 80, 70, 60, 50};
        kfactor = kfactors[n];
    } else {
        kfactor = 32;
    }
    double myesp = 1/(1+ pow(10., (float(opponent_rating)-rating)/400));

    return QPair<int,int>(int((1. - myesp)*kfactor),int((0. - myesp)*kfactor));
}

void Tier::changeName(const QString &name)
{
    this->m_name = name;
    this->sql_table = "tier_" + ::slug(name);
}

QString Tier::name() const
{
    return m_name;
}

void Tier::changeId(int id)
{
    m_id = id;
}

int Tier::make_query_number(int type)
{
    return (type << 16) + id();
}

void Tier::loadFromFile()
{
    QSqlQuery query;

    query.setForwardOnly(true);

    query.exec(QString("select * from %1 limit 1").arg(sql_table));

    if (!query.next()) {
        if (SQLCreator::databaseType == SQLCreator::PostGreSQL) {
            /* The only way to have an auto increment field with PostGreSQL is to my knowledge using the serial type */
            query.exec(QString("create table %1 (id serial, name varchar(20), rating int, matches int, primary key(id))").arg(sql_table));
        } else if (SQLCreator::databaseType == SQLCreator::MySQL){
            query.exec(QString("create table %1 (id integer auto_increment, name varchar(20) unique, rating int, matches int, primary key(id))").arg(sql_table));
        } else if (SQLCreator::databaseType == SQLCreator::SQLite){
            /* The only way to have an auto increment field with SQLite is to my knowledge having a 'integer primary key' field -- that exact quote */
            query.exec(QString("create table %1 (id integer primary key autoincrement, name varchar(20) unique, rating int, matches int)").arg(sql_table));
        } else {
            throw QString("Using a not supported database");
        }

        query.exec(QString("create index tiername_index on %1 (name)").arg(sql_table));
        query.exec(QString("create index tierrating_index on %1 (rating)").arg(sql_table));

        Server::print(QString("Importing old database for tier %1 to table %2").arg(name(), sql_table));

        QFile in("tier_" + name() + ".txt");
        in.open(QIODevice::ReadOnly);

        QStringList members = QString::fromUtf8(in.readAll()).split('\n');

        clock_t t = clock();

        query.prepare(QString("insert into %1(name, rating, matches) values (:name, :rating, :matches)").arg(sql_table));

        QSqlDatabase::database().transaction();

        foreach(QString member, members) {
            QString m2 = member.toLower();
            QStringList mmr = m2.split('%');
            if (mmr.size() != 3)
                continue;

            query.bindValue(":name", mmr[0]);
            query.bindValue(":matches", mmr[1].toInt());
            query.bindValue(":rating", mmr[2].toInt());

            query.exec();
        }
        
        QSqlDatabase::database().commit();

        t = clock() - t;

        Server::print(QString::number(float(t)/CLOCKS_PER_SEC) + " secs");
        Server::print(query.lastError().text());
    }
}

QString Tier::toString() const {
    QString ret = name() + "=";

    if (parent.length() > 0) {
        ret += parent + "+";
    }

    if (bannedPokes.count() > 0) {
        foreach(BannedPoke pok, bannedPokes) {
            ret += PokemonInfo::Name(pok.poke) + (pok.item == 0 ? "" : QString("@%1").arg(ItemInfo::Name(pok.item))) + ", ";
        }
        ret.resize(ret.length()-2);
    }

    return ret;
}

int Tier::count()
{
    if (m_count != -1 && time(NULL) - last_count_time < 3600) {
        return m_count;
    } else {
        QSqlQuery q;
        q.setForwardOnly(true);

        q.exec(QString("select count(*) from %1").arg(sql_table));

        q.next();
        last_count_time = time(NULL);
        return (m_count = q.value(0).toInt());
    }
}

void Tier::addBanParent(Tier *t)
{
    if (!t) {
        parent = NULL;
        return;
    }

    Tier *it = *t;

    while (it != NULL) {
        if (it == this) {
            parent = NULL;
            return;
        }

        it = it->parent;
    }

    /* No cyclic tree, so we can assign it */
    parent = it;
}

bool Tier::isBanned(const PokeBattle &p) const {
    if (bannedPokes2.contains(p.num())) {
        QList<BannedPoke> values = bannedPokes2.values(p.num());

        foreach (BannedPoke b, values) {
            if (b.item == 0 || b.item == p.item())
                return true;
        }
    }
    if (parent.length() > 0 && boss != NULL) {
        return boss->tier(parent).isBanned(p);
    } else {
        return false;
    }
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
    int r = rating(name);
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("select count(*) from %1 where (rating>:r1 or (rating=:r2 and name<=:name))").arg(sql_table));
    q.bindValue(":r1", r);
    q.bindValue(":r2", r);
    q.bindValue(":name", name);
    q.exec();

    if (q.next())
        return q.value(0).toInt() + 1;
    else
        return -1;
}

bool Tier::isValid(const TeamBattle &t)  const
{
    for (int i = 0; i< 6; i++)
        if (t.poke(i).num() != 0 && isBanned(t.poke(i)))
            return false;

    return true;
}

void Tier::changeRating(const QString &player, int newRating)
{
    MemberRating m = member(player);
    m.rating = newRating;

    updateMember(m);
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
        return holder.member(name).rating;
    } else {
        return 1000;
    }
}

void Tier::loadMemberInMemory(const QString &name, QObject *o, const char *slot)
{
    QString n2 = name.toLower();

    if (o == NULL) {
        if (holder.isInMemory(n2))
            return;

        QSqlQuery q;
        q.setForwardOnly(true);
        processQuery(&q, n2, GetInfoOnUser, NULL);

        return;
    }

    holder.cleanCache();

    WaitingObject *w = WaitingObjects::getObject();

    /* It is important that this connect is done before the connect to freeObject(),
       because then the user at the signal's reception can use the object at will knowing it's not already
       used by another Player or w/e */
    QObject::connect(w, SIGNAL(waitFinished()), o, slot);
    QObject::connect(w, SIGNAL(waitFinished()), WaitingObjects::getInstance(), SLOT(freeObject()));

    if (holder.isInMemory(n2)) {
        w->emitSignal();
    }
    else {
        LoadThread *t = getThread();

        t->pushQuery(n2, w, make_query_number(GetInfoOnUser));
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

    LoadThread *t = getThread();

    t->pushQuery(data, w, make_query_number(GetRankings));
}

void Tier::exportDatabase() const
{
    QFile out(QString("tier_%1.txt").arg(name()));

    out .open(QIODevice::WriteOnly);

    QSqlQuery q;
    q.setForwardOnly(true);

    q.exec(QString("select name, matches, rating from %1 order by name asc").arg(sql_table));

    while (q.next()) {
        MemberRating m(q.value(0).toString(), q.value(1).toInt(), q.value(2).toInt());
        out.write(m.toString().toUtf8());
    }

    Server::print(QString("Database of tier %1 exported!").arg(name()));
}

/* Precondition: name is in lowercase */
void Tier::processQuery(QSqlQuery *q, const QVariant &name, int type, WaitingObject *w)
{
    if (type == GetInfoOnUser) {
        q->prepare(QString("select matches, rating from %1 where name=? limit 1").arg(sql_table));
        q->addBindValue(name);
        q->exec();
        if (!q->next()) {
            holder.addNonExistant(name.toString());
        } else {
            MemberRating m(name.toString(), q->value(0).toInt(), q->value(1).toInt());
            holder.addMemberInMemory(m);
        }
        q->finish();
    } else if (type == GetRankings) {
        int p;

        if (name.type() == QVariant::String) {
            int r = ranking(name.toString());
            p = (r-1)/TierMachine::playersByPage + 1;
        }
        else {
            p = name.toInt();
        }

        if (SQLCreator::databaseType == SQLCreator::PostGreSQL)
            q->prepare(QString("select name, rating from %1 order by rating desc, name asc offset ? limit ?").arg(sql_table));
        else
            q->prepare(QString("select name, rating from %1 order by rating desc, name asc limit ?, ?").arg(sql_table));

        q->addBindValue((p-1)*TierMachine::playersByPage);
        q->addBindValue(TierMachine::playersByPage);

        QVector<QPair<QString, int> > results;
        results.reserve(TierMachine::playersByPage);

        q->exec();
        while (q->next()) {
            results.push_back(QPair<QString, int>(q->value(0).toString(), q->value(1).toInt()));
        }
        q->finish();
        w->data["rankingpage"] = p;
        w->data["tier"] = this->name();
        w->data["rankingdata"] = QVariant::fromValue(results);
    }
}

void Tier::insertMember(QSqlQuery *q, void *data, int update)
{
    MemberRating *m = (MemberRating*) data;

    if (update)
        q->prepare(QString("update %1 set matches=:matches, rating=:rating where name=:name").arg(sql_table));
    else
        q->prepare(QString("insert into %1(name, matches, rating) values(:name, :matches, :rating)").arg(sql_table));

    q->bindValue(":name", m->name);
    q->bindValue(":matches", m->matches);
    q->bindValue(":rating", m->rating);

    q->exec();
    q->finish();
}

void Tier::updateMember(const MemberRating &m, bool add)
{
    holder.addMemberInMemory(m);

    if (add) {
        m_count += 1;
    }
    updateMemberInDatabase(m, add);
}

void Tier::updateMemberInDatabase(const MemberRating &m, bool add)
{
    boss->ithread->pushMember(m, make_query_number(int(!add)));
}

void Tier::loadFromXml(const QDomElement &elem)
{
    banPokes = elem.attribute("banMode", "ban") == "ban";
    banParentS = elem.attribute("banParent");
    parent = NULL;
    changeName(elem.attribute("name"));
    gen = elem.attribute("gen", "4").toInt();
    maxLevel = elem.attribute("maxLevel", "100").toInt();
    numberOfPokemons = elem.attribute("numberOfPokemons", "6").toInt();
    maxRestrictedPokes = elem.attribute("numberOfRestricted", "1").toInt();
    doubles = elem.attribute("doubles", "0").toInt();

    clauses = 0;
    bannedPokes.clear();
    bannedPokes2.clear();
    restrictedPokes.clear();
    restrictedPokes2.clear();

    bannedItems.clear();
    bannedMoves.clear();

    m_count = -1;
    last_count_time = 0;

    QDomNodeList moves = elem.elementsByTagName("clausedMove");
    foreach(QDomNode n, moves) {
        if (MoveInfo::Exists(n.nodeValue(), gen))
            bannedMoves.insert(MoveInfo::Number(n.nodeValue()));
    }
    QDomNodeList items = elem.elementsByTagName("clausedMove");
    foreach(QDomNode n, items) {
        if (ItemInfo::Exists(n.nodeValue(), gen))
            bannedItem.insert(ItemInfo::Number(n.nodeValue()));
    }
    QDomNodeList clausesL = elem.elementsByTagName("clause");
    foreach(QDomNode n, clausesL) {
        int index = ChallengeInfo::clause(n.nodeValue());

        if (index > -1)  {
            clauses |= 1 << index;
        }
    }

    QDomElement pokElem = elem.firstChildElement("clausedPokemon");
    if (!pokElem.isNull()) {
        pokElem = elem.firstChildElement("pokemon");

        while (!pokElem.isNull()) {
            BannedPoke p;
            p.loadFromXml(elem);

            bannedPokes.push_back(p);
            bannedPokes2.insert(p.poke, p);

            pokElem = elem.nextSiblingElement("pokemon");
        }
    }

    pokElem = elem.firstChildElement("restrictedPokemon");
    if (!pokElem.isNull()) {
        pokElem = elem.firstChildElement("pokemon");

        while (!pokElem.isNull()) {
            BannedPoke p;
            p.loadFromXml(elem);

            restrictedPokes.push_back(p);
            restrictedPokes2.insert(p.poke, p);

            pokElem = elem.nextSiblingElement("pokemon");
        }
    }
}

void BannedPoke::loadFromXml(const QDomElement &elem) {
    poke = PokemonInfo::Number(elem.attribute("name"));
    item = ItemInfo::Number("item");
    QDomNodeList moves = elem.elementsByTagName("move");
    foreach(QDomNode n, moves) {
        if (MoveInfo::Exists(n.nodeValue(), gen))
            moves.insert(MoveInfo::Number(n.nodeValue()));
    }
}

Tier::Tier(TierMachine *boss) : boss(boss), m_count(-1), last_count_time(0), holder(1000) {

}

QPair<int, int> Tier::pointChangeEstimate(const QString &player, const QString &foe)
{
    MemberRating p = exists(player) ? member(player) : MemberRating(player);
    MemberRating f = exists(foe) ? member(foe) : MemberRating(foe);

    return p.pointChangeEstimate(f.rating);
}

LoadThread * Tier::getThread()
{
    return boss->getThread();
}
