#include <cmath>
#include <ctime>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/battlestructs.h"
#include "tier.h"
#include "security.h"
#include "server.h"

TierMachine* TierMachine::inst;

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
    this->slug = ::slug(name);
}

QString Tier::name() const
{
    return m_name;
}

void Tier::loadFromFile()
{
    QSqlQuery query;

    query.setForwardOnly(true);

    query.exec(QString("select * from %1 limit 1").arg(slug));

    if (!query.next()) {
        if (SQLCreator::databaseType == SQLCreator::PostGreSQL) {
            /* The only way to have an auto increment field with PostGreSQL is to my knowledge using the serial type */
            query.exec(QString("create table %1 (id serial, name varchar(20), rating int, matches int, primary key(id))").arg(slug));
        } else if (SQLCreator::databaseType == SQLCreator::SQLite){
            /* The only way to have an auto increment field with SQLite is to my knowledge having a 'integer primary key' field -- that exact quote */
            query.exec(QString("create table %1 (id integer primary key autoincrement, name varchar(20) unique, rating int, matches int, primary key(id))").arg(slug));
        } else {
            throw QString("Using a not supported database");
        }

        query.exec(QString("create index tiername_index on %1 (name)").arg(slug));
        query.exec(QString("create index tierrating_index on %1 (rating)").arg(slug));

        Server::print(QString("Importing old database for tier %1").arg(name()));

        QFile in("tier_" + name() + ".txt");
        in.open(QIODevice::ReadOnly);

        QStringList members = QString::fromUtf8(in.readAll()).split('\n');

        clock_t t = clock();

        query.prepare(QString("insert into %1(name, rating, matches) values (:name, :rating, :matches)").arg(slug));

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

void TierMachine::init()
{
    inst = new TierMachine();

    QFile in("tiers.txt");
    in.open(QIODevice::ReadOnly);
    inst->fromString(QString::fromUtf8(in.readAll()));
}

void TierMachine::save()
{
    QFile out("tiers.txt");
    out.open(QIODevice::WriteOnly);
    out.write(toString().toUtf8());
}

void TierMachine::fromString(const QString &s)
{
    m_tiers.clear();
    m_tierNames.clear();

    QStringList candidates = s.split('\n', QString::SkipEmptyParts);

    if (candidates.empty()) {
        candidates.push_back("All=");
    }


    foreach(QString candidate, candidates) {
        m_tiers.push_back(Tier(this));
        m_tiers.back().fromString(candidate);
        if (m_tierNames.contains(m_tiers.back().name())) {
            m_tiers.pop_back();
        } else {
            m_tierNames.push_back(m_tiers.back().name());
        }
    }

    /* Now, we just check there isn't any cyclic inheritance tree */
    for(int i = 0; i < m_tiers.length(); i++) {
        QSet<QString> family;
        Tier *t = & m_tiers[i];
        family.insert(t->name());
        while (t->parent.length() > 0) {
            if (family.contains(t->parent)) {
                tier(t->name()).parent.clear();
                break;
            }
            family.insert(t->parent);
            t = &tier(t->parent);
        }
    }

    /* Then, we open the files and load the ladders for each tier and people */
    for (int i =0; i < m_tiers.size(); i++) {
        m_tiers[i].loadFromFile();
    }

    /* And we change the tierList variable too! */
    tierList().clear();
    foreach (QString tier, m_tierNames)
    {
        tierList() += tier + "\n";
    }
    if (tierList().length() > 0) {
        tierList().resize(tierList().size()-1);
    }
}

QString TierMachine::toString() const
{
    QString res = "";

    for(int i = 0; i < m_tiers.size(); i++) {
        res += m_tiers[i].toString() + "\n";
    }
    if (res.length() > 0) {
        res.resize(res.size()-1);
    }

    return res;
}


Tier &TierMachine::tier(const QString &name)
{
    for(int i = 0; i < m_tierNames.length(); i++) {
        if (m_tierNames[i].compare(name, Qt::CaseInsensitive) == 0) {
            return m_tiers[i];
        }
    }
    return m_tiers[0];
}

const Tier &TierMachine::tier(const QString &name) const
{
    for(int i = 0; i < m_tierNames.length(); i++) {
        if (m_tierNames[i].compare(name, Qt::CaseInsensitive) == 0) {
            return m_tiers[i];
        }
    }
    return m_tiers[0];
}

bool TierMachine::isValid(const TeamBattle &t, QString tier) const
{
    if (!exists(tier))
        return false;

    return this->tier(tier).isValid(t);
}

bool TierMachine::isBanned(const PokeBattle &pok, const QString & tier) const
{
    return this->tier(tier).isBanned(pok);
}

TierMachine *TierMachine::obj()
{
    return inst;
}

const QStringList & TierMachine::tierNames() const
{
    return m_tierNames;
}

int TierMachine::rating(const QString &name, const QString &tier)
{
    return this->tier(tier).rating(name);
}

int TierMachine::ranking(const QString &name, const QString &tier)
{
    return this->tier(tier).ranking(name);
}

int TierMachine::count(const QString &tier)
{
    return this->tier(tier).members.count();
}

void TierMachine::changeRating(const QString &winner, const QString &loser, const QString &tier)
{
    return this->tier(tier).changeRating(winner, loser);
}

void TierMachine::changeRating(const QString &player, const QString &tier, int newRating)
{
    return this->tier(tier).changeRating(player, newRating);
}

QPair<int, int> TierMachine::pointChangeEstimate(const QString &player, const QString &foe, const QString &tier)
{
    return this->tier(tier).pointChangeEstimate(player, foe);
}

QString TierMachine::findTier(const TeamBattle &t) const
{
    for (int i = m_tiers.size()-1; i >= 0; i--) {
        if (m_tiers[i].isValid(t)) {
            return m_tierNames[i];
        }
    }
    return m_tierNames[0];
}

TierWindow::TierWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose,true);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(m_editWindow = new QPlainTextEdit(),0,0,1,2);
    QPushButton *ok;
    layout->addWidget(ok = new QPushButton(tr("&Done")),1,1);

    m_editWindow->setPlainText(TierMachine::obj()->toString());

    connect(ok, SIGNAL(clicked()), SLOT(done()));
    connect(ok, SIGNAL(clicked()), SLOT(close()));
}

void TierWindow::done()
{
    TierMachine::obj()->fromString(m_editWindow->toPlainText());
    TierMachine::obj()->save();

    emit tiersChanged();
}
