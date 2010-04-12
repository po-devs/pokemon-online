#include <cmath>
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/battlestructs.h"
#include "tier.h"
#include "security.h"

TierMachine* TierMachine::inst;

QString MemberRating::toString() const
{
    return name() + "%" + QString::number(matches()).rightJustified(5,'0',true) + "%" +
            QString::number(rating()).rightJustified(5,'0',true);
}

void MemberRating::changeRating(int opponent_rating, bool win)
{
    int n = matches();
    int newrating;

    int kfactor;
    if (n <= 5) {
        static const int kfactors[] = {200, 150, 100, 80, 65, 50};
        kfactor = kfactors[n];
    } else {
        kfactor = 32;
    }
    double myesp = 1/(1+ pow(10., (float(opponent_rating)-rating())/400));
    double result = win;

    newrating = rating() + (result - myesp)*kfactor;

    rating() = newrating;
    if (matches() <=9999)
        matches() += 1;
}

void Tier::loadFromFile()
{
    ratings.clear();

    delete in;
    in = new QFile("tier_" + name + ".txt");
    in->open(QIODevice::ReadOnly);

    QStringList members = QString::fromUtf8(in->readAll()).split('\n');
    foreach(QString member, members) {
        QString m2 = member.toLower();
        QStringList mmr = m2.split('%');
        if (mmr.size() != 3)
            continue;
        if (!SecurityManager::exist(mmr[0]))
            continue;
        if (ratings.contains(mmr[0]))
            continue;
        MemberRating m;
        m.name() = mmr[0];
        m.matches() = mmr[1].toInt();
        m.rating() = mmr[2].toInt();
        m.node() = rankings.insert(m.rating(), m.name());
        ratings.insert(m.name(), m);
    }

    in->close();
    in->open(QIODevice::WriteOnly);

    int pos = 0;
    QHash<QString,MemberRating>::iterator it;
    for (it = ratings.begin(); it != ratings.end(); ++it) {
        it->filePos() = pos;
        in->write(it->toString().toUtf8());
        in->putChar('\n');
        pos = in->pos();
    }
    lastFilePos = pos;
    in->close();
    in->open(QIODevice::ReadWrite);
}

QString Tier::toString() const {
    QString ret = name + "=";

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
    name.clear();
    parent.clear();
    bannedPokes.clear();
    bannedPokes2.clear();

    QStringList s2 = s.split('=');
    if (s2.size() > 1) {
        name = s2[0];
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
        if (isBanned(t.poke(i)))
            return false;

    return true;
}

void Tier::changeRating(const QString &w, const QString &l)
{
    QString w2 = w.toLower();
    QString l2 = l.toLower();
    if (!ratings.contains(w2)) {
        MemberRating m;
        m.name() = w2;
        m.filePos() = lastFilePos;
        m.node() = rankings.insert(m.rating(), m.name());
        in->seek(lastFilePos);
        in->write(m.toString().toUtf8());
        in->putChar('\n');
        lastFilePos = in->pos();
        ratings[w2] = m;
    }
    if (!ratings.contains(l2)) {
        MemberRating m;
        m.name() = l2;
        m.filePos() = lastFilePos;
        m.node() = rankings.insert(m.rating(), m.name());
        in->seek(lastFilePos);
        in->write(m.toString().toUtf8());
        in->putChar('\n');
        lastFilePos = in->pos();
        ratings[l2] = m;
    }
    int oldw2 = ratings[w2].rating();
    ratings[w2].changeRating(ratings[l2].rating(), true);
    ratings[l2].changeRating(oldw2, false);
    ratings[w2].node() = rankings.changeKey(ratings[w2].node().node(), ratings[w2].rating());
    ratings[l2].node() = rankings.changeKey(ratings[l2].node().node(), ratings[l2].rating());
    in->seek(ratings[w2].filePos());
    in->write(ratings[w2].toString().toUtf8());
    in->seek(ratings[l2].filePos());
    in->write(ratings[l2].toString().toUtf8());
    in->flush();
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
        if (m_tierNames.contains(m_tiers.back().name)) {
            m_tiers.pop_back();
        } else {
            m_tierNames.push_back(m_tiers.back().name);
        }
    }

    /* Now, we just check there isn't any cyclic inheritance tree */
    for(int i = 0; i < m_tiers.length(); i++) {
        QSet<QString> family;
        Tier *t = & m_tiers[i];
        family.insert(t->name);
        while (t->parent.length() > 0) {
            if (family.contains(t->parent)) {
                tier(t->name).parent.clear();
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

const QList<QString> & TierMachine::tierNames() const
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
    return this->tier(tier).rankings.count();
}

void TierMachine::changeRating(const QString &winner, const QString &loser, const QString &tier)
{
    return this->tier(tier).changeRating(winner, loser);
}

const RankingTree<QString> *TierMachine::getRankingTree(const QString &tier)
{
    return &this->tier(tier).rankings;
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
