#include "tiermachine.h"
#include "tier.h"
#include "loadinsertthread.h"

TierMachine* TierMachine::inst;

void TierMachine::init()
{
    inst = new TierMachine();
}

TierMachine::TierMachine()
{
    loadDecaySettings();

    threads = new LoadThread[loadThreadCount];

    for (int i = 0; i < loadThreadCount; i++) {
        connect(&threads[i], SIGNAL(processQuery (QSqlQuery *, QVariant, int, WaitingObject*)), this, SLOT(processQuery(QSqlQuery*, QVariant, int, WaitingObject *)), Qt::DirectConnection);
        threads[i].start();
    }

    nextLoadThreadNumber = 0;

    ithread = new InsertThread<MemberRating>();
    connect(ithread, SIGNAL(processMember(QSqlQuery*,void*,int)), this, SLOT(insertMember(QSqlQuery*,void*,int)), Qt::DirectConnection);

    ithread->start();

    load();
}

void TierMachine::loadDecaySettings()
{
    QSettings s("config", QSettings::IniFormat);

    alt_expiration = std::max(s.value("ladder_months_expiration", 3).toInt(), 1);
    hours_per_period = std::max(s.value("ladder_period_duration", 24).toInt(), 1);
    percent_per_period = std::max(s.value("ladder_percent_per_period", 5).toInt(), 1);
    max_saved_periods = std::max(s.value("ladder_bonus_time", 3).toInt(), 1);
    max_percent_decay = std::min(s.value("ladder_max_decay", 50).toInt(), 100);
}

void TierMachine::load()
{
    QFile in("tiers.xml");
    in.open(QIODevice::ReadOnly);
    fromString(QString::fromUtf8(in.readAll()));

    emit tiersChanged();
}

void TierMachine::processQuery(QSqlQuery *q, const QVariant &data, int queryNo, WaitingObject *w)
{
    int tierno = queryNo % (1 << 16);

    if (m_tiers.length() > tierno) {
        m_tiers[tierno]->processQuery(q, data, queryNo >> 16,w);
    } else {
        qDebug() << "Critical! invalid load tier member query, tier requested: " << tierno << "query no: " << queryNo;
        return;
    }
}

void TierMachine::insertMember(QSqlQuery *q, void *m, int queryNo)
{
    int tierno = queryNo % (1 << 16);

    if (m_tiers.length() > tierno) {
        m_tiers[tierno]->insertMember(q, m, queryNo >> 16);
    } else {
        qDebug() << "Critical! invalid insert tier query, tier requested: " << tierno << "query no: " << queryNo;
        return;
    }
}

void TierMachine::save()
{
    QFile out("tiers.xml");
    out.open(QIODevice::WriteOnly);
    out.write(toString().toUtf8());
}

void TierMachine::clear()
{
    m_tierNames.clear();
    m_tierByNames.clear();
}

void TierMachine::fromString(const QString &s)
{
    clear();

    tree.loadFromXml(s, this);

    QList<Tier *> tiers = tree.gatherTiers();
    if (tiers.empty()) {
        Tier *t = new Tier(this, &tree.root);
        t->changeName("All");
        tree.root.subNodes.push_back(t);
        tiers.push_back(t);
    }

    QHash<QString, Tier *> tierNames;

    /* Removing duplicates */
    foreach(Tier *t, tiers) {
        if (tierNames.contains(t->name())) {
            t->kill(); /* Destroys the tier */
            continue;
        }
        tierNames.insert(t->name(), t);
    }

    /* Removing useless categories */
    tree.cleanCategories();
    /* Getting the order right if it wasn't alraedy in the file */
    tree.reorder();

    /* Some duplicates may have been removed, so we gather the tiers again */
    tiers = tree.gatherTiers();

    foreach(Tier *t, tiers) {
        m_tierByNames[t->name()] = t;
        m_tierNames.push_back(t->name());
    }

    /* Doing inheritance trees */
    foreach(Tier *t, tiers) {
        QString banParent = t->banParentS;

        if (tierNames.contains(banParent)) {
            t->addBanParent(tierNames[banParent]);
        }
    }

    /* Then, we open the files and load the ladders for each tier and people */
    for (int i =0; i < tiers.size(); i++) {
        tiers[i]->changeId(i);
        tiers[i]->loadFromFile();
    }

    m_tiers = tiers;

    /* Do tierList . */
    m_tierList = tree.buildTierList();

    emit tiersChanged();
}

QByteArray TierMachine::tierList() const {
    return m_tierList;
}

QString TierMachine::toString() const
{
    return tree.toXml();
}

void TierMachine::loadMemberInMemory(const QString &name, const QString &tier, QObject *o, const char *slot)
{
    this->tier(tier).loadMemberInMemory(name, o, slot);
}

void TierMachine::fetchRankings(const QString &name, const QVariant &data, QObject *o, const char *slot)
{
    this->tier(name).fetchRankings(data, o, slot);
}

Tier &TierMachine::tier(const QString &name)
{
    if (m_tierByNames.contains(name)) {
        return *m_tierByNames[name];
    }

    return *m_tiers[0];
}

const Tier &TierMachine::tier(const QString &name) const
{
    if (m_tierByNames.contains(name)) {
        return *m_tierByNames[name];
    }

    return *m_tiers[0];
}

bool TierMachine::exists(const QString &name) const
{
    return m_tierByNames.contains(name);
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

void TierMachine::exportDatabase() const
{
    for(int i = 0; i < m_tiers.size(); i++) {
        m_tiers[i]->exportDatabase();
    }
}

int TierMachine::rating(const QString &name, const QString &tier)
{
    return this->tier(tier).rating(name);
}

int TierMachine::inner_rating(const QString &name, const QString &tier)
{
    return this->tier(tier).inner_rating(name);
}

int TierMachine::ranking(const QString &name, const QString &tier)
{
    return this->tier(tier).ranking(name);
}

int TierMachine::count(const QString &tier)
{
    if (!exists(tier))
        return 0;

    return this->tier(tier).count();
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
        if (m_tiers[i]->isValid(t)) {
            return m_tierNames[i];
        }
    }
    return m_tierNames[0];
}

bool TierMachine::existsPlayer(const QString &name, const QString &player)
{
   return exists(name) && tier(name).exists(player);
}

LoadThread *TierMachine::getThread()
{
    /* '%' is a safety thing, in case nextLoadThreadNumber is also accessed in writing and that messes it up, at least it isn't out of bounds now */
    int n = nextLoadThreadNumber % loadThreadCount;
    nextLoadThreadNumber = (n + 1) % loadThreadCount;
    return threads + n;
}

TierTree *TierMachine::getDataTree() const
{
    return tree.dataClone();
}

void TierMachine::processDailyRun()
{
    for(int i = 0; i < m_tiers.size(); i++) {
        m_tiers[i]->processDailyRun();
    }
}
