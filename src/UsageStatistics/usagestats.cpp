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

QString PokemonOnlineStatsPlugin::pluginName()
{
    return "Usage Statistics";
}

/*
 * From here onward C functions are used in order to optimize
 * speed for that crucial problem, because we access files a lot.
 */

QByteArray PokemonOnlineStatsPlugin::data(const PokeBattle &p) const {
    QByteArray ret;
    ret.resize(bufsize);

    /* Constructs 24 bytes of raw data representing the pokemon */
    qint32 *a = (qint32*)ret.data();
    
    a[0] = p.num() << 16 + p.item();
    a[1] = p.ability() << 16 + p.gender() << 8 + p.level();
    a[2] = p.nature() << 24 + p.evs()[0] << 16 + p.evs()[1] << 8 + p.evs()[2];
    a[3] = p.evs()[3] << 16 + p.evs()[4] << 8 + p.evs()[5];
    a[4] = p.dvs()[5] << 25 + p.dvs()[4] << 20 + p.dvs()[3] << 15 + p.dvs()[2] << 10 + p.dvs()[1] << 5 + p.dvs()[0];

    /* Here the moves are sorted because we don't want to have different
       movesets when moves are in a different order */
    quint16 *moves = (quint16*) (a + 5);
    moves[0] = p.move(0).num();
    moves[1] = p.move(1).num();
    moves[2] = p.move(2).num();
    moves[3] = p.move(3).num();
    qSort(&moves[0], &moves[3]);

    return ret;
}

void PokemonOnlineStatsPlugin::battleStarting(PlayerInterface *p1, PlayerInterface *p2)
{
    if (p1->tier() != p2->tier())
        return;

    QString tier = p1->tier();
    if (!existingDirs.contains(tier)) {
        QDir d;
        d.mkdir(QString("usage_stats/raw/%1").arg(tier));
        existingDirs[tier] = QString("usage_stats/raw/%1/").arg(tier);
    }

    PlayerInterface *players[2] = {p1, p2};

    for (int i = 0; i < 2; i++) {
        const TeamBattle &team = players[i]->team();

        for (int j = 0; j < 6; j++) {
            savePokemon(team.poke(j), j==0, existingDirs[tier]);
        }
    }
}

/* Basically, we take the first 2 letters of the hash of the pokemon's raw data,
   and we open that file. We then put the pokemon if it isn't already in, and
   we also open another file with those two letters + _count, in which we write
   two numbers: the usage and the lead usage of the set. */
void PokemonOnlineStatsPlugin::savePokemon(const PokeBattle &p, bool lead, const QString &d)
{
    if (p.num() == 0)
        return;

    const QByteArray &data = this->data(p);

    QByteArray file = (d + QCryptographicHash::hash(data, QCryptographicHash::Md5).left(1).toHex()).toUtf8();
    QByteArray file2 = file + "_count";

    FILE *raw_f = fopen(file.data(), "rw");

    char buffer[bufsize];

    int count = 0;
    while (fread(buffer, sizeof(char), bufsize/sizeof(char), raw_f) == signed(bufsize) ) {
        if (memcmp(data.data(), buffer, bufsize) == 0) {
            break;
        }
        count = count + 1;
    }

    bool newFile = false;
    if (feof(raw_f)) {
        newFile = true;
        fseek(raw_f, 0, SEEK_END);
        fwrite(data.data(), sizeof(char), bufsize/sizeof(char), raw_f);
    }

    fclose(raw_f);

    FILE *count_f = fopen(file2.data(), "rw");

    fseek(count_f, count * sizeof(qint32) * 2, SEEK_SET);

    qint32 usage, leadusage;

    if (newFile) {
        usage = 0;
        leadusage = 0;
    } else {
        fread(&usage, sizeof(qint32), 1, count_f);
        fread(&leadusage, sizeof(qint32), 1, count_f);
        fseek(count_f, -sizeof(qint32)*2, SEEK_CUR);
    }

    usage += 1;
    leadusage += int(lead);

    fwrite(&usage, sizeof(qint32), 1, count_f);
    fwrite(&leadusage, sizeof(qint32), 1, count_f);

    fclose(count_f);
}
