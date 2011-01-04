#include <cstdio>
#include "usagestats.h"
#include "../Server/playerinterface.h"
#include "../PokemonInfo/battlestructs.h"

ServerPlugin * createPluginClass() {
    return new PokemonOnlineStatsPlugin();
}

PokemonOnlineStatsPlugin::PokemonOnlineStatsPlugin()
    : md5(QCryptographicHash::Md5)
{
    QDir d;
    d.mkdir("usage_stats");
    d.mkdir("usage_stats/raw");
    d.mkdir("usage_stats/formatted");
}

QString PokemonOnlineStatsPlugin::pluginName() const
{
    return "Usage Statistics";
}

inline int norm(int ev) {
    return (ev/4)*4;
}

/*
 * From here onward C functions are used in order to optimize
 * speed for that crucial problem, because we access files a lot.
 */

QByteArray PokemonOnlineStatsPlugin::data(const PokeBattle &p) const {
    QByteArray ret;
    ret.resize(bufsize);

    /* Constructs 28 bytes of raw data representing the pokemon */
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

void PokemonOnlineStatsPlugin::battleStarting(PlayerInterface *p1, PlayerInterface *p2, int mode, unsigned int &clauses, bool)
{
    /* We only keep track of battles between players of the same tier
       and not CC battles */
    if (clauses & ChallengeInfo::ChallengeCup)
        return;

    QString tier = p1->tier();

    if (p1->tier() != p2->tier()) {
        tier = QString("Mixed Tiers Gen %1").arg(p1->team().gen);
    }

    if (!existingDirs.contains(tier)) {
        QDir d;
        d.mkdir(QString("usage_stats/raw/%1").arg(tier));
        existingDirs[tier] = QString("usage_stats/raw/%1/").arg(tier);
    }

    PlayerInterface *players[2] = {p1, p2};

    for (int i = 0; i < 2; i++) {
        const TeamBattle &team = players[i]->team();

        for (int j = 0; j < 6; j++) {
            bool lead = false;

            if (mode == ChallengeInfo::Singles) {
                lead = j == 0;
            } else if (mode == ChallengeInfo::Doubles) {
                lead = j <= 1;
            } else if (mode == ChallengeInfo::Triples) {
                lead = j <= 2;
            }

            savePokemon(team.poke(j), lead, existingDirs[tier]);
        }
    }
}

/* Basically, we take the first 2 letters of the hash of the pokemon's raw data,
   and we open that file. We then put the pokemon if it isn't already in, and
   we also open another file with those two letters + _count, in which we write
   two numbers: the usage and the lead usage of the set.

   The reason for using low level functions is because this is a pretty critical
   section in my opinion for big servers, and C++ file management systems are
   pretty slow.
*/
void PokemonOnlineStatsPlugin::savePokemon(const PokeBattle &p, bool lead, const QString &d)
{
    QByteArray data = this->data(p);

    QByteArray file = (d + QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex().left(3)).toUtf8();

    FILE *raw_f = fopen(file.data(), "r+b");

    if (!raw_f) {
        raw_f = fopen(file.data(), "w+b");
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

bool PokemonOnlineStatsPlugin::hasConfigurationWidget() const {
    return false;
}
