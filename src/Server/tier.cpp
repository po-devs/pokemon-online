#include <cmath>
#include <ctime>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/battlestructs.h"
#include "tier.h"
#include "tiermachine.h"
#include "security.h"
#include "server.h"

QString MemberRating::toString() const
{
    return name() + "%" + QString::number(matches()).rightJustified(5,'0',true) + "%" +
            QString::number(rating()).rightJustified(5,'0',true);
}

void MemberRating::changeRating(int opponent_rating, bool win)
{
    QPair<int,int> change = pointChangeEstimate(opponent_rating);

    rating() = rating() + (win ? change.first : change.second);

    if (matches() <=9999)
        matches() += 1;
}

QPair<int, int> MemberRating::pointChangeEstimate(int opponent_rating)
{
    int n = matches();

    int kfactor;
    if (n <= 5) {
        static const int kfactors[] = {200, 100, 80, 70, 60, 50};
        kfactor = kfactors[n];
    } else {
        kfactor = 32;
    }
    double myesp = 1/(1+ pow(10., (float(opponent_rating)-rating())/400));

    return QPair<int,int>((1. - myesp)*kfactor,(0. - myesp)*kfactor);
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
            query.exec(QString("create table %1 (id integer primary key autoincrement, name varchar(20) unique, rating int, matches int, primary key(id))").arg(sql_table));
        } else {
            throw QString("Using a not supported database");
        }

        query.exec(QString("create index tiername_index on %1 (name)").arg(sql_table));
        query.exec(QString("create index tierrating_index on %1 (rating)").arg(sql_table));

        Server::print(QString("Importing old database for tier %1").arg(name()));

        QFile in("tier_" + name() + ".txt");
        in.open(QIODevice::ReadOnly);

        QStringList members = QString::fromUtf8(in.readAll()).split('\n');

        clock_t t = clock();

        query.prepare(QString("insert into %1(name, rating, matches) values (:name, :rating, :matches)").arg(sql_table));

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

void Tier::fromString(const QString &s) {
    changeName("");
    parent.clear();
    bannedPokes.clear();
    bannedPokes2.clear();

    QStringList s2 = s.split('=');
    if (s2.size() > 1) {
        changeName(s2[0]);
        QStringList rest = s2[1].split('+');
        QStringList pokes;

        if (rest.length() > 1) {
            parent = rest[0];
            pokes = rest[1].split(',');
        } else {
            pokes = rest[0].split(',');
        }

         foreach(QString poke, pokes) {
            BannedPoke pok = BannedPoke(PokemonInfo::Number(poke.section('@',0,0).trimmed()), ItemInfo::Number(poke.section('@',1,1).trimmed()));
            if (pok.poke != 0) {
                bannedPokes.push_back(pok);
                bannedPokes2.insert(pok.poke, pok);
            }
        }
    }
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

bool Tier::isValid(const TeamBattle &t)  const
{
    for (int i = 0; i< 6; i++)
        if (t.poke(i).num() != 0 && isBanned(t.poke(i)))
            return false;

    return true;
}

void Tier::changeRating(const QString &player, int newRating)
{
    QString w2 = player.toLower();
    if (!members.contains(w2)) {
        MemberRating m;
        m.name() = w2;
        members[w2] = m;
    }
    members[w2].rating() = newRating;
}

void Tier::changeRating(const QString &w, const QString &l)
{
    QString w2 = w.toLower();
    QString l2 = l.toLower();

    /* Not really necessary, as pointChangeEstimate should always be called
       at the beginning of the battle, but meh maybe it's not a battle */
    if (!members.contains(w2)) {
        MemberRating m;
        m.name() = w2;
        members[w2] = m;
    }
    if (!members.contains(l2)) {
        MemberRating m;
        m.name() = l2;
        members[l2] = m;
    }

    int oldw2 = members[w2].rating();
    members[w2].changeRating(members[l2].rating(), true);
    members[l2].changeRating(oldw2, false);
}

QPair<int, int> Tier::pointChangeEstimate(const QString &player, const QString &foe)
{
    QString w2 = player.toLower();
    QString l2 = foe.toLower();

    if (!members.contains(w2)) {
        MemberRating m;
        m.name() = w2;
        members[w2] = m;
    }
    if (!members.contains(l2)) {
        MemberRating m;
        m.name() = l2;
        members[l2] = m;
    }

    return members[w2].pointChangeEstimate(members[l2].rating());
}
