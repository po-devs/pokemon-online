namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include <cstdio>
#include "usagestats.h"
#include "../PokemonInfo/battlestructs.h"

BattleServerPlugin * createBattleServerPlugin() {
    return new PokemonOnlineStatsPlugin();
}

/*************************/
/*************************/

TierRank::TierRank(QString tier) : tier(tier), m(QMutex::Recursive)
{
    timer = time(NULL);

    QFile f("usage_stats/raw/"+tier+"/ranks.rnk");
    f.open(QIODevice::ReadOnly);

    QByteArray content = f.readAll();

    if (content.length() > 0) {
        DataStream d(&content, QIODevice::ReadOnly);

        d >> uses;

        for (int i = 0; i < uses.length(); i++) {
            positions[uses[i].first] = i;
        }
    }
}

TierRank::~TierRank()
{
    writeContents();
}

void TierRank::addUsage(const Pokemon::uniqueId &pokemon)
{
    QMutexLocker l(&m);

    if (!positions.contains(pokemon)) {
        if (!PokemonInfo::IsAesthetic(pokemon)) {
            positions.insert(pokemon, uses.size());
            uses.push_back(QPair<Pokemon::uniqueId, int>(pokemon, 1));
        } else {
            TierRank::addUsage(PokemonInfo::OriginalForme(pokemon));
        }
    } else {
        int pos = positions[pokemon];
        uses[pos].second += 1;

        while (pos > 0 && uses[pos-1].second < uses[pos].second) {
            uses.swap(pos, pos-1);
            positions[uses[pos-1].first] = pos - 1;
            positions[uses[pos].first] = pos;
            pos--;
        }

        /* Saves every 5 minutes */
        if (time(NULL) - timer > 5*60) {
            writeContents();
        }
    }
}

void TierRank::writeContents()
{
    QMutexLocker l(&m);

    QFile f("usage_stats/raw/"+tier+"/ranks.rnk");
    f.open(QIODevice::WriteOnly);
    QByteArray data;
    DataStream d(&data, QIODevice::WriteOnly);
    d << uses;

    f.write(data);
    f.close();

    timer = time(NULL);
}

/*************************/
/*************************/

PokemonOnlineStatsPlugin::PokemonOnlineStatsPlugin()
{
    QDir d;
    d.mkdir("usage_stats");
    d.mkdir("usage_stats/raw");
    d.mkdir("usage_stats/formatted");
}

PokemonOnlineStatsPlugin::~PokemonOnlineStatsPlugin()
{
    foreach(TierRank *t, tierRanks) {
        delete t;
    }

    tierRanks.clear();
}

QString PokemonOnlineStatsPlugin::pluginName() const
{
    return "Usage Statistics";
}

BattlePlugin * PokemonOnlineStatsPlugin::getBattlePlugin(BattleInterface*b)
{
    if (b->tier().length() == 0)
        return new PokemonOnlineStatsBattlePlugin(this, NULL);
    if (!tierRanks.contains(b->tier())) {
        tierRanks.insert(b->tier(), new TierRank(b->tier()));
    }
    return new PokemonOnlineStatsBattlePlugin(this, tierRanks[b->tier()]);
}

bool PokemonOnlineStatsPlugin::hasConfigurationWidget() const {
    return false;
}

/*************************/
/*************************/

PokemonOnlineStatsBattlePlugin::PokemonOnlineStatsBattlePlugin(PokemonOnlineStatsPlugin *master, TierRank *t) : master(master), ranked_ptr(t)
{
    master->refCounter.ref();
}

PokemonOnlineStatsBattlePlugin::~PokemonOnlineStatsBattlePlugin()
{
    //qDebug() << "Deleting stats plugin " << this;
    master->refCounter.deref();
    //qDebug() << "Deleted stats plugin " << this;
}

QHash<QString, BattlePlugin::Hook> PokemonOnlineStatsBattlePlugin::getHooks()
{
    QHash<QString, Hook> ret;

    ret.insert("battleStarting(BattleInterface&)", (Hook)(&PokemonOnlineStatsBattlePlugin::battleStarting));

    return ret;
}

int PokemonOnlineStatsBattlePlugin::battleStarting(BattleInterface &b)
{
    //qDebug() << "Battle Starting Stats " << this;
    /* We only keep track of battles between players of the same tier
       and not CC battles */
    if (b.clauses() & ChallengeInfo::ChallengeCup) {
        qDebug() << "CC " << this;
        return -1;
    }

    QString tier = b.tier();

    if (tier.length() == 0) {
        tier = QString("Mixed Tiers Gen %1").arg(b.gen().num);
    }


    QString dir = QString("usage_stats/raw/%1/").arg(tier);
    QDir d;
    d.mkdir(dir);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            bool lead = false;

            if (b.mode() == ChallengeInfo::Singles) {
                lead = j == 0;
            } else if (b.mode() == ChallengeInfo::Doubles) {
                lead = j <= 1;
            } else if (b.mode() == ChallengeInfo::Triples) {
                lead = j <= 2;
            }

            savePokemon(b.poke(i,j), lead, dir);

            if (b.rating(i) > 1000 && ranked_ptr) {
                ranked_ptr->addUsage(b.poke(i,j).num());
            }
        }
    }

    //qDebug() << "End Battle Starting Stats " << this;

    return -1; /* Means the plugin is done */
}


inline int norm(int ev) {
    return (ev/4)*4;
}

/*
 * From here onward C functions are used in order to optimize
 * speed for that crucial problem, because we access files a lot.
 */

QByteArray PokemonOnlineStatsBattlePlugin::data(const PokeBattle &p) const {
    QByteArray ret;
    ret.resize(bufsize);

    /* Constructs bufsize bytes of raw data representing the pokemon */
    qint32 *a = (qint32*)ret.data();
    
    a[0] = (p.num().toPokeRef());
    a[1] = (p.item());
    a[2] = (p.ability() << 16) + (p.gender() << 8) + p.level();
    a[3] = (p.nature() << 24) + (norm(p.evs()[0]) << 16) + (norm(p.evs()[1]) << 8) + norm(p.evs()[2]);
    a[4] = (norm(p.evs()[3]) << 16) + (norm(p.evs()[4]) << 8) + norm(p.evs()[5]);
    a[5] = (p.dvs()[0] << 25) + (p.dvs()[1] << 20) + (p.dvs()[2] << 15) + (p.dvs()[3] << 10) + (p.dvs()[4] << 5) + p.dvs()[5];

    /* Here the moves are sorted because we don't want to have different
       movesets when moves are in a different order */
    quint16 *moves = (quint16*) (a + 6);
    moves[0] = p.move(0).num();
    moves[1] = p.move(1).num();
    moves[2] = p.move(2).num();
    moves[3] = p.move(3).num();
    qSort(&moves[0], &moves[4]);

    return ret;
}

/* Basically, we take the first 3 letters of the hash of the pokemon's raw data,
   and we open that file. We then put the pokemon if it isn't already in, and
   we also open another file with those two letters + _count, in which we write
   two numbers: the usage and the lead usage of the set.

   The reason for using low level functions is because this is a pretty critical
   section in my opinion for big servers, and C++ file management systems are
   pretty slow.
*/
void PokemonOnlineStatsBattlePlugin::savePokemon(const PokeBattle &p, bool lead, const QString &d)
{
    QByteArray data = this->data(p);

    QByteArray file = (d + QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex().left(3) + ".stat").toUtf8();

    FILE *raw_f = fopen(file.data(), "r+b");

    if (!raw_f) {
        raw_f = fopen(file.data(), "w+b");
    }

    if (!raw_f) {
        QDir dir;
        dir.mkpath(d);

        raw_f = fopen(file.data(), "r+b");

        if (!raw_f) {
            raw_f = fopen(file.data(), "w+b");

            if (!raw_f) {
                qDebug() << "Usage stats error: impossible to open file " << file.data();
                return;
            }
        }
    }

    char buffer[bufsize];

    /* We look for the pokemon in the file. Read 28 bytes, compare, skip 8 bytes, read 28 bytes, ... */
    while (!feof(raw_f) && fread(buffer, sizeof(char), bufsize/sizeof(char), raw_f) == signed(bufsize) ) {
        if (memcmp(data.data(), buffer, bufsize) == 0) {
            break;
        }
        /* Not being interested by the count, so we seek forward */
        fseek(raw_f, 2*sizeof(qint32), SEEK_CUR);
    }

    qint32 usage(0), leadusage(0);

    /* The pokemon was never used before? */
    if (feof(raw_f)) {
        fseek(raw_f, 0, SEEK_END);
        fwrite(data.data(), sizeof(char), bufsize/sizeof(char), raw_f);
    } else {
        /* The pokemon was used before so there's already a count,
            so we read the count and then move back */
        fread(&usage, sizeof(qint32), 1, raw_f);
        fread(&leadusage, sizeof(qint32), 1, raw_f);
        /* Seek back to the place where the count is... */
        fseek(raw_f, -2*sizeof(qint32), SEEK_CUR);
    }

    usage += 1;
    leadusage += int(lead);

    /* Write the final counts */
    fwrite(&usage, sizeof(qint32), 1, raw_f);
    fwrite(&leadusage, sizeof(qint32), 1, raw_f);

    fclose(raw_f);
}
