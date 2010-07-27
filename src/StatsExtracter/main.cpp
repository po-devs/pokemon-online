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

class Skeleton {
public:
    Skeleton(const QString &path);

    void addDefaultValue(const char *member, const QVariant &value);
    Skeleton &appendChild(const QString &item);

    QString generate() const;
private:
    Skeleton *top;
    Skeleton(Skeleton *top, const QString &body);
    Skeleton *parent () const {
        return top;
    }
    bool isBoss() const {
        return top == NULL;
    }

    /* {moveset *} */
    QHash<QString, QList<Skeleton> > children;
    /* {pokemon} */
    QHash<QString, QString> values;
    /* moveset{{
       xxx
       }} */
    QHash<QString, QString> bodies;
    /* page {{
       }} */
    QString body;

    QString getSkeletonBody(const QString &name) const;
};

Skeleton::Skeleton(const QString &path) {
    QFile f(path);
    f.open(QIODevice::ReadOnly);

    QString text = QString(f.readAll());

    QRegExp r("([a-z]+)\\{\\{((?:[^}].?)*)\\}\\}", Qt::CaseSensitive, QRegExp::Wildcard);
    r.setPatternSyntax(QRegExp::RegExp2);
    int pos = 0;

    while (r.indexIn(text, pos) > -1) {
        pos += r.matchedLength();

        if (r.cap(1) == "page") {
            body = r.cap(2);
        } else {
            bodies[r.cap(1)] = r.cap(2);
        }
    }

    top = NULL;
}

Skeleton::Skeleton(Skeleton *top, const QString &body) : top(top), body(body) {
}

void Skeleton::addDefaultValue(const char *member, const QVariant &value) {
    values[member] = value.toString();
}

Skeleton & Skeleton::appendChild(const QString &item) {
    children[item].push_back(Skeleton(parent() == NULL ? this : parent(), getSkeletonBody(item)));
    return children[item].back();
}

QString Skeleton::getSkeletonBody(const QString &name) const {
    return isBoss() ? bodies.value(name) : parent()->getSkeletonBody(name);
}

QString Skeleton::generate() const {
    QHashIterator<QString, QList<Skeleton> > it(children);
    
    QString body = this->body;

    while (it.hasNext()) {
        it.next();

        if (it.value().size() == 0) {
            continue;
        }

        if (body.indexOf(QString("{%1}").arg(it.key())) != -1) {
            body.replace(QString("{%1}").arg(it.key()), it.value().front().generate());
        } else if (body.indexOf(QString("{%1*}").arg(it.key())) != -1) {
            QString remplacement;

            for (int i = 0; i < it.value().size(); i++) {
                remplacement += it.value()[i].generate();
            }

            body.replace(QString("{%1*}").arg(it.key()), remplacement);
        }
    }

    QHashIterator <QString, QString> it2(values);
    while (it2.hasNext()) {
        it2.next();
        body.replace(QString("{%1}").arg(it2.key()), it2.value());
    }

    body.replace(QRegExp("\\{[^}]*\\}"), "");

    return body;
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

    void complete(Skeleton &s) const;
};

void SecondaryStuff::complete(Skeleton &s) const
{
    static const char* stats[] = {"HP", "Atk", "Def", "Spd", "SAtk", "SDef"};

    s.addDefaultValue("nature", NatureInfo::Name(nature));

    QStringList inc;

    for (int i = 0; i < 6; i++) {
        if (evs[i] != 0)
            inc.append(QString("%1 %2").arg(evs[i]).arg(stats[i]));
    }

    if (inc.size() > 0) {
        s.addDefaultValue("evs", inc.join(" / "));
    } else {
        s.addDefaultValue("evs", 0);
    }

    inc.clear();

    for (int i = 0; i < 6; i++) {
        if (dvs[i] != 31) {
            inc.append(QString("%1 %2").arg(dvs[i]).arg(stats[i]));
        }
    }

    if (inc.size() > 0) {
        s.addDefaultValue("ivs", inc.join(" / "));
    } else {
        s.addDefaultValue("ivs", "All 31");
    }
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

    void complete(Skeleton &m) const;
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

void MoveSet::complete(Skeleton &m) const
{
    QList<int> ab = PokemonInfo::Abilities(num);
    int tot = abilities[0] + abilities[1];

    m.addDefaultValue("pokemon", PokemonInfo::Name(num));
    m.addDefaultValue("item", ItemInfo::Name(item));
    m.addDefaultValue("level", level);
    m.addDefaultValue("abilities", abilities[0]*abilities[1] == 0 ? (abilities[0] == 0 ? AbilityInfo::Name(ab[1]) : AbilityInfo::Name(ab[0]))
        : QString("%1 (%2 %) / %3 (%4 %)").arg(AbilityInfo::Name(ab[0])).arg(double(100*abilities[0])/tot,0,'f',1)
        .arg(AbilityInfo::Name(ab[1])).arg(double(100*abilities[1])/tot,0,'f',1).toUtf8());


    QMultiMap<int, SecondaryStuff> usageMap;
    foreach(const SecondaryStuff &s, options)  {
        usageMap.insertMulti(s.usage, s);
    }

    QMapIterator<int, SecondaryStuff> it(usageMap);
    it.toBack();

    it.previous();

    Skeleton &s = m.appendChild("firststatset");
    s.addDefaultValue("percentage", QString::number(double(100*it.value().usage)/usage, 'f', 1));
    it.value().complete(s);

    while (it.hasPrevious()) {
        it.previous();
        Skeleton &s = m.appendChild("statset");
        s.addDefaultValue("percentage", QString::number(double(100*it.value().usage)/usage, 'f', 1));
        it.value().complete(s);
    }

    m.addDefaultValue("move1", MoveInfo::Name(moves[0]));
    m.addDefaultValue("move2", MoveInfo::Name(moves[1]));
    m.addDefaultValue("move3", MoveInfo::Name(moves[2]));
    m.addDefaultValue("move4", MoveInfo::Name(moves[3]));
}

static QString getImageLink(int pokemon, bool root=false)
{
    return QString(root ? "poke_img/%1/DP%2.png" : "../poke_img/%1/DP%2.png").arg(pokemon).arg(PokemonInfo::Gender(pokemon) == Pokemon::FemaleAvail ? "f" : "m");
}

static QString getIconLink(int pokemon, bool root=false)
{
    return QString(root ? "icons/%1.PNG" : "../icons/%1.PNG").arg(pokemon);
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

    QList<QPair<QString, int> > mostUsedPokemon;

    foreach(QString dir, dirs) {
        d.cd(dir);

        QStringList files = d.entryList(QDir::Files);

        /* First we get the usage of each pokemon */
        QHash<int, int> usage;
        qint32 totalusage(0);

        foreach(QString file, files) {
            FILE *f = fopen(d.absoluteFilePath(file).toAscii().data(), "r");

            char buffer[28];
            int pos = 0;
            while (fread(buffer, sizeof(char), 28/sizeof(char), f) == 28) {
                pos += 28;
                qint32 iusage(0), ileadusage(0);

                fseek(f, pos, SEEK_SET);
                fread(&iusage, sizeof(qint32), 1, f);
                fread(&ileadusage, sizeof(qint32), 1, f);
                pos += 2 * sizeof(qint32);
                fseek(f, pos, SEEK_SET);

                int pokenum = (*((qint32*) buffer)) >> 16;

                usage[pokenum] += iusage;
                totalusage += iusage;
            }

            fclose(f);
        }

        QDir outDir;
        outDir.mkpath("usage_stats/formatted/" + dir);
        outDir.cd("usage_stats/formatted/" + dir);

        int totalBattles = totalusage/6;

        Skeleton tierSk("usage_stats/formatted/tier_page.template");
        tierSk.addDefaultValue("tier", dir);
        tierSk.addDefaultValue("battles", totalBattles/2);

        QHashIterator<int, int> hit(usage);
        QMultiMap<int, int> reverseUsage;

        while (hit.hasNext()) {
            hit.next();
            reverseUsage.insert(hit.value(), hit.key());
        }

        int i = 0;

        QMapIterator<int, int> it(reverseUsage);
        it.toBack();

        while (it.hasPrevious()) {
            i += 1;
            it.previous();
            Skeleton &childSk = tierSk.appendChild(i <= 5 ? "toppokemon" : "lowpokemon");
            childSk.addDefaultValue("rank", i);
            childSk.addDefaultValue("imagelink", getImageLink(it.value()));
            childSk.addDefaultValue("iconlink", getIconLink(it.value()));
            childSk.addDefaultValue("pokemonlink", QString("%1.html").arg(it.value()));
            childSk.addDefaultValue("percentage", QString::number(double(100*it.key())/totalBattles,'f',2));
            childSk.addDefaultValue("pokemon", PokemonInfo::Name(it.value()));
        }

        QFile index(outDir.absoluteFilePath("index.html"));
        index.open(QIODevice::WriteOnly);
        index.write(tierSk.generate().toUtf8());
        index.close();

        it.toBack();

        while (it.hasPrevious()) {
            it.previous();

            int pokemon = it.value();

            QMap<MoveSet, MoveSet> movesets;
            int defAb = PokemonInfo::Abilities(it.value())[0];

            foreach(QString file, files) {
                FILE *f = fopen(d.absoluteFilePath(file).toAscii().data(), "r");

                char buffer[28];
                int pos = 0;
                while (fread(buffer, sizeof(char), 28/sizeof(char), f) == 28) {
                    pos += 28;
                    qint32 iusage(0), ileadusage(0);

                    fseek(f, pos, SEEK_SET);
                    fread(&iusage, sizeof(qint32), 1, f);
                    fread(&ileadusage, sizeof(qint32), 1, f);
                    pos += 2 * sizeof(qint32);
                    fseek(f, pos, SEEK_SET);

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

            Skeleton s("usage_stats/formatted/pokemon_page.template");
            s.addDefaultValue("pokemon", PokemonInfo::Name(pokemon));
            s.addDefaultValue("tier", dir);
            s.addDefaultValue("imagelink", getImageLink(pokemon));
            s.addDefaultValue("percentage", QString::number(double(100*it.key())/totalBattles,'f',2));
            s.addDefaultValue("battles", it.key());

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

                Skeleton &m = s.appendChild("moveset");
                m.addDefaultValue("rank", i);
                m.addDefaultValue("percentage", QString::number(double(100*usageIt.key())/it.key(),'f',2));
                m.addDefaultValue("battles", usageIt.key());
                usageIt.value().complete(m);
            }
            QFile pokef(outDir.absoluteFilePath("%1.html").arg(pokemon));
            pokef.open(QIODevice::WriteOnly);
            pokef.write(s.generate().toUtf8());
        }

        mostUsedPokemon.push_back(QPair<QString, int> (dir, reverseUsage.size() > 0 ? (--reverseUsage.end()).value() : 0));
        d.cdUp();
    }

    typedef QPair<QString, int> pair;

    Skeleton indexSk("usage_stats/formatted/index.template");
    foreach(pair p, mostUsedPokemon) {
        if (p.second == 0)
            continue;
        Skeleton &pok = indexSk.appendChild("tier");

        pok.addDefaultValue("tier", p.first);
        pok.addDefaultValue("pokemon", PokemonInfo::Name(p.second));
        pok.addDefaultValue("imagelink", getImageLink(p.second, true));
        pok.addDefaultValue("iconlink", getIconLink(p.second, true));
    }

    QFile f("usage_stats/formatted/index.html");
    f.open(QIODevice::WriteOnly);
    f.write(indexSk.generate().toUtf8());

    recurseRemove(dirname);

    return 0;
}
