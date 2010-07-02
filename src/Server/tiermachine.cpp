#include "tiermachine.h"
#include "tier.h"

TierMachine* TierMachine::inst;

void TierMachine::init()
{
    inst = new TierMachine();
}

TierMachine::TierMachine()
{
    threads = new LoadThread[loadThreadCount];

    for (int i = 0; i < loadThreadCount; i++) {
        connect(&threads[i], SIGNAL(processQuery (QSqlQuery *, QString, int)), this, SLOT(processQuery(QSqlQuery*,QString,int)), Qt::DirectConnection);
        threads[i].start();
    }

    ithread = new InsertThread<MemberRating>();
    connect(ithread, SIGNAL(processMember(QSqlQuery*,void*,int)), this, SLOT(insertMember(QSqlQuery*,void*,int)), Qt::DirectConnection);

    ithread->start();

    QFile in("tiers.txt");
    in.open(QIODevice::ReadOnly);
    fromString(QString::fromUtf8(in.readAll()));
}

void TierMachine::processQuery(QSqlQuery *q, const QString &member, int queryNo)
{
    int tierno = queryNo % (1 << 16);

    if (m_tiers.length() > tierno) {
        m_tiers[tierno]->processQuery(q, member, queryNo >> 16);
    } else {
        qDebug() << "Critical! invalid load tier membe query, tier requested: " << tierno << "query no: " << queryNo;
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
    QFile out("tiers.txt");
    out.open(QIODevice::WriteOnly);
    out.write(toString().toUtf8());
}

void TierMachine::clear()
{
    while (m_tiers.size() > 0) {
        delete m_tiers.takeLast();
    }

    m_tierNames.clear();
}

void TierMachine::fromString(const QString &s)
{
    clear();

    QStringList candidates = s.split('\n', QString::SkipEmptyParts);

    if (candidates.empty()) {
        candidates.push_back("All=");
    }


    foreach(QString candidate, candidates) {
        m_tiers.push_back(new Tier(this));
        m_tiers.back()->fromString(candidate);
        if (m_tierNames.contains(m_tiers.back()->name())) {
            delete m_tiers.takeLast();
        } else {
            m_tierNames.push_back(m_tiers.back()->name());
        }
    }

    /* Now, we just check there isn't any cyclic inheritance tree */
    for(int i = 0; i < m_tiers.length(); i++) {
        QSet<QString> family;
        Tier *t = m_tiers[i];
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
        m_tiers[i]->changeId(i);
        m_tiers[i]->loadFromFile();
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
        res += m_tiers[i]->toString() + "\n";
    }
    if (res.length() > 0) {
        res.resize(res.size()-1);
    }

    return res;
}

void TierMachine::loadMemberInMemory(const QString &name, const QString &tier, QObject *o, const char *slot)
{
    this->tier(tier).loadMemberInMemory(name, o, slot);
}


Tier &TierMachine::tier(const QString &name)
{
    for(int i = 0; i < m_tierNames.length(); i++) {
        if (m_tierNames[i].compare(name, Qt::CaseInsensitive) == 0) {
            return *m_tiers[i];
        }
    }
    return *m_tiers[0];
}

const Tier &TierMachine::tier(const QString &name) const
{
    for(int i = 0; i < m_tierNames.length(); i++) {
        if (m_tierNames[i].compare(name, Qt::CaseInsensitive) == 0) {
            return *m_tiers[i];
        }
    }
    return *m_tiers[0];
}

bool TierMachine::exists(const QString &name) const
{
    for(int i = 0; i < m_tierNames.length(); i++) {
        if (m_tierNames[i].compare(name, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
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
    (void) tier;
    return 0;
    //return this->tier(tier).members.count();
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

LoadThread *TierMachine::getThread()
{
    /* '%' is a safety thing, in case nextLoadThreadNumber is also accessed in writing and that messes it up, at least it isn't out of bounds now */
    int n = nextLoadThreadNumber % loadThreadCount;
    nextLoadThreadNumber = (nextLoadThreadNumber + 1) % loadThreadCount;
    return threads + n;
}
