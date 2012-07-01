#include <QPixmapCache>

#include "pokemoninfo.h"
#include "pokemonstructs.h"

#ifdef _WIN32
#include "../../SpecialIncludes/zip.h"
#else
#include <zip.h>
#endif
#include "../Utilities/functions.h"

/*initialising static variables */
QString PokemonInfo::m_Directory;
QMap<Pokemon::uniqueId, QString> PokemonInfo::m_Names;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Weights;
QHash<int, QHash<int, QString> > PokemonInfo::m_Desc;
QHash<int, QString> PokemonInfo::m_Classification;
QHash<int, int> PokemonInfo::m_GenderRates;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Height;

QHash<Pokemon::uniqueId, int> PokemonInfo::m_Genders;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Type1[NUMBER_GENS];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Type2[NUMBER_GENS];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_MinLevels[NUMBER_GENS];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_MinEggLevels[NUMBER_GENS];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Abilities[NUMBER_GENS][3];
QHash<Pokemon::uniqueId, PokeBaseStats> PokemonInfo::m_BaseStats;
QHash<Pokemon::uniqueId,int> PokemonInfo::m_SpecialStats;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_LevelBalance;
QHash<Pokemon::uniqueId, PokemonMoves> PokemonInfo::m_Moves;
QHash<int, quint16> PokemonInfo::m_MaxForme;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Options;
int PokemonInfo::m_trueNumberOfPokes;
QSet<Pokemon::uniqueId> PokemonInfo::m_AestheticFormes;

QHash<int, QList<int> > PokemonInfo::m_Evolutions;
QHash<int, int> PokemonInfo::m_OriginalEvos;
QHash<int, QList<int> > PokemonInfo::m_DirectEvos;
QHash<int, int> PokemonInfo::m_PreEvos;

QString MoveInfo::m_Directory;
MoveInfo::Gen MoveInfo::gens[Version::NumberOfGens];
QHash<int,QString> MoveInfo::m_Names;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;
QHash<int, QStringList> MoveInfo::m_MoveMessages;
QHash<int,QString> MoveInfo::m_Details;
QHash<int,QString> MoveInfo::m_SpecialEffects, MoveInfo::m_RbySpecialEffects;
QHash<int,int> MoveInfo::m_OldMoves;
QHash<int,bool> MoveInfo::m_KingRock;

QString ItemInfo::m_Directory;
QHash<int,QString> ItemInfo::m_BerryNames;
QHash<int,QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QList<QString> ItemInfo::m_SortedNames[NUMBER_GENS];
QList<QString> ItemInfo::m_SortedUsefulNames[NUMBER_GENS];
QHash<int,QList<ItemInfo::Effect> > ItemInfo::m_RegEffects[NUMBER_GENS];
QHash<int,QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QHash<int, QStringList> ItemInfo::m_RegMessages;
QHash<int, QStringList> ItemInfo::m_BerryMessages;
QHash<int,int> ItemInfo::m_Powers;
QHash<int,int> ItemInfo::m_BerryPowers;
QHash<int,int> ItemInfo::m_BerryTypes;
QList<int> ItemInfo::m_UsefulItems;
QSet<int> ItemInfo::m_GenItems[NUMBER_GENS];

QHash<int, QString> TypeInfo::m_Names;
QString TypeInfo::m_Directory;
QHash<int, QVector<int> > TypeInfo::m_TypeVsType;
QHash<int, QVector<int> > TypeInfo::m_TypeVsTypeGen1;
QHash<int, int> TypeInfo::m_Categories;

QHash<int, QString> NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QHash<int, QString> CategoryInfo::m_Names;
QString CategoryInfo::m_Directory;

QHash<int,QString> AbilityInfo::m_Names, AbilityInfo::m_Desc, AbilityInfo::m_BattleDesc;
QString AbilityInfo::m_Directory;
QHash<int,AbilityInfo::Effect> AbilityInfo::m_Effects[NUMBER_GENS];
QHash<int, QStringList> AbilityInfo::m_Messages;
QHash<int,int> AbilityInfo::m_OldAbilities;

QHash<int, QString> GenderInfo::m_Names;
QString GenderInfo::m_Directory;

QString HiddenPowerInfo::m_Directory;

QString StatInfo::m_Directory;
QHash<int, QString> StatInfo::m_stats;
QHash<int, QString> StatInfo::m_status;

QString GenInfo::m_Directory;
QHash<int, QString> GenInfo::m_gens;
QHash<Pokemon::gen, QString> GenInfo::m_versions;

QByteArray readZipFile(const char *archiveName, const char *fileName)
{
    int error = 0;
    char buffer[1024];
    int readsize = 0;
    QByteArray ret;

    zip * archive = zip_open(archiveName, 0, &error);

    if (!archive)
    {
        return ret;
    }

    zip_file *file = zip_fopen(archive, fileName, 0);

    if (!file)
    {
        zip_close(archive);
        return ret;
    }

    do
    {
        ret.append(buffer, readsize);

        readsize = zip_fread(file, buffer, 1024);
    } while (readsize > 0) ;

    zip_fclose(file);
    zip_close(archive);

    return ret;
}

static QString transPath, modPath;

void PokemonInfoConfig::changeTranslation(const QString &ts)
{
    if (ts.length() > 0) {
        transPath = QString("trans/%1/").arg(ts);
    } else {
        transPath.clear();
    }
}

void PokemonInfoConfig::changeMod(const QString &mod, FillMode::FillModeType mode)
{
    if (mod.length() == 0 || mode == FillMode::NoMod) {
        modPath.clear();
    } else {
        QString cleanMod = QString::fromUtf8(QUrl::toPercentEncoding(mod));
        if (mode == FillMode::Client) {
            modPath = appDataPath("Mods") + "/" + cleanMod + "/";
        } else {
            modPath = QString("Mods/%1/").arg(cleanMod);
        }
    }
}

static QStringList allFiles(const QString &filename, bool trans=false) {
    QStringList ret;

    ret << filename;

    if (trans && transPath.length() > 0 && QFile::exists(transPath+filename)) {
        ret << (transPath + filename);
    }

    if (modPath.length() > 0 && QFile::exists(modPath+filename)) {
        ret << (modPath + filename);
    }

    return ret;
}

template <class T, class U>
static QHash<U,T> reverse_hash(const QHash<T,U> &hash) {
    QHashIterator<T,U> it(hash);
    QHash<U,T> ret;

    ret.reserve(hash.size());

    while (it.hasNext()) {
        it.next();
        ret[it.value()] = it.key();
    }

    return ret;
}

static void fill_container_with_file(QStringList &container, const QString & filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);

        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream filestream(&file);

        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            container << filestream.readLine();
        }
    }
}

static void fill_int_char(QHash<int, char> &container, const QString & filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int line, var;
            filestream >> line >> var;
            container[line] = var;
        }
    }
}

static void fill_int_bool(QHash<int, bool> &container, const QString & filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int line, var;
            filestream >> line >> var;
            container[line] = var;
        }
    }
}

static void fill_int_char(QHash<int, unsigned char> &container, const QString & filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int line, var;
            filestream >> line >> var;
            container[line] = var;
        }
    }
}

static void fill_int_char(QHash<int, signed char> &container, const QString & filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int line, var;
            filestream >> line >> var;
            container[line] = var;
        }
    }
}

static void fill_uid_int(QHash<Pokemon::uniqueId, int> &container, const QString &filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream filestream(&file);
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            QString current = filestream.readLine().trimmed();
            QString other_data;
            Pokemon::uniqueId pokeid;
            bool ok = Pokemon::uniqueId::extract(current, pokeid, other_data);
            if(ok) {
                bool converted;
                int data = other_data.toInt(&converted);
                if(converted) {
                    container[pokeid] = data;
                }
            }
        }
    }
}

static void fill_uid_str(QHash<Pokemon::uniqueId, QString> &container, const QString &filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream filestream(&file);
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            QString current = filestream.readLine().trimmed();
            QString other_data;
            Pokemon::uniqueId pokeid;
            bool ok = Pokemon::uniqueId::extract(current, pokeid, other_data);
            if(ok) {
                container[pokeid] = other_data;
            }
        }
    }
}

static void fill_gen_string(QHash<Pokemon::gen, QString> &container, const QString &filename, bool trans = false)
{

    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream filestream(&file);
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            QString current = filestream.readLine().trimmed();
            QString other_data;
            Pokemon::gen gen;
            bool ok = Pokemon::gen::extract(current, gen, other_data);
            if(ok) {
                container[gen] = other_data;
            }
        }
    }
}

template <class T, class U>
static void fill_double(QHash<T, U> &container, const QString &filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);

        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream filestream(&file);

        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            T var1;
            U var2;
            filestream >> var1 >> var2;
            container.insert(var1, var2);
        }
    }
}

static void fill_int_str(QHash<int, QString> &container, const QString &filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);

        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream filestream(&file);

        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int var1;
            filestream >> var1;
            container.insert(var1, filestream.readLine().trimmed());
        }
    }
}

template <class T>
static void fill_container_with_file(T &container, const QString & filename, bool trans = false)
{
    container.clear();

    foreach(QString fileName, allFiles(filename, trans)) {
        QFile file(fileName);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            typename T::value_type var;
            filestream >> var;
            container << var;
        }
    }
}

QString PokemonInfo::Desc(const Pokemon::uniqueId &pokeid, int cartridge)
{
    QString result = "";
    if(m_Desc.contains(cartridge)) {
        result = m_Desc.value(cartridge).value(pokeid.pokenum, "");
    }
    return result;
}

QString PokemonInfo::Classification(const Pokemon::uniqueId &pokeid)
{
    return m_Classification.value(pokeid.pokenum, "");
}

int PokemonInfo::GenderRate(const Pokemon::uniqueId &pokeid)
{
    return m_GenderRates.value(pokeid.pokenum,4);
}

QString PokemonInfo::Height(const Pokemon::uniqueId &pokeid)
{
    return m_Height.value(pokeid, "0.0");
}

int PokemonInfo::Type1(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Type1[gen.num-GEN_MIN].value(pokeid);
}

int PokemonInfo::Type2(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Type2[gen.num-GEN_MIN].value(pokeid);
}

int PokemonInfo::calc_stat(int gen, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    return ((2*basestat + dv * (1 + (gen <= 2) ) + ev/4)*level)/100 + 5;
}

int PokemonInfo::Stat(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int stat, int level, quint8 dv, quint8 ev)
{
    quint8 basestat = PokemonInfo::BaseStats(pokeid).baseStat(stat);

    if (stat == SpAttack && gen == 1) {
        basestat = SpecialStat(pokeid);
    }

    if (stat == Hp) {
        /* Formerly direct check for Shedinja */
        if(m_Options.value(pokeid).contains('1')) {
            return 1;
        }else{
            return calc_stat(gen.num, basestat, level, dv, ev) + level + 5;
        }
    }
    return calc_stat(gen.num, basestat, level, dv, ev);
}

int PokemonInfo::FullStat(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int nature, int stat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp) {
        return Stat(pokeid, gen, stat, level, dv, ev);
    }
    else {
        return Stat(pokeid, gen, stat, level, dv, ev) * (10+NatureInfo::Boost(nature, stat)) / 10;
    }
}

int PokemonInfo::BoostedStat(int stat, int boost)
{
    return stat * std::max(2, 2+boost) / std::max(2, 2-boost);
}

void PokemonInfo::init(const QString &dir, FillMode::FillModeType mode, const QString &modName)
{
    /* makes sure it isn't already initialized */
    if (NumberOfPokemons() != 0) return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    // Load db/pokes data.
    reloadMod(mode, modName);
}

void PokemonInfo::reloadMod(FillMode::FillModeType mode, const QString &modName)
{
    PokemonInfoConfig::changeMod(modName, mode);

    clearData();

    loadNames();
    loadEvos();
    loadMoves();

    fill_uid_int(m_Genders, path("poke_gender.txt"));

    for (int i = 0; i < NUMBER_GENS; i++) {
        Pokemon::gen gen = i+GEN_MIN;

        fill_uid_int(m_Type1[i], path(QString("poke_type1-%1G.txt").arg(gen.num)));
        fill_uid_int(m_Type2[i], path(QString("poke_type2-%1G.txt").arg(gen.num)));

        if (gen >= 3) {
            for (int j = 0; j < 3; j++) {
                fill_uid_int(m_Abilities[i][j], path(QString("poke_ability%1_%2G.txt").arg(j+1).arg(gen.num)));
            }
        }
    }

    fill_uid_int(m_LevelBalance, path("level_balance.txt"));
    fill_uid_int(m_SpecialStats, path("specialstat.txt"));
    loadClassifications();
    loadGenderRates();
    loadHeights();
    loadDescriptions();
    loadBaseStats();
    loadMinLevels();

    makeDataConsistent();
}

void PokemonInfo::retranslate()
{
    loadNames();
    loadClassifications();
    loadDescriptions();
}

void PokemonInfo::clearData()
{
    m_Names.clear();
    m_Options.clear();
    m_MaxForme.clear();
    m_Weights.clear();
    m_Genders.clear();
    for (int i = 0; i < NUMBER_GENS; ++i) {
        m_Type1[i].clear();
        m_Type2[i].clear();
        for (int j = 0; j < 3; ++j) {
            m_Abilities[i][j].clear();
        }
        m_MinLevels[i].clear();
    }
    m_LevelBalance.clear();
    m_Classification.clear();
    m_GenderRates.clear();
    m_Height.clear();
    m_Desc.clear();
    m_BaseStats.clear();
    m_Evolutions.clear();
    m_OriginalEvos.clear();
    m_PreEvos.clear();
    m_DirectEvos.clear();
    m_AestheticFormes.clear();
    m_Moves.clear();
}

void PokemonInfo::loadClassifications()
{
    fill_int_str(m_Classification, path("classification.txt"), true);
}

void PokemonInfo::loadGenderRates()
{
    fill_double(m_GenderRates, path("gender_rate.txt"));
}

void PokemonInfo::loadMinLevels()
{
    for (int i = 0; i < NUMBER_GENS; i++) {
        m_MinLevels[i].clear();
        m_MinEggLevels[i].clear();

        QHash<Pokemon::uniqueId, QString> temp;
        fill_uid_str(temp, path(QString("minlevels_G%1.txt").arg(GEN_MIN+i)));

        QHashIterator<Pokemon::uniqueId, QString> it(temp);

        while (it.hasNext()) {
            it.next();
            m_MinLevels[i][it.key()] = it.value().section('/', 0, 0).toInt();
            m_MinEggLevels[i][it.key()] = it.value().section('/', -1, -1).toInt();
        }
    }
}

void PokemonInfo::loadHeights()
{
    fill_uid_str(m_Height,path("height.txt"));
    fill_uid_str(m_Weights,path("weight.txt"));
}

void PokemonInfo::loadDescriptions()
{
    static const int CARTS_LEN = 3;
    int carts[] = { 14, 15, 16 };
    for(int i = 0; i < CARTS_LEN; i++) {
        fill_int_str(m_Desc[carts[i]], path("description_%1").arg(carts[i]), true);
    }
}


int PokemonInfo::TrueCount(Pokemon::gen gen)
{
    if (gen.num == 1)
        return 152;
    if (gen.num == 2)
        return 252;
    if (gen.num == 3)
        return 387;
    if (gen.num == 4)
        return 494;
    return m_trueNumberOfPokes;
}

int PokemonInfo::NumberOfPokemons()
{
    return m_Names.size();
}

QString PokemonInfo::Name(const Pokemon::uniqueId &pokeid)
{
    if(m_Names.contains(pokeid))
    {
        return m_Names.value(pokeid);
    }else{
        return m_Names.value(Pokemon::uniqueId());
    }
}

bool PokemonInfo::Exists(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    if (pokeid.toPokeRef() == Pokemon::SpikyPichu) {
        return gen == 4;
    }
    if(m_Names.contains(pokeid))
    {
        return pokeid.pokenum < TrueCount(gen);
    }else{
        return false;
    }
}

bool PokemonInfo::Exists(const Pokemon::uniqueId &pokeid)
{
    return m_Names.contains(pokeid);
}

Pokemon::uniqueId PokemonInfo::Number(const QString &pokename)
{
    return m_Names.key(pokename, Pokemon::uniqueId());
}

int PokemonInfo::LevelBalance(const Pokemon::uniqueId &pokeid)
{
    return m_LevelBalance.value(pokeid);
}

int PokemonInfo::Gender(const Pokemon::uniqueId &pokeid)
{
    return m_Genders.value(pokeid);
}

bool PokemonInfo::IsAesthetic(Pokemon::uniqueId id)
{
    return m_AestheticFormes.contains(id);
}

Pokemon::uniqueId PokemonInfo::NonAestheticForme(Pokemon::uniqueId id)
{
    return IsAesthetic(id) ? OriginalForme(id) : id;
}

QPixmap PokemonInfo::Picture(const QString &url)
{
    QStringList params = url.split('&');

    Pokemon::gen gen = GEN_MAX;
    int gender = 0;
    Pokemon::uniqueId num = Pokemon::NoPoke;
    bool shiny=false;
    bool back = false;
    bool substitute = false;
    bool cropped = false;

    foreach (QString param, params) {
        QString par = param.section('=', 0,0);
        QString val = param.section('=', 1);

        if (par.length() > 0 && val.length() == 0 && (par[0].isDigit() || par == "substitute")) {
            val = par;
            par = "num";
        }

        if (par == "gen") {
            gen = val.toInt();
        } else if (par == "num") {
            if (val == "substitute") {
                substitute  = true;
            } else {
                if (val.indexOf('-') != -1) {
                    num = Pokemon::uniqueId(val.section('-', 0,0).toInt(), val.section('-', 1).toInt());
                } else {
                    num = val.toInt();
                }
            }
        } else if (par == "shiny") {
            shiny = val == "true";
        } else if (par == "gender") {
            gender = val == "male" ? Pokemon::Male : (val == "female"?Pokemon::Female : Pokemon::Neutral);
        } else if (par == "back") {
            back = val == "true";
        } else if (par == "cropped") {
            cropped = val == "true";
        }
    }

    if (substitute) {
        return Sub(gen, back);
    } else {
        QPixmap ret = Picture(num, gen, gender, shiny, back);

        if (cropped) {
            QImage img = ret.toImage();
            cropImage(img);
            ret = QPixmap::fromImage(img);
        }

        return ret;
    }
}

QPixmap PokemonInfo::Picture(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int gender, bool shiney, bool back, bool mod)
{
    QString archive;

    if (gen.num == 1)
        archive = path("rby.zip");
    else if (gen.num == 2)
        archive = path("gsc.zip");
    else if (gen.num == 3)
        archive = path("advance.zip");
    else if (gen.num == 4)
        archive = path("generation-4.zip");
    else {
        archive = path("black-white.zip");
    }

    if (mod && modPath.length() > 0 && QFile::exists(modPath+archive)) {
        archive.prepend(modPath);
    } else {
        mod = false;
    }

    QString file;

    if (gen.num == 1)
        file = QString("%1/%2").arg(pokeid.toString(), back?"GBRYback.png":"Y.gif");
    else if (gen.num == 2)
        file = QString("%1/%2.png").arg(pokeid.toString(), back?"GSCback%3":"S%3").arg(shiney?"s":"");
    else if (gen.num ==3)
        file = QString("firered-leafgreen/%2%4%1.png").arg(pokeid.toString(), back?"back/":"", shiney?"shiny/":"");
    else if (gen.num == 4)
        file = QString("heartgold-soulsilver/%2%4%3%1.png").arg(pokeid.toString(), back?"back/":"", (gender==Pokemon::Female)?"female/":"", shiney?"shiny/":"");
    else
        file = QString("black-white/%2%4%3%1.png").arg(pokeid.toString(), back?"back/":"", (gender==Pokemon::Female)?"female/":"", shiney?"shiny/":"");

    QPixmap ret;

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
    {
        if (mod) {
            return PokemonInfo::Picture(pokeid, gen, gender,shiney,back,false);
        }
        if (gen.num == 1) {
            return PokemonInfo::Picture(pokeid, 2, gender, shiney, back);
        } else if (gen.num == 2) {
            if (shiney)
                return PokemonInfo::Picture(pokeid, 2, gender, false, back);
            else
                return PokemonInfo::Picture(pokeid, 3, gender, shiney, back);
        } else if (gen.num == 3) {
            if (shiney)
                return PokemonInfo::Picture(pokeid, 3, gender, false, back);
            else
                return PokemonInfo::Picture(pokeid, 4, gender, shiney, back);
        } else if (gen.num == 4 && gender == Pokemon::Female) {
            return PokemonInfo::Picture(pokeid, 4, Pokemon::Male, shiney, back);
        } else if (gen.num == 4 && shiney) {
            return PokemonInfo::Picture(pokeid, 4, Pokemon::Male, false, back);
        } else if (gen.num == 5) {
            if (gender == Pokemon::Female) {
                return PokemonInfo::Picture(pokeid, 5, Pokemon::Male, shiney, back);
            } else if (shiney) {
                return PokemonInfo::Picture(pokeid, 5, Pokemon::Male, false, back);
            } else if (pokeid.subnum != 0) {
                return PokemonInfo::Picture(OriginalForme(pokeid), 5, Pokemon::Male, false, back);
            }
        }
        return ret;
    }

    ret.loadFromData(data, file.section(".", -1).toAscii().data());

    return ret;
}

QMovie *PokemonInfo::AnimatedSprite(const Pokemon::uniqueId &pokeId, int gender, bool shiny, bool back)
{
    QString archive = path("black_white_animated.zip");
    QString file = QString("%1/%2%3%4.gif").arg(pokeId.toString(), back?"b":"f", (gender == Pokemon::Female)?"f":"", shiny?"s":"");
    QByteArray *data = new QByteArray(readZipFile(archive.toUtf8(), file.toUtf8()));
    QBuffer *animatedSpriteData = new QBuffer(data);
    QMovie *AnimatedSprite = new QMovie(animatedSpriteData);
    if(data->length() == 0) {
        if(gender == Pokemon::Female) {
            return PokemonInfo::AnimatedSprite(pokeId, Pokemon::Male, shiny, back);
        }
        if(shiny) {
            return PokemonInfo::AnimatedSprite(pokeId, Pokemon::Male, false, back);
        }
        if(pokeId.subnum != 0) {
            return PokemonInfo::AnimatedSprite(OriginalForme(pokeId), Pokemon::Male, false, back);
        }
        AnimatedSprite->start();
        return AnimatedSprite;
    }
    AnimatedSprite->start();
    return AnimatedSprite;
}

bool PokemonInfo::HasAnimatedSprites()
{
    QFile file(path("black_white_animated.zip"));
    if(file.exists()) {
        return true;
    }
    return false;
}

bool PokemonInfo::HasAnimatedSpritesEnabled()
{
    QSettings MySettings;
    return MySettings.value("animated_sprites").toBool();
}

QPixmap PokemonInfo::Sub(Pokemon::gen gen, bool back)
{
    QString archive;
    if (gen <= 3)
        archive = path("advance.zip");
    else if (gen == 4)
        archive = path("generation-4.zip");
    else
        archive = path("black-white.zip");

    QString file;

    if (gen <= 3) {
        file = QString("advance/%1substitute.png").arg(back?"back/":"");
    } else if (gen <= 4) {
        file = QString("heartgold-soulsilver/%1substitute.png").arg(back?"back/":"");
    } else {
        file = QString("black-white/%1substitute.png").arg(back?"back/":"");
    }

    QPixmap ret;

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
        return ret;


    ret.loadFromData(data, "png");

    return ret;
}

QPixmap PokemonInfo::Icon(const Pokemon::uniqueId &pokeid, bool mod)
{
    QString archive = path("icons.zip");

    if (mod && modPath.length() > 0 && QFile::exists(modPath + archive)) {
        archive.prepend(modPath);
    } else {
        mod = false;
    }

    QString file = QString("%1.png").arg(pokeid.toString());

    QPixmap p;

    if (QPixmapCache::find(archive+file, &p)) {
        return p;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        if (mod) {
            return Icon(pokeid, false);
        }

        if (IsForme(pokeid)) {
            return Icon(OriginalForme(pokeid));
        }

        qDebug() << "error loading icon";
        return p;
    }

    p.loadFromData(data,"png");
    return p;
}

QByteArray PokemonInfo::Cry(const Pokemon::uniqueId &pokeid, bool mod)
{
    quint16 num = pokeid.pokenum;
    QString archive = "cries.zip";

    // TODO: Read this number from somewhere else.
    if (mod && modPath.length() > 0 && QFile::exists(modPath+archive)) {
        archive.prepend(modPath);
    } else {
        mod = false;
    }

    QString file = QString("%1.wav").arg(num).rightJustified(7, '0');

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        if (mod) {
            return Cry(pokeid, false);
        }
        qDebug() << "error loading pokemon cry " << num;
    }

    return data;
}

QSet<int> PokemonInfo::Moves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).genMoves[gen.num-GEN_MIN];
}

bool PokemonInfo::HasMoveInGen(const Pokemon::uniqueId &pokeid, int move, Pokemon::gen gen)
{
    return m_Moves[pokeid].regularMoves[gen.num-GEN_MIN].contains(move) || m_Moves[pokeid].specialMoves[gen.num-GEN_MIN].contains(move)
            || m_Moves[pokeid].eggMoves[gen.num-GEN_MIN].contains(move) || m_Moves[pokeid].preEvoMoves[gen.num-GEN_MIN].contains(move);
}

QSet<int> PokemonInfo::RegularMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).regularMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::EggMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).eggMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::LevelMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).levelMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::TutorMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).tutorMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::TMMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).TMMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::SpecialMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).specialMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::PreEvoMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_Moves.value(pokeid).preEvoMoves[gen.num-GEN_MIN];
}

QSet<int> PokemonInfo::dreamWorldMoves(const Pokemon::uniqueId &pokeid)
{
    return m_Moves.value(pokeid).dreamWorldMoves;
}

AbilityGroup PokemonInfo::Abilities(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    AbilityGroup ret;

    for (int i = 0; i < 3; i++) {
        ret._ab[i] = m_Abilities[gen.num-GEN_MIN][i].value(pokeid);
    }

    return ret;
}

int PokemonInfo::Ability(const Pokemon::uniqueId &pokeid, int slot, Pokemon::gen gen)
{
    return m_Abilities[gen.num-GEN_MIN][slot].value(pokeid);
}

void PokemonInfo::loadBaseStats()
{
    m_BaseStats.clear();

    QHash<Pokemon::uniqueId, QString> temp;
    fill_uid_str(temp, path("poke_stats.txt"));

    QHashIterator<Pokemon::uniqueId, QString> it(temp);

    while (it.hasNext()) {
        it.next();
        QString text_stats = it.value();
        QTextStream statsstream(&text_stats, QIODevice::ReadOnly);

        int hp, att, def, spd, satt, sdef;
        statsstream >> hp >> att >> def >> spd >> satt >> sdef;
        m_BaseStats[it.key()] = PokeBaseStats(hp, att, def, spd, satt, sdef);
    }
}

PokeBaseStats PokemonInfo::BaseStats(const Pokemon::uniqueId &pokeid)
{
    return m_BaseStats.value(pokeid);
}

int PokemonInfo::SpecialStat(const Pokemon::uniqueId &pokeid)
{
    if (!Exists(pokeid, 1)) {
        return 0;
    }
    return m_SpecialStats[pokeid.pokenum];
}

void PokemonInfo::loadNames()
{
    /* The need for options prevents us from using fill_uid_str */
    QStringList temp;
    fill_container_with_file(temp, path("pokemons.txt"));

    m_Names.clear();
    m_Options.clear();
    m_MaxForme.clear();

    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString name;
        QString options;
        Pokemon::uniqueId id;
        bool ok = Pokemon::uniqueId::extract(current, id, name, &options);
        if(ok) {
            m_Names[id] = name;
            m_Options[id] = options;

            // Calculate a number of formes a given base pokemon have.
            quint16 max_forme = m_MaxForme.value(id.pokenum, 0);
            if(max_forme < id.subnum){
                max_forme = id.subnum;
            }
            m_MaxForme[id.pokenum] = max_forme;
        }
    }
}

bool PokemonInfo::HasFormes(const Pokemon::uniqueId &pokeid)
{
    return NumberOfAFormes(pokeid) > 0;
}

bool PokemonInfo::AFormesShown(const Pokemon::uniqueId &pokeid)
{
    return !m_Options.value(pokeid.pokenum).contains('H');
}

quint16 PokemonInfo::NumberOfAFormes(const Pokemon::uniqueId &pokeid)
{
    return m_MaxForme.value(pokeid.pokenum, 0);
}

bool PokemonInfo::IsForme(const Pokemon::uniqueId &pokeid)
{
    return pokeid.subnum != 0;
}

Pokemon::uniqueId PokemonInfo::OriginalForme(const Pokemon::uniqueId &pokeid)
{
    return Pokemon::uniqueId(pokeid.pokenum, 0);
}

QList<Pokemon::uniqueId> PokemonInfo::Formes(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    QList<Pokemon::uniqueId> result;
    for(quint16 i = 0; i <= NumberOfAFormes(pokeid); i++) {
        if (Exists(Pokemon::uniqueId(pokeid.pokenum, i), gen))
            result.append(Pokemon::uniqueId(pokeid.pokenum, i));
    }
    return result;
}

QList<Pokemon::uniqueId> PokemonInfo::VisibleFormes(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    QList<Pokemon::uniqueId> result;
    for(quint16 i = 0; i <= NumberOfAFormes(pokeid); i++) {
        Pokemon::uniqueId poke(pokeid.pokenum, i);
        if(Exists(poke, gen) && AFormesShown(poke)) result.append(poke);
    }
    return result;
}

int PokemonInfo::MinLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    int g = gen.num-GEN_MIN;

    if (!m_MinLevels[g].contains(pokeid))
        return 100;

    return m_MinLevels[g][pokeid];
}

int PokemonInfo::MinEggLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    int g = gen.num-GEN_MIN;

    if (!m_MinLevels[g].contains(pokeid))
        return 100;

    return m_MinLevels[g][pokeid];
}

int PokemonInfo::AbsoluteMinLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    int limit = (gen >= 3 ? 3 : GEN_MIN);

    int min = 100;
    for (int g = gen.num; g >= limit; g--) {
        int level = MinLevel(pokeid, g);

        if (level < min) {
            min = level;
        }
    }

    return min;
}

Pokemon::uniqueId PokemonInfo::OriginalEvo(const Pokemon::uniqueId &pokeid)
{
    return Pokemon::uniqueId(m_OriginalEvos.value(pokeid.pokenum), 0);
}

int PokemonInfo::PreEvo(int pokenum)
{
    return m_PreEvos.value(pokenum);
}

bool PokemonInfo::HasPreEvo(int pokenum)
{
    return m_PreEvos.contains(pokenum);
}


QList<int> PokemonInfo::Evos(int pokenum)
{
    return m_Evolutions.value(OriginalEvo(Pokemon::uniqueId(pokenum, 0)).pokenum);
}

QList<int> PokemonInfo::DirectEvos(int pokenum)
{
    return m_DirectEvos.value(pokenum);
}

bool PokemonInfo::HasEvolutions(int pokenum)
{
    return m_DirectEvos.contains(pokenum);
}

bool PokemonInfo::IsInEvoChain(const Pokemon::uniqueId &pokeid)
{
    return Evos(pokeid.pokenum).size() > 1;
}

void PokemonInfo::loadMoves()
{
    static const int filesize = 29;

    QString fileNames[filesize] = {
        path("1G_tm_and_hm_moves.txt"), path("1G_level_moves.txt"),
        path("1G_special_moves.txt"), path("1G_pre_evo_moves.txt"),

        path("2G_tm_and_hm_moves.txt"), path("2G_egg_moves.txt"), path("2G_level_moves.txt"),
        path("2G_tutor_moves.txt"), path("2G_special_moves.txt"), path("2G_pre_evo_moves.txt"),

        path("3G_tm_and_hm_moves.txt"), path("3G_egg_moves.txt"), path("3G_level_moves.txt"),
        path("3G_tutor_moves.txt"), path("3G_special_moves.txt"), path("3G_pre_evo_moves.txt"),

        path("4G_tm_and_hm_moves.txt"), path("4G_pre_evo_moves.txt"), path("4G_egg_moves.txt"),
        path("4G_level_moves.txt"), path("4G_tutor_moves.txt"), path("4G_special_moves.txt"),

        path("5G_tm_and_hm_moves.txt"), path("5G_pre_evo_moves.txt"), path("5G_egg_moves.txt"),
        path("5G_level_moves.txt"), path("5G_tutor_moves.txt"), path("5G_special_moves.txt"),
        path("5G_dw_moves.txt")
    };

    m_Moves.clear();

    for (int i = 0; i < filesize; i++) {
        QHash<Pokemon::uniqueId, QString> temp;
        fill_uid_str(temp, fileNames[i]);

        QHashIterator<Pokemon::uniqueId, QString> it(temp);

        while(it.hasNext()) {
            it.next();

            QStringList move_list = it.value().split(' ');

            QSet<int> data_set;
            for(int ml_counter = 0; ml_counter < move_list.size(); ml_counter++) {
                int move = move_list[ml_counter].toInt();
                if(move != 0)
                    data_set.insert(move);
            }
            /* Should create an item with pokeid key in m_Moves if it does not exist. */
            PokemonMoves &moves = m_Moves[it.key()];

            QSet<int> *refs[filesize] = {
                &moves.TMMoves[0], &moves.levelMoves[0], &moves.specialMoves[0], &moves.preEvoMoves[0],
                &moves.TMMoves[1], &moves.eggMoves[1], &moves.levelMoves[1], &moves.tutorMoves[1], &moves.specialMoves[1], &moves.preEvoMoves[1],
                &moves.TMMoves[2], &moves.eggMoves[2], &moves.levelMoves[2], &moves.tutorMoves[2], &moves.specialMoves[2], &moves.preEvoMoves[2],
                &moves.TMMoves[3], &moves.preEvoMoves[3], &moves.eggMoves[3], &moves.levelMoves[3], &moves.tutorMoves[3], &moves.specialMoves[3],
                &moves.TMMoves[4], &moves.preEvoMoves[4], &moves.eggMoves[4], &moves.levelMoves[4], &moves.tutorMoves[4], &moves.specialMoves[4],
                &moves.dreamWorldMoves
            };

            *refs[i] = data_set;
        }
    }

    QHashIterator<Pokemon::uniqueId, PokemonMoves> it(m_Moves);
    while(it.hasNext()) {
        it.next();
        PokemonMoves moves = it.value();

        for (int i = 0; i < NUMBER_GENS; i++) {
            moves.regularMoves[i] = moves.TMMoves[i];
            moves.regularMoves[i].unite(moves.levelMoves[i]).unite(moves.tutorMoves[i]);
            moves.genMoves[i] = moves.regularMoves[i];
            moves.genMoves[i].unite(moves.specialMoves[i]).unite(moves.eggMoves[i]).unite(moves.preEvoMoves[i]);

            if (i == 5 - GEN_MIN) {
                moves.genMoves[i].unite(moves.dreamWorldMoves);
            }

            if (i > 0 && i+GEN_MIN != 3) {
                moves.genMoves[i].unite(moves.genMoves[i-1]);
            }
        }

        m_Moves[it.key()] = moves;
    }
}

QString PokemonInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

QList<Pokemon::uniqueId> PokemonInfo::AllIds()
{
    return m_Names.keys();
}

void PokemonInfo::loadEvos()
{
    QHash<int, QList<int> > &evos = m_Evolutions;

    foreach(QByteArray s, getFileContent(path("evos.txt")).split('\n')) {
        QList<QByteArray> evs = s.split(' ');
        int num = evs[0].toInt();

        /* It's normal to start from 0 */
        foreach(QByteArray ev, evs) {
            int n = ev.toInt();

            if (n != num)
                m_PreEvos[n] = num;

            evos[num].push_back(n);
        }
    }

    m_DirectEvos = evos;

    foreach(int key, m_DirectEvos.keys()) {
        m_DirectEvos[key].removeOne(key);
    }

    QHash<int, QList<int> > copy = evos;

    QHashIterator<int, QList<int> > it(copy);

    while (it.hasNext()) {
        it.next();

        if (!evos.contains(it.key()))
            continue;

        QList<int> res;

        foreach(int ev, evos[it.key()]) {
            res.push_back(ev);

            if (ev != it.key() && evos.contains(ev)) {
                foreach(int ev2, evos[ev]) {
                    if (ev2 != ev)
                        res.push_back(ev2);
                }

                evos.remove(ev);
            }
        }

        evos[it.key()] = res;

        foreach(int x, res) {
            m_OriginalEvos[x] = it.key();
        }
    }
}

void PokemonInfo::makeDataConsistent()
{
    // Count base forms. We no longer need to save it in a file.
    m_trueNumberOfPokes = 0;
    // Also adds data to pokemon that do not have data set explicitely (some formes).

    foreach (Pokemon::uniqueId id, AllIds()) {
        if(id.subnum == 0) {
            // Count base forms.
            m_trueNumberOfPokes++;
            // Original evolutions.
            if(!m_OriginalEvos.contains(id.pokenum)) {
                m_OriginalEvos[id.pokenum] = id.pokenum;
                // m_Evolutions initial filler data.
                m_Evolutions[id.pokenum] = QList<int>() << id.pokenum;
            }
        }
        // Weight
        if(!m_Weights.contains(id)) {
            m_Weights[id] = m_Weights.value(OriginalForme(id), "0.0");
        }
        // Base stats.
        if(!m_BaseStats.contains(id)) {
            m_BaseStats[id] = m_BaseStats.value(OriginalForme(id), PokeBaseStats());
            if (id != OriginalForme(id))
                m_AestheticFormes.insert(id);
        }
        // Moves.
        if(!m_Moves.contains(id)) {
            m_Moves[id] = m_Moves.value(OriginalForme(id));
        }
        // Other.
        if(!m_LevelBalance.contains(id)) {
            m_LevelBalance[id] = m_LevelBalance.value(OriginalForme(id), 1);
        }
        if(!m_Genders.contains(id)) {
            m_Genders[id] = m_Genders.value(OriginalForme(id), Pokemon::NeutralAvail);
        }

        for (int gen = GEN_MIN; gen <= GEN_MAX; gen++) {
            int i = gen-GEN_MIN;

            if (!Exists(id, gen))
                continue;

            for (int j = 0; j < 3; j++) {
                if(!m_Abilities[i][j].contains(id)) {
                    m_Abilities[i][j][id] = m_Abilities[i][j].value(OriginalForme(id), Ability::NoAbility);
                }
            }

            if(!m_Type1[i].contains(id)) {
                m_Type1[i][id] = m_Type1[i].value(OriginalForme(id), Pokemon::Normal);
            }
            if(!m_Type2[i].contains(id)) {
                m_Type2[i][id] = m_Type2[i].value(OriginalForme(id), Pokemon::Curse);
            }
            if (!m_MinLevels[i].contains(id)) {
                m_MinLevels[i][id] = m_MinLevels[i].value(OriginalForme(id), 100);
                m_MinEggLevels[i][id] = m_MinEggLevels[i].value(OriginalForme(id), 100);
            }
        }
    }
}

Pokemon::uniqueId PokemonInfo::getRandomPokemon(Pokemon::gen gen)
{
    int total = TrueCount(gen);
    int random = true_rand() % total;
    if((random == 0) && (total > 1)) random = 1;
    Pokemon::uniqueId poke(random, 0);
    if(HasFormes(poke)) {
        QList<Pokemon::uniqueId> formesList = VisibleFormes(poke, gen);
        /* The pokemon doesn't always have visible formes */
        if (formesList.count() > 0)
            poke = formesList.value(true_rand() %  formesList.count());
    }
    return Pokemon::uniqueId(poke);
}

bool PokemonInfo::modifyAbility(const Pokemon::uniqueId &pokeid, int slot, int ability, Pokemon::gen gen)
{
    if ((slot >= 0) && (slot <= 2) && Exists(pokeid, gen)) {
        m_Abilities[gen.num - GEN_MIN][slot][pokeid] = ability;
        return true;
    }else{
        return false;
    }
}


void MoveInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfMoves(GEN_MAX) != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadMoveMessages();
    loadDetails();
    loadSpecialEffects();

    fill_double(m_OldMoves, path("oldmoves.txt"));
    fill_int_bool(m_KingRock, path("king_rock.txt"));

    for (int i = 0; i < Version::NumberOfGens; i++) {
        gens[i].load(dir, i+1);
    }
}

void MoveInfo::Gen::load(const QString &dir, int gen)
{
    this->gen = gen;
    this->dir = QString("%1%2G/").arg(dir).arg(gen);

    fill_int_char(accuracy, path("accuracy.txt"));
    fill_int_char(category, path("category.txt"));
    fill_int_char(causedEffect, path("caused_effect.txt"));
    fill_int_char(critRate, path("crit_rate.txt"));
    fill_int_char(damageClass, path("damage_class.txt"));
    fill_int_str(effect, path("effect.txt"), true);
    fill_int_char(effectChance, path("effect_chance.txt"));
    fill_double(flags, path("flags.txt"));
    fill_int_char(flinchChance, path("flinch_chance.txt"));
    fill_int_char(healing, path("healing.txt"));
    fill_int_char(maxTurns, path("max_turns.txt"));
    fill_int_char(minTurns, path("min_turns.txt"));
    fill_int_char(minMaxHits, path("min_max_hits.txt"));
    fill_double(stataffected, path("stataffected.txt"));
    fill_double(statboost, path("statboost.txt"));
    fill_double(statrate, path("statrate.txt"));
    fill_int_char(power, path("power.txt"));
    fill_int_char(pp, path("pp.txt"));
    fill_int_char(priority, path("priority.txt"));
    fill_int_char(range, path("range.txt"));
    fill_int_char(recoil, path("recoil.txt"));
    fill_int_char(status, path("status.txt"));
    fill_int_char(type, path("type.txt"));

    /* Not needed because HM pokemon can be traded between gens got gen 1 & 2*/
    //    if (gen == 1) {
    //        HMs << Move::Cut << Move::Flash << Move::Surf << Move::Strength <<Move::Fly;
    //    } else if (gen == 2) {
    //        HMs << Move::Cut << Move::Flash << Move::Surf << Move::Strength << Move::Whirlpool
    //                        << Move::Waterfall << Move::Fly;
    //    }

    if (gen == 3) {
        HMs << Move::Cut << Move::Flash << Move::Surf << Move::RockSmash << Move::Strength << Move::Dive
            << Move::Waterfall << Move::Fly;
    } else if (gen == 4) {
        HMs << Move::Cut << Move::Surf << Move::RockSmash << Move::Strength
            << Move::Waterfall << Move::Fly << Move::RockClimb;
    } else if (gen == 5) {
        HMs << Move::Cut << Move::Surf << Move::Dive << Move::Waterfall << Move::Fly << Move::Strength;
    }
}

void MoveInfo::Gen::retranslate()
{
    fill_int_str(effect, path("effect.txt"), true);
}

QString MoveInfo::Gen::path(const QString &fileName)
{
    return dir + fileName;
}

template <class T>
static void loadMessages(const QString &path, T &container)
{
    container.clear();

    QHash<int, QString> temp;
    fill_int_str(temp, path, true);

    QHashIterator<int, QString> it(temp);
    while (it.hasNext()) {
        it.next();
        if (it.value().length() > 0) {
            QStringList split = it.value().split('|');
            for (int j = 0; j < split.length(); j++) {
                if (container[it.key()].length() > j) {
                    container[it.key()][j] = split[j];
                } else {
                    container[it.key()].push_back(split[j]);
                }
            }
        }
    }
}

void MoveInfo::loadMoveMessages()
{
    loadMessages(path("move_message.txt"), m_MoveMessages);
}

void MoveInfo::retranslate()
{
    loadNames();
    loadMoveMessages();
    loadDetails();

    for (int i = 0; i < Version::NumberOfGens; i++) {
        gens[i].retranslate();
    }
}

void MoveInfo::loadNames()
{
    fill_int_str(m_Names, path("moves.txt"), true);

    QHashIterator<int, QString> it(m_Names);

    while(it.hasNext()) {
        it.next();
        m_LowerCaseMoves.insert(it.value().toLower(),it.key());
    }
}

void MoveInfo::loadDetails()
{
    fill_int_str(m_Details, path("move_description.txt"), true);
}

void MoveInfo::loadSpecialEffects()
{
    {
        fill_int_str(m_SpecialEffects, path("move_special_effects.txt"));

        /* Removing comments, aka anything starting from '#' */
        QMutableHashIterator<int,QString> it(m_SpecialEffects);
        while(it.hasNext()) {
            it.next();
            it.value() = it.value().section('#', 0, 0);
        }
    }

    {
        fill_int_str(m_RbySpecialEffects, path("move_special_effects_rby.txt"));

        /* Removing comments, aka anything starting from '#' */
        QMutableHashIterator<int,QString> it(m_RbySpecialEffects);
        while(it.hasNext()) {
            it.next();
            it.value() = it.value().section('#', 0, 0);
        }
    }
}

QString MoveInfo::Name(int movenum)
{
    return Exists(movenum, GEN_MAX) ? m_Names[movenum] : m_Names[0];
}

int MoveInfo::Type(int movenum, Pokemon::gen g)
{
    return gen(g).type[movenum];
}

int MoveInfo::ConvertFromOldMove(int oldmovenum)
{
    return m_OldMoves[oldmovenum];
}

int MoveInfo::Category(int movenum, Pokemon::gen g)
{
    if (g >= 4)
        return gen(g).damageClass[movenum];

    if (Power(movenum, g) == 0)
        return Move::Other;

    return TypeInfo::Category(Type(movenum, g));
}

int MoveInfo::Classification(int movenum, Pokemon::gen g)
{
    return gen(g).category[movenum];
}

bool MoveInfo::FlinchByKingRock(int movenum, Pokemon::gen gen)
{
    if (gen >= 5 && movenum == Move::BeatUp) {
        return true;
    }
    return m_KingRock[movenum];
}

int MoveInfo::Number(const QString &movename)
{
    return m_LowerCaseMoves.value(movename.toLower());
}

int MoveInfo::NumberOfMoves(Pokemon::gen g)
{
    return gen(g).power.count();
}

int MoveInfo::FlinchRate(int num, Pokemon::gen g)
{
    return gen(g).flinchChance[num];
}

int MoveInfo::Recoil(int movenum, Pokemon::gen g)
{
    return gen(g).recoil[movenum];
}

QString MoveInfo::Description(int movenum, Pokemon::gen g)
{
    QString r = gen(g).effect[movenum];
    r.replace("$effect_chance", QString::number(EffectRate(movenum, g)));

    return r;
}

int MoveInfo::Power(int movenum, Pokemon::gen g)
{
    return gen(g).power[movenum];
}

QString MoveInfo::PowerS(int movenum, Pokemon::gen gen)
{
    int p = Power(movenum, gen);

    if (p == 0)
        return "--";
    else if (p == 1)
        return "???";
    else
        return QString::number(p);
}

int MoveInfo::PP(int movenum, Pokemon::gen g)
{
    return gen(g).pp[movenum];
}

int MoveInfo::Acc(int movenum, Pokemon::gen g)
{
    return gen(g).accuracy[movenum];
}

QString MoveInfo::AccS(int movenum, Pokemon::gen gen)
{
    int acc = MoveInfo::Acc(movenum, gen);

    if (acc == 101)
        return "--";
    else
        return QString::number(acc);
}

int MoveInfo::CriticalRaise(int movenum, Pokemon::gen g)
{
    return gen(g).critRate[movenum];
}

int MoveInfo::RepeatMin(int movenum, Pokemon::gen g)
{
    return gen(g).minMaxHits[movenum] & 0xF;
}

int MoveInfo::RepeatMax(int movenum, Pokemon::gen g)
{
    return gen(g).minMaxHits[movenum] >> 4;
}

int MoveInfo::SpeedPriority(int movenum, Pokemon::gen g)
{
    return gen(g).priority[movenum];
}

int MoveInfo::Flags(int movenum, Pokemon::gen g)
{
    return gen(g).flags[movenum];
}

bool MoveInfo::Exists(int movenum, Pokemon::gen g)
{
    return gen(g).power.size() > movenum;
}

bool MoveInfo::isOHKO(int movenum, Pokemon::gen gen)
{
    return Classification(movenum, gen) == Move::OHKOMove;
}

bool MoveInfo::isHM(int movenum, Pokemon::gen g)
{
    return gen(g).HMs.contains(movenum);
}

int MoveInfo::EffectRate(int movenum, Pokemon::gen g)
{
    return gen(g).effectChance[movenum];
}

quint32 MoveInfo::StatAffected(int movenum, Pokemon::gen g)
{
    return gen(g).stataffected[movenum];
}

quint32 MoveInfo::BoostOfStat(int movenum, Pokemon::gen g)
{
    return gen(g).statboost[movenum];
}

quint32 MoveInfo::RateOfStat(int movenum, Pokemon::gen g)
{
    return gen(g).statrate[movenum];
}

int MoveInfo::Target(int movenum, Pokemon::gen g)
{
    return gen(g).range[movenum];
}

int MoveInfo::Healing(int movenum, Pokemon::gen g)
{
    return gen(g).healing[movenum];
}

int MoveInfo::MinTurns(int movenum, Pokemon::gen g)
{
    return gen(g).minTurns[movenum];
}

int MoveInfo::MaxTurns(int movenum, Pokemon::gen g)
{
    return gen(g).maxTurns[movenum];
}

int MoveInfo::Status(int movenum, Pokemon::gen g)
{
    return gen(g).causedEffect[movenum];
}

int MoveInfo::StatusKind(int movenum, Pokemon::gen g)
{
    return gen(g).status[movenum];
}

QString MoveInfo::MoveMessage(int moveeffect, int part)
{
    if (!m_MoveMessages.contains(moveeffect) || part < 0 || part >= m_MoveMessages[moveeffect].size()) {
        return "";
    }
    return m_MoveMessages[moveeffect][part];
}

QString MoveInfo::SpecialEffect(int movenum, Pokemon::gen gen)
{
    return gen <= 1 ? m_RbySpecialEffects.value(movenum) : m_SpecialEffects.value(movenum);
}

QString MoveInfo::DetailedDescription(int movenum)
{
    return m_Details[movenum];
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}

///////////////////////////////////////////////////////
/////////////// ITEMINFO //////////////////////////////
///////////////////////////////////////////////////////

void ItemInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfItems() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadGenData();
    loadNames();
    loadEffects();
    loadFlingData();
    loadMessages();
}

void ItemInfo::retranslate()
{
    loadNames();
    loadMessages();
}

void ItemInfo::loadGenData()
{
    fill_container_with_file(m_UsefulItems, path("item_useful.txt"));

    for (int g = GEN_MIN_ITEMS; g <= GEN_MAX; g++) {
        int i = g-GEN_MIN;
        fill_container_with_file(m_GenItems[i], path(QString("items_gen%1.txt").arg(g)));

        QList<int> tempb;
        fill_container_with_file(tempb, path(QString("berries_gen%1.txt").arg(g)));
        foreach(int b, tempb) {
            m_GenItems[i].insert(b+8000);
        }
    }
}

void ItemInfo::loadNames()
{
    m_BerryNamesH.clear();

    fill_int_str(m_RegItemNames, path("items.txt"), true);
    m_ItemNamesH = reverse_hash(m_RegItemNames);

    fill_int_str(m_BerryNames, path("berries.txt"), true);
    m_BerryNamesH.reserve(m_BerryNames.size());

    QHash<int, QString>::const_iterator it2 = m_BerryNames.constBegin();
    for (int i = 0; it2 != m_BerryNames.constEnd(); i++, ++it2) {
        m_BerryNamesH[*it2] = i+8000;
    }

    /* clears in case of retranslate */
    for (int i = 0; i < NUMBER_GENS; i++) {
        m_SortedNames[i].clear();
        m_SortedUsefulNames[i].clear();
    }

    int mg = GEN_MAX - GEN_MIN;

    m_SortedNames[mg] << m_RegItemNames.values() << m_BerryNames.values();
    qSort(m_SortedNames[mg]);

    m_SortedUsefulNames[mg] << m_BerryNames.values();
    for (int i = 0; i < m_RegItemNames.size(); i++) {
        if (isUseful(i))
            m_SortedUsefulNames[mg].push_back(m_RegItemNames[i]);
    }
    qSort(m_SortedUsefulNames[mg]);

    for (int j = GEN_MAX-1; j >= GEN_MIN_ITEMS; j--) {
        int g = j-GEN_MIN;
        for (int i = 0; i < m_SortedNames[g+1].size(); i++) {
            if (Exists(Number(m_SortedNames[g+1][i]), j))
                m_SortedNames[g].push_back(m_SortedNames[g+1][i]);
        }

        for (int i = 0; i < m_SortedUsefulNames[g+1].size(); i++) {
            if (Exists(Number(m_SortedUsefulNames[g+1][i]), j))
                m_SortedUsefulNames[g].push_back(m_SortedUsefulNames[g+1][i]);
        }

        if (j == 2) {
            m_SortedNames[g].push_back(ItemInfo::Name(Item::BerserkGene));
            m_SortedUsefulNames[g].push_back(ItemInfo::Name(Item::BerserkGene));
            qSort(m_SortedNames[g]);
            qSort(m_SortedUsefulNames[g]);
        }
    }
}

void ItemInfo::loadMessages()
{
    ::loadMessages(path("item_messages.txt"), m_RegMessages);
    ::loadMessages(path("berry_messages.txt"), m_BerryMessages);
}

void ItemInfo::loadFlingData()
{
    fill_double(m_BerryPowers, path("berry_pow.txt"));
    fill_double(m_BerryTypes, path("berry_type.txt"));
    fill_double(m_Powers, path("items_pow.txt"));
}

void ItemInfo::loadEffects()
{
    for (int i = GEN_MIN_ITEMS; i <= GEN_MAX; i++) {
        m_RegEffects[i-GEN_MIN].clear();

        QHash<int,QString> temp;
        fill_int_str(temp, path("item_effects_%1G.txt").arg(i));

        /* Removing comments, aka anything starting from '#' */
        QHashIterator<int,QString> it(temp);
        while(it.hasNext()) {
            it.next();
            QStringList effects = it.value().split('#').front().split('|');
            QList<Effect> toPush;
            foreach(QString eff, effects) {
                std::string s = eff.toStdString();
                size_t pos = s.find('-');
                if (pos != std::string::npos) {
                    toPush.push_back(Effect(atoi(s.c_str()), eff.mid(pos+1)));
                } else {
                    toPush.push_back(Effect(atoi(s.c_str())));
                }
            }
            m_RegEffects[i-GEN_MIN][it.key()] = toPush;
        }
    }

    m_BerryEffects.clear();

    QHash<int,QString> temp;
    fill_int_str(temp, path("berry_effects.txt"));

    /* Removing comments, aka anything starting from '#' */
    QHashIterator<int,QString> it(temp);
    while(it.hasNext()) {
        it.next();
        QStringList effects = it.value().split('#').front().split('|');
        QList<Effect> toPush;
        foreach(QString eff, effects) {
            std::string s = eff.toStdString();
            size_t pos = s.find('-');
            if (pos != std::string::npos) {
                toPush.push_back(Effect(atoi(s.c_str()), eff.mid(pos+1)));
            } else {
                toPush.push_back(Effect(atoi(s.c_str())));
            }
        }
        m_BerryEffects[it.key()] = toPush;
    }
}

QList<ItemInfo::Effect> ItemInfo::Effects(int item, Pokemon::gen gen)
{
    if (!Exists(item, gen)) {
        return QList<ItemInfo::Effect>();
    } else {
        return isBerry(item) ? m_BerryEffects.value(item-8000) : m_RegEffects[gen.num-GEN_MIN].value(item);
    }
}

QString ItemInfo::Message(int effect, int part)
{
    if (effect < 8000) {
        if (!m_RegMessages.contains(effect) || m_RegMessages[effect].size() <= part) {
            return QString();
        }
        return m_RegMessages[effect][part];
    } else {
        effect = effect-8000;
        if (!m_BerryMessages.contains(effect) || m_BerryMessages[effect].size() <= part) {
            return QString();
        }
        return m_BerryMessages[effect][part];
    }
}

QString ItemInfo::path(const QString &file)
{
    return m_Directory + file;
}

int ItemInfo::NumberOfItems()
{
    return m_SortedNames[GEN_MAX-GEN_MIN].size();
}

int ItemInfo::Power(int itemnum) {
    if (isBerry(itemnum)) {
        return 10;
    } else if (Exists(itemnum)) {
        return m_Powers[itemnum];
    } else return 0;
}

int ItemInfo::BerryPower(int itemnum)
{
    if (!isBerry(itemnum) || !Exists(itemnum)) {
        return 0;
    }

    return m_BerryPowers[itemnum-8000];
}

int ItemInfo::BerryType(int itemnum)
{
    if (!isBerry(itemnum) || !Exists(itemnum)) {
        return 0;
    }

    return m_BerryTypes[itemnum-8000];
}

QPixmap ItemInfo::Icon(int itemnum)
{
    QPixmap ret;

    if (itemnum == 0)
        return ret;

    QString archive = path("Items.zip");
    if (isBerry(itemnum)) {
        itemnum -= 7999;
        archive = path("Berries.zip");
    }

    QString file = QString("%1.png").arg(itemnum);

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        qDebug() << "error loading icon";
        return QPixmap();
    }

    ret.loadFromData(data,"png");
    return ret;
}

QPixmap ItemInfo::HeldItem()
{
    QPixmap ret;

    QString archive = path("Items.zip");

    QString file = QString("helditem.png");

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        qDebug() << "error loading held item icon";
        return QPixmap();
    }

    ret.loadFromData(data,"png");
    return ret;
}

QString ItemInfo::Name(int itemnum)
{
    if (itemnum < 8000) {
        return m_RegItemNames.value(itemnum);
    } else {
        return m_BerryNames.value(itemnum-8000);
    }
}

bool ItemInfo::Exists(int itemnum, Pokemon::gen gen)
{
    return m_GenItems[gen.num-GEN_MIN].contains(itemnum);
}

bool ItemInfo::isBerry(int itemnum)
{
    return itemnum >= 8000;
}

bool ItemInfo::isPlate(int itemnum)
{
    return (itemnum >= 185 && itemnum <= 202 && itemnum != 190 && itemnum != 200);
}

bool ItemInfo::isDrive(int itemnum)
{
    return itemnum == Item::DouseDrive || itemnum == Item::BurnDrive || itemnum == Item::ChillDrive || itemnum == Item::ShockDrive;
}

bool ItemInfo::isMail(int itemnum)
{
    return (itemnum >= 214 && itemnum <= 226);
}

bool ItemInfo::isUseful(int itemnum)
{
    return isBerry(itemnum) || m_UsefulItems[itemnum] == true;
}

int ItemInfo::PlateType(int itemnum)
{
    return Effects(itemnum, GEN_MAX).front().args.toInt();
}

int ItemInfo::DriveType(int itemnum)
{
    return Effects(itemnum, GEN_MAX).front().args.toInt();
}

int ItemInfo::DriveForme(int itemnum)
{
    switch(itemnum) {
    case Item::DouseDrive:
        return 1;
    case Item::ShockDrive:
        return 2;
    case Item::BurnDrive:
        return 3;
    case Item::ChillDrive:
        return 4;
    default:
        return 0;
    }
}

int ItemInfo::Number(const QString &itemname)
{
    if (m_BerryNamesH.contains(itemname)) {
        return m_BerryNamesH[itemname];
    } else if (m_ItemNamesH.contains(itemname)) {
        return m_ItemNamesH[itemname];
    } else {
        return 0;
    }
}

int PokemonInfo::Weight(const Pokemon::uniqueId &pokeid) {
    return QString(m_Weights.value(pokeid)).remove('.').toInt();
}

QString PokemonInfo::WeightS(const Pokemon::uniqueId &pokeid)
{
    return m_Weights.value(pokeid);
}

int PokemonInfo::BaseGender(const Pokemon::uniqueId &pokeid)
{
    int avail = Gender(pokeid);

    return (avail == Pokemon::MaleAvail || avail == Pokemon::MaleAndFemaleAvail) ?
                Pokemon::Male : (avail == Pokemon::NeutralAvail ? Pokemon::Neutral : Pokemon::Female);
}

QList<QString> ItemInfo::SortedNames(Pokemon::gen gen)
{
    return m_SortedNames[gen.num-GEN_MIN];
}

QList<QString> ItemInfo::SortedUsefulNames(Pokemon::gen gen)
{
    return m_SortedUsefulNames[gen.num-GEN_MIN];
}

void TypeInfo::init(const QString &dir)
{
    if (NumberOfTypes() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadCategories();
    loadEff();
}

void TypeInfo::retranslate()
{
    loadNames();
}

void TypeInfo::loadNames()
{
    fill_int_str(m_Names, path("types.txt"), true);
}

void TypeInfo::loadCategories()
{
    fill_double(m_Categories, path("category.txt"));
}

QString TypeInfo::path(const QString& file)
{
    return m_Directory+file;
}

void TypeInfo::loadEff()
{
    {
        QHash<int, QString> temp;

        fill_int_str(temp, path("typestable.txt"));

        QHashIterator<int, QString> it(temp);
        while (it.hasNext()) {
            it.next();
            QStringList l2 = it.value().split(' ');
            foreach (QString l3, l2) {
                m_TypeVsType[it.key()].push_back(l3.toInt());
            }
        }
    }

    {
        QHash<int, QString> temp;

        fill_int_str(temp, path("typestable_gen1.txt"));

        QHashIterator<int, QString> it(temp);
        while (it.hasNext()) {
            it.next();
            QStringList l2 = it.value().split(' ');
            foreach (QString l3, l2) {
                m_TypeVsTypeGen1[it.key()].push_back(l3.toInt());
            }
        }
    }
}

int TypeInfo::TypeForWeather(int weather) {
    switch(weather) {
    case Hail: return Type::Ice;
    case Rain: return Type::Water;
    case SandStorm: return Type::Rock;
    case Sunny: return Type::Fire;
    default: return Type::Normal;
    }
}

QString TypeInfo::weatherName(int weather)
{
    // Supposed to be lowercase and unique.
    switch(weather) {
    case Hail: return QObject::tr("hailstorm");
    case Rain: return QObject::tr("rain");
    case SandStorm: return QObject::tr("sandstorm");
    case Sunny: return QObject::tr("sunny");
    default: return QObject::tr("normal", "weather");
    }
}

int TypeInfo::Eff(int type_attack, int type_defend, Pokemon::gen gen)
{
    if (gen.num == 1) {
        return m_TypeVsTypeGen1[type_attack][type_defend];
    } else {
        return m_TypeVsType[type_attack][type_defend];
    }
}


int TypeInfo::Number(const QString &pokename)
{
    return m_Names.key(pokename, 0);
}


QString TypeInfo::Name(int typenum)
{
    return m_Names.value(typenum);
}

int TypeInfo::NumberOfTypes()
{
    return m_Names.size();
}

int TypeInfo::Category(int type)
{
    return m_Categories.value(type);
}

void TypeInfo::modifyTypeChart(int type_attack, int type_defend, int value)
{
    m_TypeVsType[type_attack][type_defend] = value;
}

void NatureInfo::init(const QString &dir)
{
    if (NumberOfNatures() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

void NatureInfo::retranslate()
{
    loadNames();
}

void NatureInfo::loadNames()
{
    fill_int_str(m_Names, path("nature.txt"), true);
}

QString NatureInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

QString NatureInfo::Name(int naturenum)
{
    return m_Names.value(naturenum);
}

int NatureInfo::NumberOfNatures()
{
    return m_Names.size();
}

int NatureInfo::Number(const QString &pokename)
{
    return m_Names.key(pokename, 0);
}

int NatureInfo::NatureOf(int statUp, int statDown)
{
    return (ConvertStat(statUp)-1) * 5 + ConvertStat(statDown)-1;
}

int NatureInfo::Boost(int nature, int stat)
{
    return -(nature%5 == ConvertStat(stat)-1) + (nature/5 == ConvertStat(stat)-1);
}

int NatureInfo::ConvertStat(int stat)
{
    switch (stat) {
    case Hp: return 0;
    case Attack: return 1;
    case Defense: return 2;
    case Speed: return 3;
    case SpAttack: return 4;
    case SpDefense: default: return 5;
    }
}

int NatureInfo::ConvertToStat(int stat)
{
    switch(stat) {
    case 0: return Hp;
    case 1: return Attack;
    case 2: return Defense;
    case 3: return Speed;
    case 4: return SpAttack;
    case 5: default: return SpDefense;
    }
}

int NatureInfo::StatBoosted(int nature)
{
    return Boost(nature, ConvertToStat(nature/5+1)) == 0 ? 0 : ConvertToStat(nature/5+1);
}

int NatureInfo::StatHindered(int nature)
{
    return Boost(nature, ConvertToStat((nature%5)+1)) == 0 ? 0 : ConvertToStat((nature%5)+1);
}

void CategoryInfo::init(const QString &dir)
{
    if (NumberOfCategories() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

void CategoryInfo::retranslate()
{
    loadNames();
}

void CategoryInfo::loadNames()
{
    fill_int_str(m_Names, path("categories.txt"), true);
}

QString CategoryInfo::path(const QString& file)
{
    return m_Directory+file;
}

QString CategoryInfo::Name(int catnum)
{
    return m_Names.value(catnum);
}

int CategoryInfo::NumberOfCategories()
{
    return m_Names.size();
}

void AbilityInfo::init(const QString &dir)
{
    if (NumberOfAbilities(GEN_MAX) != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadMessages();
    loadEffects();

    fill_double(m_OldAbilities, path("oldabilities.txt"));
}

void AbilityInfo::retranslate()
{
    loadNames();
    loadMessages();
}

void AbilityInfo::loadMessages()
{
    ::loadMessages(path("ability_messages.txt"), m_Messages);
}

void AbilityInfo::loadNames()
{
    fill_int_str(m_Names, path("abilities.txt"), true);
    fill_int_str(m_Desc, path("ability_desc.txt"), true);
    fill_int_str(m_BattleDesc, path("ability_battledesc.txt"), true);
}

QString AbilityInfo::Message(int ab, int part) {
    if (!m_Messages.contains(ab) || part < 0 || part >= m_Messages[ab].size()) {
        return QString();
    }

    return m_Messages[ab][part];
}

QString AbilityInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

bool AbilityInfo::Exists(int ability, Pokemon::gen gen)
{
    return gen <= 3 ? ability <= Ability::AirLock : (gen ==4 ? ability <=  Ability::BadDreams : true);
}

void AbilityInfo::loadEffects()
{
    for (int i = GEN_MIN_ABILITIES; i <= GEN_MAX; i++) {
        m_Effects[i-GEN_MIN].clear();

        QHash<int, QString> m_temp;
        fill_int_str(m_temp,path("ability_effects_%1G.txt").arg(i));

        QHashIterator<int, QString> it(m_temp);
        while (it.hasNext()) {
            it.next();
            QStringList content = it.value().section('#', 0, 0).split('-');
            if (content.size() == 1) {
                m_Effects[i-GEN_MIN].insert(it.key(), Effect(content[0].toInt()));
            } else {
                m_Effects[i-GEN_MIN].insert(it.key(), Effect(content[0].toInt(), content[1].toInt()));
            }
        }
    }
}

AbilityInfo::Effect AbilityInfo::Effects(int abnum, Pokemon::gen gen) {
    return m_Effects[gen.num-GEN_MIN].value(abnum);
}

QString AbilityInfo::Desc(int ab)
{
    return m_Desc[ab];
}

QString AbilityInfo::EffectDesc(int abnum)
{
    return m_BattleDesc[abnum];
}

int AbilityInfo::ConvertFromOldAbility(int oldability)
{
    return m_OldAbilities[oldability];
}

int AbilityInfo::Number(const QString &ability)
{
    return m_Names.key(ability, 0);
}

QString AbilityInfo::Name(int abnum)
{
    return m_Names[abnum];
}

int AbilityInfo::NumberOfAbilities(Pokemon::gen g)
{
    int total = m_Names.size();

    if (g == GEN_MAX) {
        return total;
    } else {
        int hc;
        if (g <= 2) {
            hc = 1; //No Ability
        } else if (g <= 3) {
            hc = 77; //Air lock
        } else if (g <= 4 || 1) {
            hc = 125; //Bad dreams
        }
        return std::min(hc, total); //safety check, in case db was edited
    }
}

void GenderInfo::init(const QString &dir)
{
    if (NumberOfGenders() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

void GenderInfo::retranslate()
{
    loadNames();
}

void GenderInfo::loadNames()
{
    fill_int_str(m_Names, path("genders.txt"), true);
}

QString GenderInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

QString GenderInfo::Name(int abnum)
{
    return m_Names.value(abnum);
}

int GenderInfo::Default(int genderAvail) {
    switch(genderAvail) {
    case Pokemon::MaleAndFemaleAvail:
        return Pokemon::Male;
        break;
    default:
        return genderAvail;
    }
}

bool GenderInfo::Possible(int gender, int genderAvail) {
    if (genderAvail == Pokemon::MaleAndFemaleAvail) {
        if(gender == Pokemon::Neutral) {
            return false;
        }
    } else if (gender != genderAvail) {
        return false;
    }
    return true;
}

int GenderInfo::NumberOfGenders()
{
    return m_Names.size();
}

void HiddenPowerInfo::init(const QString &dir)
{
    m_Directory = dir;
}

QString HiddenPowerInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

int HiddenPowerInfo::Type(Pokemon::gen gen, quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 satt_dv, quint8 sdef_dv, quint8 speed_dv)
{
    if (gen >= 3)
        return (((hp_dv%2) + (att_dv%2)*2 + (def_dv%2)*4 + (speed_dv%2)*8 + (satt_dv%2)*16 + (sdef_dv%2)*32)*15)/63 + 1;
    else
        return (att_dv%4)*4+(def_dv%4)+1;
}

int HiddenPowerInfo::Power(Pokemon::gen gen, quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 satt_dv, quint8 sdef_dv, quint8 speed_dv)
{
    if (gen >= 3)
        return (((hp_dv%4>1) + (att_dv%4>1)*2 + (def_dv%4>1)*4 + (speed_dv%4>1)*8 + (satt_dv%4>1)*16 + (sdef_dv%4>1)*32)*40)/63 + 30;
    else
        return (((satt_dv>>3) + (speed_dv>>3)*2 + (def_dv>>3)*4 + (att_dv>>3)*8)*5+ std::min(int(satt_dv),3))/2 + 31;
}

QList<QStringList> HiddenPowerInfo::PossibilitiesForType(int type)
{
    QStringList fileLines;

    fill_container_with_file(fileLines, path(QString("type%1_hp.txt").arg(type)));

    QList<QStringList> ret;

    foreach (QString line, fileLines)
        ret.push_back(line.split(' '));

    return ret;
}

QPair<quint8, quint8> HiddenPowerInfo::AttDefDVsForGen2(int type)
{
    return QPair<quint8, quint8>(12+ (type-1)/4, 12 + ((type-1)%4));
}

void StatInfo::init(const QString &dir)
{
    if (m_stats.size() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    fill_int_str(m_stats, path("stats.txt"), true);
    fill_int_str(m_status, path("status.txt"), true);
}

void StatInfo::retranslate()
{
    fill_int_str(m_stats, path("stats.txt"), true);
    fill_int_str(m_status, path("status.txt"), true);
}

QString StatInfo::Stat(int stat, int gen)
{
    if (stat == SpAttack && gen == 1) {
        return QObject::tr("Special", "Stat");
    }
    if (stat >= 0 && stat <= Evasion)
        return m_stats[stat];
    else
        return "";
}

QString StatInfo::Status(int stat)
{
    if (stat == Pokemon::Koed) {
        return QObject::tr("koed");
    }
    return m_status[stat];
}

QString StatInfo::ShortStatus(int stat)
{
    switch (stat) {
    case Pokemon::Koed: return QObject::tr("Ko", "Short Status");
    case Pokemon::Fine: return QObject::tr("", "Short Status");
    case Pokemon::Paralysed: return QObject::tr("Par", "Short Status");
    case Pokemon::Asleep: return QObject::tr("Slp", "Short Status");
    case Pokemon::Frozen: return QObject::tr("Frz", "Short Status");
    case Pokemon::Burnt: return QObject::tr("Brn", "Short Status");
    case Pokemon::Poisoned: return QObject::tr("Psn", "Short Status");
    case Pokemon::Confused: return QObject::tr("Cfs", "Short Status");
    default:
        return "";
    }
}

QString StatInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

void MoveInfo::setPower(int movenum, unsigned char power, int moveGen)
{
    gen(moveGen).power[movenum] = power;
}

void MoveInfo::setAccuracy(int movenum, char accuracy, int moveGen)
{
    gen(moveGen).accuracy[movenum] = accuracy;
}

void MoveInfo::setPP(int movenum, char pp, int moveGen)
{
    gen(moveGen).pp[movenum] = pp;
}

void MoveInfo::setPriority(int movenum, signed char priority, int moveGen)
{
    gen(moveGen).priority[movenum] = priority;
}

bool PokemonInfo::modifyBaseStat(const Pokemon::uniqueId &pokeid, int stat, quint8 value)
{
    if ((stat >= Hp) && (stat <= Speed) && Exists(pokeid)) {
        m_BaseStats[pokeid].setBaseStat(stat, value);
        return true;
    }else{
        return false;
    }
}

void GenInfo::init(const QString &dir)
{
    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    fill_gen_string(m_versions, path("versions.txt"), true);
    fill_double(m_gens, path("gens.txt"), true);
}

void GenInfo::retranslate()
{
    fill_gen_string(m_versions, path("versions.txt"), true);
    fill_double(m_gens, path("gens.txt"), true);
}

QString GenInfo::Gen(int gen)
{
    return m_gens.value(gen);
}

QString GenInfo::Version(const Pokemon::gen &gen)
{
    return m_versions.value(gen);
}

QString GenInfo::path(const QString &filename)
{
    return m_Directory + filename;
}
