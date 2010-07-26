#include <QtCore>
#include <ctime>
#include "../PokemonInfo/pokemoninfo.h"

/*
 * First we move to a tier.
 * we get the usage of each pokemon. (global)
 * we get the total usage for the tier
 * we create the page of each pokemon, in xml.
 * we create the pages for each pokemon, in html.
 * we create the pages of the tiers in xml.
 * we create the pages of the tiers in html
 * we create the index in html
 */

QString directory;

static void recurseRemove(const QString &path) {
    QDir d(path);

    QStringList files = d.entryList(QDir::Files | QDir::Hidden | QDir::System);

    foreach(QString file, files) {
        d.remove(file);
    }

    QStringList dirs = d.entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);

    foreach(QString dir, dirs) {
        recurseRemove(d.absoluteFilePath(dir));
    }

    d.rmdir(d.absolutePath());
}

struct SecondaryStuff {
    quint8 evs[6];
    quint8 dvs[6];
    quint8 nature;
    int usage;

    bool operator < (const SecondaryStuff &other) const {
        for(int i = 0 ; i < 6; i++) {
            if (evs[i] != other.evs[i])
                return (evs[i] < other.evs[i]);
            if (dvs[i] != other.dvs[i])
                return (dvs[i] < other.dvs[i]);
        }
        return (nature < other.nature);
    }

    bool operator == (const SecondaryStuff &other) const {
        return memcmp(evs, other.evs, sizeof(evs)) == 0 &&
                memcmp(dvs, other.dvs, sizeof(dvs)) == 0 &&
                nature == other.nature;
    }

    SecondaryStuff & operator += (const SecondaryStuff &other) {
        usage += other.usage;

        return *this;
    }

    QString toHtml() const;
};

QString SecondaryStuff::toHtml() const
{
    static const char* stats[] = {"HP", "Atk", "Def", "Spd", "SAtk", "SDef"};

    QStringList tot;

    tot.append(QString("Nature: %1").arg(NatureInfo::Name(nature)));

    QStringList inc;

    for (int i = 0; i < 6; i++) {
        if (evs[i] != 0) {
            inc.append(QString("%1 %2").arg(evs[i]).arg(stats[i]));
        }
    }

    if (inc.size() > 0) {
       tot.append("EVs: " + inc.join(" / "));
    }

    inc.clear();

    for (int i = 0; i < 6; i++) {
        if (dvs[i] != 31) {
            inc.append(QString("%1 %2").arg(dvs[i]).arg(stats[i]));
        }
    }

    if (inc.size() > 0) {
       tot.append("IVs: " + inc.join(" / "));
    }

    return tot.join(" - ");
}

struct MoveSet {
    quint16 item;
    quint16 moves[4];
    quint16 level;
    quint16 abilities[2];
    quint16 usage;
    quint16 num;

    QMap<SecondaryStuff, SecondaryStuff> options;

    MoveSet();
    MoveSet(char buffer[28], int usage, int defAb);

    bool operator < (const MoveSet &other) const {
        if (item != other.item) {
            return item < other.item;
        }

        if (level != other.level) {
            return level < other.level;
        }

        return memcmp(moves, other.moves, sizeof(moves));
    }

    bool operator == (const MoveSet &other) const {
        return item == other.item && memcmp(moves, other.moves, sizeof(moves)) == 0
                && level == other.level;
    }

    MoveSet & operator += (const MoveSet &other) {
        usage += other.usage;

        abilities[0] += other.abilities[0];
        abilities[1] += other.abilities[1];

        foreach(const SecondaryStuff &option, other.options) {
            if (options.contains(option)) {
                options[option] += option;
            } else {
                options.insert(option, option);
            }
        }

        return *this;
    }

    void write(QFile &f) const;
};

MoveSet::MoveSet()
{
}

MoveSet::MoveSet(char buffer[28], int usage, int defAb)
    : usage(usage)
{
    qint32 *buf = (qint32 *) buffer;

    item = buf[0] & 0xFFFF;
    num = buf[0] >> 16;

    if (defAb == buf[1] >> 16) {
        abilities[0] = usage;
        abilities[1] = 0;
    } else {
        abilities[0] = 0;
        abilities[1] = usage;
    }

    level = buf [1] & 0xFF;

    SecondaryStuff s;

    s.usage = usage;
    s.nature = buf[2] >> 24;
    s.evs[0] = (buf[2] >> 16) & 0xFF;
    s.evs[1] = (buf[2] >> 8) & 0xFF;
    s.evs[2] = buf[2] & 0xFF;
    s.evs[3] = buf[3] >> 16;
    s.evs[4] = (buf[3] >> 8) & 0xFF;
    s.evs[5] = buf[3] & 0xFF;

    for (int i = 0; i < 6; i++) {
        s.dvs[i] = (buf[4] >> (5-i)) & 0x1F;
    }

    qint16 *moves = (qint16 *) (&buf[5]);

    for (int i =0; i < 4; i++) {
        this->moves[i] = moves[i];
    }

    options.insert(s,s);
}

void MoveSet::write(QFile &f) const
{
    QList<int> ab = PokemonInfo::Abilities(num);
    int tot = abilities[0] + abilities[1];

    f.write(QObject::tr("<p class='pokemonTitle'>%1 @ %2 Lv. %3 -- %4</p>\n")
            .arg(PokemonInfo::Name(num), ItemInfo::Name(item)).arg(level)
            .arg(abilities[0]*abilities[1] == 0 ? (abilities[0] == 0 ? AbilityInfo::Name(ab[1]) : AbilityInfo::Name(ab[0]))
                : QString("%1 (%2 %) / %3 (%4 %)").arg(AbilityInfo::Name(ab[0])).arg(double(100*abilities[0])/tot,0,'f',2)
                .arg(AbilityInfo::Name(ab[1])).arg(double(100*abilities[1])/tot,0,'f',2)).toUtf8());
    QMultiMap<int, SecondaryStuff> usageMap;
    foreach(const SecondaryStuff &s, options)  {
        usageMap.insertMulti(s.usage, s);
    }

    QMapIterator<int, SecondaryStuff> it(usageMap);
    it.toBack();

    it.previous();

    f.write(QString("<p class='advanced'>%1 <span class='smallPercent'>(%2 %)</span> %3</p>\n")
            .arg(it.value().toHtml()).arg(double(100*it.value().usage)/usage,0,'f',1)
            .arg(it.hasPrevious() ? "<input type='button' onclick='hideShow(this)' value='+'/>": "").toUtf8());

    if (it.hasPrevious()) {
        f.write("<div style='display: none'>\n");

        while (it.hasPrevious()) {
            it.previous();
            f.write(QString("<p class='advanced'>%1 <span class='smallPercent'>(%2 %)</span></p>\n")
                    .arg(it.value().toHtml()).arg(double(100*it.value().usage)/usage,0,'f',1).toUtf8());
        }

        f.write("</div>\n");
    }

    f.write("<ul class='moveList'>\n");
    for (int i = 0; i < 4; i++) {
        f.write(QString("\t<li class='move'>%1</li>\n").arg(MoveInfo::Name(moves[i])).toUtf8());
    }
    f.write("</ul>\n");
}

static QString getImageLink(int pokemon)
{
    return QString("<img src='../poke_img/%1/DP%2.png' />").arg(pokemon).arg(PokemonInfo::Gender(pokemon) == Pokemon::FemaleAvail ? "f" : "m");
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    PokemonInfo::init("db/pokes/");
    MoveInfo::init("db/moves/");
    AbilityInfo::init("db/abilities/");
    ItemInfo::init("db/items/");
    NatureInfo::init("db/natures/");

    srand(time(NULL));
    for (int i = 0; i < 100; i++)
        rand();

    QByteArray data;

    for (int i = 0; i < 4; i++)
        data.append(QByteArray::number(rand()));

    /* For the commands we're using, the '\\' for windows is necessary */
#ifdef WIN32
    QString dirname = "usage_stats\\.tmp" + QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex().left(10);
#else
    QString dirname = "usage_stats/.tmp" + QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex().left(10);
#endif

    /* First, copy the files to another directory */
    QDir d;
    d.mkpath(dirname);
    d.cd(dirname);

#ifdef WIN32
    system( ("xcopy usage_stats\\raw\\* " + dirname + " /s").toAscii().data() );
#else
    system( ("cp -R usage_stats/raw " + dirname).toAscii().data() );
#endif

    QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QString dir, dirs) {
        d.cd(dir);

        QStringList files = d.entryList(QDir::Files);

        /* First we get the usage of each pokemon */
        QHash<int, int> usage;
        qint32 totalusage(0);

        foreach(QString file, files) {
            FILE *f = fopen(d.absoluteFilePath(file).toAscii().data(), "r");

            char buffer[28];
            while (fread(buffer, sizeof(char), 28/sizeof(char), f) == 28) {
                qint32 iusage(0), ileadusage(0);

                fread(&iusage, sizeof(qint32), 1, f);
                fread(&ileadusage, sizeof(qint32), 1, f);

                int pokenum = (*((qint32*) buffer)) >> 16;

                usage[pokenum] += iusage;
                totalusage += iusage;
            }

            fclose(f);
        }

        QDir outDir;
        outDir.mkpath("usage_stats/formatted/" + dir);
        outDir.cd("usage_stats/formatted/" + dir);

        QFile index(outDir.absoluteFilePath("index.html"));

        index.open(QIODevice::WriteOnly);

        int totalBattles = totalusage/6;

        index.write(QObject::tr("<html><head>\n<title>Tier %1</title>\n"
                                "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
                                "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style.css\" />\n</head><body>\n").arg(dir).toUtf8());
        index.write(QObject::tr("<p class='tierBattleNumber'>Number of Battles: %1</p>\n").arg(totalBattles/2).toUtf8());
        QHashIterator<int, int> hit(usage);
        QMultiMap<int, int> reverseUsage;

        while (hit.hasNext()) {
            hit.next();
            reverseUsage.insert(hit.value(), hit.key());
        }

        int i = 0;

        QMapIterator<int, int> it(reverseUsage);
        it.toBack();

        while (it.hasPrevious() && i < 5) {
            i += 1;
            it.previous();
            index.write(QString("<p class='topPokemon'>#%1 - <a href='%4.html'>%2</a> (%3 %)</p>\n").arg(i)
                        .arg(PokemonInfo::Name(it.value())).arg(double(100*it.key())/totalBattles,0,'f',2)
                        .arg(it.value()).toUtf8());
            index.write(getImageLink(it.value()).append('\n').toUtf8());
        }

        while (it.hasPrevious()) {
            i += 1;
            it.previous();
            index.write(QString("<p class='lowPokemon'>#%1 - <img src='../icons/%4.PNG' /> <a href='%4.html'>%2</a> (%3 %)</p>\n")
                        .arg(i).arg(PokemonInfo::Name(it.value())).arg(double(100*it.key())/totalBattles,0,'f',2)
                        .arg(it.value()).toUtf8());
        }

        index.write(QObject::tr("</body></html>").toUtf8());

        it.toBack();

        while (it.hasPrevious()) {
            it.previous();

            int pokemon = it.value();

            QMap<MoveSet, MoveSet> movesets;
            int defAb = PokemonInfo::Abilities(it.value())[0];

            foreach(QString file, files) {
                FILE *f = fopen(d.absoluteFilePath(file).toAscii().data(), "r");

                char buffer[28];
                while (fread(buffer, sizeof(char), 28/sizeof(char), f) == 28) {
                    qint32 iusage(0), ileadusage(0);

                    fread(&iusage, sizeof(qint32), 1, f);
                    fread(&ileadusage, sizeof(qint32), 1, f);

                    int pokenum = (*((qint32*) buffer)) >> 16;

                    if (pokenum == pokemon) {
                        MoveSet m = MoveSet(buffer, iusage, defAb);

                        if (!movesets.contains(m)) {
                            movesets.insert(m, m);
                        } else {
                            movesets[m] += m;
                        }
                    }
                }
                fclose(f);
            }

            QFile pokef(outDir.absoluteFilePath("%1.html").arg(pokemon));
            pokef.open(QIODevice::WriteOnly);

            pokef.write(QObject::tr("<html><head>\n<title>Pok&eacute;mon %1</title>\n"
                                     "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
                                     "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style.css\" />\n"
                                     "<script type=\"text/javascript\" src=\"../script.js\"></script></head><body>\n")
                        .arg(PokemonInfo::Name(pokemon)).toUtf8());
            pokef.write(QString("<header><a href='index.html'>Tier %1</a></header>").arg(dir).toUtf8());
            pokef.write(getImageLink(pokemon).append('\n').toUtf8());
            pokef.write(QObject::tr("<p class='pokemonBattleNumber'>Usage Percentage: %1 % (%2 battles)</p>")
                        .arg(double(100*it.key())/totalBattles,0,'f',2).arg(it.key()).toUtf8());

            QMultiMap <int, MoveSet> usageOrder;

            QMapIterator<MoveSet, MoveSet> mit (movesets);

            while (mit.hasNext()) {
                mit.next();

                usageOrder.insertMulti(mit.value().usage, mit.value());
            }

            QMapIterator<int, MoveSet> usageIt(usageOrder);

            usageIt.toBack();
            int i = 0;

            while (usageIt.hasPrevious() && i < 25) {
                usageIt.previous();
                i+= 1;

                pokef.write(QObject::tr("<p class='pokemonRank'># %1 - %2 % (%3 battles)</p>\n<hr />\n")
                            .arg(i).arg(double(100*usageIt.key())/it.key(),0,'f',2).arg(usageIt.key()).toUtf8());
                usageIt.value().write(pokef);
            }
            pokef.write("</body></html>");
        }

        d.cdUp();
    }

    recurseRemove(dirname);

    return 0;
}
