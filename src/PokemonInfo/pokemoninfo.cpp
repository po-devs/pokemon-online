#include <QPixmapCache>

#include "pokemoninfo.h"
#include "pokemonstructs.h"

#ifdef WIN32
#include "../../SpecialIncludes/zip.h"
#else
#include <zip.h>
#endif
#include "../Utilities/functions.h"

/*initialising static variables */
QString PokemonInfo::m_Directory;
QMap<Pokemon::uniqueId, QString> PokemonInfo::m_Names;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Weights;
QHash<int, QHash<quint16, QString> > PokemonInfo::m_Desc;
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
QVector<int> PokemonInfo::m_SpecialStats;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_LevelBalance;
QHash<Pokemon::uniqueId, PokemonMoves> PokemonInfo::m_Moves;
QHash<int, quint16> PokemonInfo::m_MaxForme;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Options;
int PokemonInfo::m_trueNumberOfPokes;
QSet<Pokemon::uniqueId> PokemonInfo::m_AestheticFormes;

QHash<int, QList<int> > PokemonInfo::m_Evolutions;
QHash<int, int> PokemonInfo::m_OriginalEvos;
QHash<int, QList<int> > PokemonInfo::m_DirectEvos;
QList<Pokemon::uniqueId> PokemonInfo::m_VisiblePokesPlainList;
QHash<int, int> PokemonInfo::m_PreEvos;
FillMode::FillModeType PokemonInfo::m_CurrentMode = FillMode::NoMod;

QString MoveInfo::m_Directory;
MoveInfo::Gen MoveInfo::gens[Version::NumberOfGens];
QList<QString> MoveInfo::m_Names;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;
QList<QStringList> MoveInfo::m_MoveMessages;
QList<QString> MoveInfo::m_Details;
QList<QString> MoveInfo::m_SpecialEffects;
QList<int> MoveInfo::m_OldMoves;
QVector<bool> MoveInfo::m_KingRock;

QString ItemInfo::m_Directory;
QList<QString> ItemInfo::m_BerryNames;
QList<QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QList<QString> ItemInfo::m_SortedNames[NUMBER_GENS];
QList<QString> ItemInfo::m_SortedUsefulNames[NUMBER_GENS];
QList<QList<ItemInfo::Effect> > ItemInfo::m_RegEffects[NUMBER_GENS];
QList<QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QList<QStringList> ItemInfo::m_RegMessages;
QList<QStringList> ItemInfo::m_BerryMessages;
QList<int> ItemInfo::m_Powers;
QList<int> ItemInfo::m_BerryPowers;
QList<int> ItemInfo::m_BerryTypes;
QList<int> ItemInfo::m_UsefulItems;
QSet<int> ItemInfo::m_GenItems[NUMBER_GENS];

QList<QString> TypeInfo::m_Names;
QString TypeInfo::m_Directory;
QList<int> TypeInfo::m_TypeVsType;
QList<int> TypeInfo::m_Categories;

QList<QString> NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QList<QString> CategoryInfo::m_Names;
QString CategoryInfo::m_Directory;

QList<QString> AbilityInfo::m_Names;
QString AbilityInfo::m_Directory;
QList<AbilityInfo::Effect> AbilityInfo::m_Effects[NUMBER_GENS];
QList<QStringList> AbilityInfo::m_Messages;
QList<int> AbilityInfo::m_OldAbilities;

QList<QString> GenderInfo::m_Names;
QString GenderInfo::m_Directory;

QString HiddenPowerInfo::m_Directory;

QString StatInfo::m_Directory;
QList<QString> StatInfo::m_stats;
QList<QString> StatInfo::m_status;

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

int fill_count_files(const QString &filename, FillMode::FillModeType m) {
    return ((m != FillMode::NoMod) && filename.startsWith("db/pokes/")) ? 2 : 1;
}

void fill_check_mode_path(FillMode::FillModeType, QString &) {
}

static void fill_container_with_file(QStringList &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
    
        file.open(QIODevice::ReadOnly | QIODevice::Text);
    
        QTextStream filestream(&file);
    
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            container << filestream.readLine();
        }
    }
}

static void fill_container_with_file(QList<QString> &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            container << filestream.readLine();
        }
    }
}

static void fill_container_with_file(QVector<char> &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int var;
            filestream >> var;
            container << var;
        }
    }
}

static void fill_container_with_file(QVector<bool> &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int var;
            filestream >> var;
            container << var;
        }
    }
}

static void fill_container_with_file(QVector<unsigned char> &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int var;
            filestream >> var;
            container << var;
        }
    }
}

static void fill_container_with_file(QVector<signed char> &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
        
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        
        QTextStream filestream(&file);
        
        /* discarding all the uninteresting lines, should find a more effective way */
        while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
        {
            int var;
            filestream >> var;
            container << var;
        }
    }
}

static void fill_uid_int(QHash<Pokemon::uniqueId, int> &container, const QString &filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
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

template <class T>
static void fill_container_with_file(T &container, const QString & filename, FillMode::FillModeType m = FillMode::NoMod)
{
    QString files[] = { filename, PoCurrentModPath + "mod_" + filename };
    fill_check_mode_path(m, files[1]);
    int files_count = fill_count_files(filename, m);
    for (int i = 0; i < files_count; ++i) {
        QFile file(files[i]);
        
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

static QString get_line(const QString & filename, int linenum)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    for (int i = 0; i < linenum; i++)
    {
        filestream.readLine();
    }

    return filestream.readLine();
}

static QString trFile(const QString &beg)
{
    QSettings s;
    QString locale = s.value("language").toString();

    if (QFile::exists(beg + "_" + locale + ".txt")) {
        return beg + "_" + locale + ".txt";
    } else {
        return beg + ".txt";
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

int PokemonInfo::Type1(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Type1[gen-GEN_MIN].value(pokeid);
}

int PokemonInfo::Type2(const Pokemon::uniqueId &pokeid,int gen)
{
    return m_Type2[gen-GEN_MIN].value(pokeid);
}

int PokemonInfo::calc_stat(int gen, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    return ((2*basestat + dv * (1 + (gen <= 2) ) + ev/4)*level)/100 + 5;
}

int PokemonInfo::Stat(const Pokemon::uniqueId &pokeid, int gen, int stat, int level, quint8 dv, quint8 ev)
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
            return calc_stat(gen, basestat, level, dv, ev) + level + 5;
        }
    }
        return calc_stat(gen, basestat, level, dv, ev);
}

int PokemonInfo::FullStat(const Pokemon::uniqueId &pokeid, int gen, int nature, int stat, int level, quint8 dv, quint8 ev)
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
    m_CurrentMode = mode;
    PoCurrentModPath = readModDirectory(modName);

    QString modpath = PoCurrentModPath;
    if (modpath.isEmpty() && (m_CurrentMode != FillMode::Server)) {
        m_CurrentMode = FillMode::NoMod;
    }

    clearData();

    loadNames();
    loadEvos();
    loadMoves();

    fill_uid_int(m_Genders, path("poke_gender.txt"), m_CurrentMode);

    for (int i = 0; i < NUMBER_GENS; i++) {
        int gen = i+GEN_MIN;

        fill_uid_int(m_Type1[i], path(QString("poke_type1-%1G.txt").arg(gen)), m_CurrentMode);
        fill_uid_int(m_Type2[i], path(QString("poke_type2-%1G.txt").arg(gen)), m_CurrentMode);

        if (gen >= 3) {
            for (int j = 0; j < 3; j++) {
                fill_uid_int(m_Abilities[i][j], path(QString("poke_ability%1_%2G.txt").arg(j+1).arg(gen)), m_CurrentMode);
            }
        }
    }

    fill_uid_int(m_LevelBalance, path("level_balance.txt"), m_CurrentMode);
    fill_container_with_file(m_SpecialStats, path("specialstat.txt"), m_CurrentMode);
    loadClassifications();
    loadGenderRates();
    loadHeights();
    loadDescriptions();
    loadBaseStats();
    loadMinLevels();

    makeDataConsistent();
}

void PokemonInfo::clearData()
{
    m_Names.clear();
    m_Options.clear();
    m_MaxForme.clear();
    m_VisiblePokesPlainList.clear();
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
    m_VisiblePokesPlainList.clear();
}

void PokemonInfo::loadClassifications()
{
    QStringList temp;
    fill_container_with_file(temp, trFile(path("classification")), m_CurrentMode);
    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString description;
        quint16 pokenum;
        bool ok = Pokemon::uniqueId::extract_short(current, pokenum, description);
        if(ok) m_Classification[pokenum] = description;
    }
}

void PokemonInfo::loadGenderRates()
{
    QStringList temp;
    fill_container_with_file(temp, path("gender_rate.txt"), m_CurrentMode);
    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString description;
        quint16 pokenum;
        bool ok = Pokemon::uniqueId::extract_short(current, pokenum, description);
        if(ok) m_GenderRates[pokenum] = description.toInt();
    }
}

void PokemonInfo::loadMinLevels()
{
    for (int i = 0; i < NUMBER_GENS; i++) {
        QStringList temp;
        fill_container_with_file(temp, path(QString("minlevels_G%1.txt").arg(GEN_MIN+i)), m_CurrentMode);

        for(int j = 0; j < temp.size(); j++) {
            QString current = temp[j].trimmed();
            QString description;
            Pokemon::uniqueId pokeid;
            bool ok = Pokemon::uniqueId::extract(current, pokeid, description);
            if(ok)  {
                QStringList eggWild = description.split('/');

                m_MinLevels[i][pokeid] = eggWild.back().toInt();
                m_MinEggLevels[i][pokeid] = eggWild.front().toInt();
            }
        }
    }
}

void PokemonInfo::loadHeights()
{
    QStringList temp;
    fill_container_with_file(temp, path("height.txt"), m_CurrentMode);
    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString height;
        Pokemon::uniqueId pokeid;

        if(Pokemon::uniqueId::extract(current, pokeid, height))
            m_Height[pokeid] = height;
    }
}

void PokemonInfo::loadDescriptions()
{
    static const int CARTS_LEN = 3;
    int carts[] = { 14, 15, 16 };
    for(int i = 0; i < CARTS_LEN; i++)
    {
        QStringList temp;
        fill_container_with_file(temp, trFile(path("description_%1").arg(carts[i])), m_CurrentMode);
        for(int j = 0; j < temp.size(); j++) {
            QString current = temp[j].trimmed();
            QString description;
            quint16 pokenum;
            bool ok = Pokemon::uniqueId::extract_short(current, pokenum, description);
            if(ok) m_Desc[carts[i]][pokenum] = description;
        }
    }
}


int PokemonInfo::TrueCount(int gen)
{
    if (gen == 1)
        return 152;
    if (gen == 2)
        return 252;
    if (gen == 3)
        return 387;
    if (gen == 4)
        return 494;
    return m_trueNumberOfPokes;
}

int PokemonInfo::NumberOfPokemons()
{
    return m_Names.size();
}

int PokemonInfo::NumberOfVisiblePokes() {
    return m_VisiblePokesPlainList.size();
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

bool PokemonInfo::Exists(const Pokemon::uniqueId &pokeid, int gen)
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

    int gen = GEN_MAX;
    int gender = 0;
    Pokemon::uniqueId num = Pokemon::NoPoke;
    bool shiny=false;
    bool back = false;
    bool substitute = false;

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
        }
    }

    if (substitute) {
        return Sub(gen, back);
    } else {
        return Picture(num, gen, gender, shiny, back);
    }
}

QPixmap PokemonInfo::Picture(const Pokemon::uniqueId &pokeid, int gen, int gender, bool shiney, bool back)
{
    QString archive;

    if (gen == 1)
        archive = path("rby.zip");
    else if (gen == 2)
        archive = path("gsc.zip");
    else if (gen == 3)
        archive = path("advance.zip");
    else if (gen == 4)
        archive = path("hgss.zip");
    else {
        // TODO: Read this number from somewhere else.
        if (pokeid.pokenum > 649) {
            archive = PoCurrentModPath + "mod_" + path("mod_sprites.zip");
        } else {
            archive = path("black_white.zip");
        }
    }

    QString file;

    if (gen == 1)
        file = QString("%1/%2").arg(pokeid.toString(), back?"GBRYback.png":"Y.gif");
    else if (gen == 2)
        file = QString("%1/%2.png").arg(pokeid.toString(), back?"GSCback%3":"S%3").arg(shiney?"s":"");
    else if (gen ==3)
        file = QString("%1/%2%3.png").arg(pokeid.toString(), back?"3Gback":"RFLG", shiney?"s":"");
    else if (gen == 4)
        file = QString("%1/DP%2%3%4.png").arg(pokeid.toString(), back?"b":"", (gender==Pokemon::Female)?"f":"m", shiney?"s":"");
    else
        file = QString("%1/%2%3%4.png").arg(pokeid.toString(), back?"back":"front", (gender==Pokemon::Female)?"f":"", shiney?"s":"");

    QPixmap ret;

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
    {
        if (gen == 3) {
            if (shiney)
                return PokemonInfo::Picture(pokeid, 3, Pokemon::Male, false, back);
            else
                return PokemonInfo::Picture(pokeid, 4, gender, shiney, back);
        } else if (gen == 4 && gender == Pokemon::Female) {
            return PokemonInfo::Picture(pokeid, 4, Pokemon::Male, shiney, back);
        } else if (gen == 4 && shiney) {
            return PokemonInfo::Picture(pokeid, 4, Pokemon::Male, false, back);
        } else if (gen == 5) {
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

QPixmap PokemonInfo::Sub(int gen, bool back)
{
    QString archive;
    if (gen <= 3)
        archive = path("advance.zip");
    else
        archive = path("hgss.zip");

    QString file = QString("sub%1%2.png").arg(back?"b":"").arg(gen>=4?"":"3G");

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

QPixmap PokemonInfo::Icon(const Pokemon::uniqueId &pokeid)
{
    QString archive;
    // TODO: Read this number from somewhere else.
    if (pokeid.pokenum > 649) {
        archive = PoCurrentModPath + "mod_" + path("mod_icons.zip");
    } else {
        archive = path("icons.zip");
    }
    QString file = QString("%1.png").arg(pokeid.toString());

    QPixmap p;

    if (QPixmapCache::find(archive+file, &p)) {
        return p;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        if (IsForme(pokeid)) {
            return Icon(OriginalForme(pokeid));
        }

        qDebug() << "error loading icon";
        return p;
    }

    p.loadFromData(data,"png");
    return p;
}

QByteArray PokemonInfo::Cry(const Pokemon::uniqueId &pokeid)
{
    quint16 num = pokeid.pokenum;
    QString archive;
    // TODO: Read this number from somewhere else.
    if (pokeid.pokenum > 649) {
        archive = PoCurrentModPath + "mod_" + path("mod_cries.zip");
    } else {
        archive = path("cries.zip");
    }
    QString file = QString("%1.wav").arg(num).rightJustified(7, '0');

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        qDebug() << "error loading pokemon cry " << num;
    }

    return data;
}

QSet<int> PokemonInfo::Moves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).genMoves[gen-GEN_MIN];
}

bool PokemonInfo::HasMoveInGen(const Pokemon::uniqueId &pokeid, int move, int gen)
{
    return m_Moves[pokeid].regularMoves[gen-GEN_MIN].contains(move) || m_Moves[pokeid].specialMoves[gen-GEN_MIN].contains(move)
                || m_Moves[pokeid].eggMoves[gen-GEN_MIN].contains(move) || m_Moves[pokeid].preEvoMoves[gen-GEN_MIN].contains(move);
}

QSet<int> PokemonInfo::RegularMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).regularMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::EggMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).eggMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::LevelMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).levelMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::TutorMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).tutorMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::TMMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).TMMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::SpecialMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).specialMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::PreEvoMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).preEvoMoves[gen-GEN_MIN];
}

QSet<int> PokemonInfo::dreamWorldMoves(const Pokemon::uniqueId &pokeid)
{
    return m_Moves.value(pokeid).dreamWorldMoves;
}

AbilityGroup PokemonInfo::Abilities(const Pokemon::uniqueId &pokeid, int gen)
{
    AbilityGroup ret;

    for (int i = 0; i < 3; i++) {
        ret._ab[i] = m_Abilities[gen-GEN_MIN][i].value(pokeid);
    }

    return ret;
}

int PokemonInfo::Ability(const Pokemon::uniqueId &pokeid, int slot, int gen)
{
    return m_Abilities[gen-GEN_MIN][slot].value(pokeid);
}

void PokemonInfo::loadBaseStats()
{
    QStringList temp;
    fill_container_with_file(temp, path("poke_stats.txt"), m_CurrentMode);

    for (int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString text_stats;
        Pokemon::uniqueId id;
        bool ok = Pokemon::uniqueId::extract(current, id, text_stats);
        if(ok){
            QTextStream statsstream(&text_stats, QIODevice::ReadOnly);
            int hp, att, def, spd, satt, sdef;
            statsstream >> hp >> att >> def >> spd >> satt >> sdef;
            m_BaseStats[id] = PokeBaseStats(hp, att, def, spd, satt, sdef);
        } // if ok
    } // for i
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
    QStringList temp;
    fill_container_with_file(temp, trFile(path("pokemons")), m_CurrentMode);

    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString name;
        QString options;
        Pokemon::uniqueId id;
        bool ok = Pokemon::uniqueId::extract(current, id, name, &options);
        if(ok) {
            m_Names[id] = name;
            m_Options[id] = options;

            if (AFormesShown(id)) {
                m_VisiblePokesPlainList.append(id);
            }

            // Calculate a number of formes a given base pokemon have.
            quint16 max_forme = m_MaxForme.value(id.pokenum, 0);
            if(max_forme < id.subnum){
                max_forme = id.subnum;
            }
            m_MaxForme[id.pokenum] = max_forme;
        }
    }

    // Loading weights too for grass knot and low kick...
    temp.clear();
    fill_container_with_file(temp, path("poke_weight.txt"), m_CurrentMode);
    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString weight;
        Pokemon::uniqueId id;
        bool ok = Pokemon::uniqueId::extract(current, id, weight);
        if(ok)
            m_Weights[id] = weight;
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

QList<Pokemon::uniqueId> PokemonInfo::Formes(const Pokemon::uniqueId &pokeid, int gen)
{
    QList<Pokemon::uniqueId> result;
    for(quint16 i = 0; i <= NumberOfAFormes(pokeid); i++) {
        if (Exists(Pokemon::uniqueId(pokeid.pokenum, i), gen))
            result.append(Pokemon::uniqueId(pokeid.pokenum, i));
    }
    return result;
}

QList<Pokemon::uniqueId> PokemonInfo::VisibleFormes(const Pokemon::uniqueId &pokeid, int gen)
{
    QList<Pokemon::uniqueId> result;
    for(quint16 i = 0; i <= NumberOfAFormes(pokeid); i++) {
        Pokemon::uniqueId poke(pokeid.pokenum, i);
        if(Exists(poke, gen) && AFormesShown(poke)) result.append(poke);
    }
    return result;
}

int PokemonInfo::MinLevel(const Pokemon::uniqueId &pokeid, int gen)
{
    int g = gen-GEN_MIN;

    if (!m_MinLevels[g].contains(pokeid))
        return 100;

    return m_MinLevels[g][pokeid];
}

int PokemonInfo::MinEggLevel(const Pokemon::uniqueId &pokeid, int gen)
{
    int g = gen-GEN_MIN;

    if (!m_MinLevels[g].contains(pokeid))
        return 100;

    return m_MinLevels[g][pokeid];
}

int PokemonInfo::AbsoluteMinLevel(const Pokemon::uniqueId &pokeid, int gen)
{
    int limit = (gen >= 3 ? 3 : GEN_MIN);

    int min = 100;
    for (int g = gen; g >= limit; g--) {
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

    for (int i = 0; i < filesize; i++) {
        QStringList temp;
        fill_container_with_file(temp, fileNames[i], m_CurrentMode);
        for(int j = 0; j < temp.size(); j++) {
            QString current = temp[j].trimmed();
            QString text_moves;
            Pokemon::uniqueId pokeid;

            if(Pokemon::uniqueId::extract(current, pokeid, text_moves)) {
                QStringList move_list = text_moves.split(' ');
                QSet<int> data_set;
                for(int ml_counter = 0; ml_counter < move_list.size(); ml_counter++) {
                    int move = move_list[ml_counter].toInt();
                    if(move != 0)
                        data_set.insert(move);
                }
                // Should create an item with pokeid key
                // in m_Moves if it does not exist.
                PokemonMoves &moves = m_Moves[pokeid];
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

Pokemon::uniqueId PokemonInfo::getRandomPokemon(int gen)
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

bool PokemonInfo::modifyAbility(const Pokemon::uniqueId &pokeid, int slot, int ability, int gen)
{
    if ((slot >= 0) && (slot <= 2) && Exists(pokeid, gen)) {
        m_Abilities[gen - GEN_MIN][slot][pokeid] = ability;
        return true;
    }else{
        return false;
    }
}

void MoveInfo::Gen::load(const QString &dir, int gen)
{
    this->gen = gen;
    this->dir = QString("%1%2G/").arg(dir).arg(gen);

    fill_container_with_file(accuracy, path("accuracy.txt"));
    fill_container_with_file(category, path("category.txt"));
    fill_container_with_file(causedEffect, path("caused_effect.txt"));
    fill_container_with_file(critRate, path("crit_rate.txt"));
    fill_container_with_file(damageClass, path("damage_class.txt"));
    fill_container_with_file(effect, trFile(path("effect")));
    fill_container_with_file(effectChance, path("effect_chance.txt"));
    fill_container_with_file(flags, path("flags.txt"));
    fill_container_with_file(flinchChance, path("flinch_chance.txt"));
    fill_container_with_file(healing, path("healing.txt"));
    fill_container_with_file(maxTurns, path("max_turns.txt"));
    fill_container_with_file(minTurns, path("min_turns.txt"));
    fill_container_with_file(minMaxHits, path("min_max_hits.txt"));
    fill_container_with_file(none0, path("None0.txt"));
    fill_container_with_file(none1, path("None1.txt"));
    fill_container_with_file(none2, path("None2.txt"));
    fill_container_with_file(power, path("power.txt"));
    fill_container_with_file(pp, path("pp.txt"));
    fill_container_with_file(priority, path("priority.txt"));
    fill_container_with_file(range, path("range.txt"));
    fill_container_with_file(recoil, path("recoil.txt"));
    fill_container_with_file(status, path("status.txt"));
    fill_container_with_file(type, path("type.txt"));

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

QString MoveInfo::Gen::path(const QString &fileName)
{
    return dir + fileName;
}

void MoveInfo::loadMoveMessages()
{
    QStringList temp;
    fill_container_with_file(temp, trFile(path("move_message")));

    foreach(QString str, temp) {
	m_MoveMessages.push_back(str.split('|'));
    }
}

void MoveInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfMoves() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadMoveMessages();
    loadDetails();
    loadSpecialEffects();

    fill_container_with_file(m_OldMoves, path("oldmoves.txt"));
    fill_container_with_file(m_KingRock, path("king_rock.txt"));

    for (int i = 0; i < Version::NumberOfGens; i++) {
        gens[i].load(dir, i+1);
    }
}

void MoveInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("moves")));
    for (int i = 0; i < m_Names.size(); i++) {
        m_LowerCaseMoves.insert(m_Names[i].toLower(),i);
    }
}

void MoveInfo::loadDetails()
{
    fill_container_with_file(m_Details, trFile(path("move_description")));
}

void MoveInfo::loadSpecialEffects()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_special_effects.txt"));

    /* Removing comments, aka anything starting from '#' */
    foreach (QString eff, temp) {
        m_SpecialEffects.push_back(eff.split('#').front());
    }
}

QString MoveInfo::Name(int movenum)
{
    return Exists(movenum, GEN_MAX) ? m_Names[movenum] : m_Names[0];
}

int MoveInfo::Type(int movenum, int g)
{
    return gen(g).type[movenum];
}

int MoveInfo::ConvertFromOldMove(int oldmovenum)
{
    return m_OldMoves[oldmovenum];
}

int MoveInfo::Category(int movenum, int g)
{
    if (g >= 4)
        return gen(g).damageClass[movenum];

    if (Power(movenum, g) == 0)
        return Move::Other;

    return TypeInfo::Category(Type(movenum, g));
}

int MoveInfo::Classification(int movenum, int g)
{
    return gen(g).category[movenum];
}

bool MoveInfo::FlinchByKingRock(int movenum, int gen)
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

int MoveInfo::NumberOfMoves()
{
    return m_Names.size();
}

int MoveInfo::FlinchRate(int num, int g)
{
    return gen(g).flinchChance[num];
}

int MoveInfo::Recoil(int movenum, int g)
{
    return gen(g).recoil[movenum];
}

QString MoveInfo::Description(int movenum, int g)
{
    QString r = gen(g).effect[movenum];
    r.replace("$effect_chance", QString::number(EffectRate(movenum, g)));

    return r;
}

int MoveInfo::Power(int movenum, int g)
{
    return gen(g).power[movenum];
}

QString MoveInfo::PowerS(int movenum, int gen)
{
    int p = Power(movenum, gen);

    if (p == 0)
        return "--";
    else if (p == 1)
        return "???";
    else
        return QString::number(p);
}

int MoveInfo::PP(int movenum, int g)
{
    return gen(g).pp[movenum];
}

int MoveInfo::Acc(int movenum, int g)
{
    return gen(g).accuracy[movenum];
}

QString MoveInfo::AccS(int movenum, int gen)
{
    int acc = MoveInfo::Acc(movenum, gen);

    if (acc == 101)
        return "--";
    else
        return QString::number(acc);
}

int MoveInfo::CriticalRaise(int movenum, int g)
{
    return gen(g).critRate[movenum];
}

int MoveInfo::RepeatMin(int movenum, int g)
{
    return gen(g).minMaxHits[movenum] & 0xF;
}

int MoveInfo::RepeatMax(int movenum, int g)
{
    return gen(g).minMaxHits[movenum] >> 4;
}

int MoveInfo::SpeedPriority(int movenum, int g)
{
    return gen(g).priority[movenum];
}

int MoveInfo::Flags(int movenum, int g)
{
    return gen(g).flags[movenum];
}

bool MoveInfo::Exists(int movenum, int g)
{
    return gen(g).power.size() > movenum;
}

bool MoveInfo::isOHKO(int movenum, int gen)
{
    return Classification(movenum, gen) == Move::OHKOMove;
}

bool MoveInfo::isHM(int movenum, int g)
{
    return gen(g).HMs.contains(movenum);
}

int MoveInfo::EffectRate(int movenum, int g)
{
    return gen(g).effectChance[movenum];
}

quint32 MoveInfo::StatAffected(int movenum, int g)
{
    return gen(g).none0[movenum];
}

quint32 MoveInfo::BoostOfStat(int movenum, int g)
{
    return gen(g).none1[movenum];
}

quint32 MoveInfo::RateOfStat(int movenum, int g)
{
    return gen(g).none2[movenum];
}

int MoveInfo::Target(int movenum, int g)
{
    return gen(g).range[movenum];
}

int MoveInfo::Healing(int movenum, int g)
{
    return gen(g).healing[movenum];
}

int MoveInfo::MinTurns(int movenum, int g)
{
    return gen(g).minTurns[movenum];
}

int MoveInfo::MaxTurns(int movenum, int g)
{
    return gen(g).maxTurns[movenum];
}

int MoveInfo::Status(int movenum, int g)
{
    return gen(g).causedEffect[movenum];
}

int MoveInfo::StatusKind(int movenum, int g)
{
    return gen(g).status[movenum];
}

QString MoveInfo::MoveMessage(int moveeffect, int part)
{
    if (moveeffect < 0 || moveeffect >= m_MoveMessages.size() || part < 0 || part >= m_MoveMessages[moveeffect].size()) {
	return "";
    }
    return m_MoveMessages[moveeffect][part];
}

QString MoveInfo::SpecialEffect(int movenum)
{
    return m_SpecialEffects[movenum];
}

QString MoveInfo::DetailedDescription(int movenum)
{
    return m_Details[movenum];
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}


//void MoveInfo::loadEffects()
//{
//    for (int i = 0; i < 3; i++) {
//        QStringList temp;
//        fill_container_with_file(temp, path(QString("moveeffects_%1G.txt").arg(i+3)));

//        /* Removing comments, aka anything starting from '#' */
//        foreach (QString eff, temp) {
//            m_Effects[i].push_back(eff.split('#').front());
//        }
//        makeConsistent(m_Effects[i]);
//    }
//}



QStringList MoveInfo::MoveList()
{
    return m_Names;
}

void ItemInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfItems() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

void ItemInfo::loadNames()
{
    fill_container_with_file(m_RegItemNames, trFile(path("items")));

    m_ItemNamesH.reserve(m_RegItemNames.size());

    QStringList::const_iterator it = m_RegItemNames.begin();
    for (int i = 0; it != m_RegItemNames.end(); i++, ++it) {
	m_ItemNamesH.insert(*it, i);
    }

    fill_container_with_file(m_BerryNames, trFile(path("berries")));
    m_BerryNamesH.reserve(m_BerryNames.size());

    QStringList::const_iterator it2 = m_BerryNames.begin();
    for (int i = 0; it2 != m_BerryNames.end(); i++, ++it2) {
	m_BerryNamesH.insert(*it2, i+8000);
    }

    fill_container_with_file(m_BerryPowers, path("berry_pow.txt"));
    fill_container_with_file(m_BerryTypes, path("berry_type.txt"));
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

    int mg = GEN_MAX - GEN_MIN;

    m_SortedNames[mg] << m_RegItemNames << m_BerryNames;
    qSort(m_SortedNames[mg]);

    m_SortedUsefulNames[mg] << m_BerryNames;
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

    for (int i = GEN_MIN_ITEMS; i <= GEN_MAX; i++) {
        QStringList temp;
        fill_container_with_file(temp, path("item_effects_%1G.txt").arg(i));

        /* Removing comments, aka anything starting from '#' */
        foreach (QString eff, temp) {
            QStringList effects = eff.split('#').front().split('|');
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
            m_RegEffects[i-GEN_MIN].push_back(toPush);
        }
    }

    QStringList temp;
    fill_container_with_file(temp, path("berry_effects.txt"));
    /* Removing comments, aka anything starting from '#' */
    foreach (QString eff, temp) {
	QStringList effects = eff.split('#').front().split('|');
	QList<Effect> toPush;
	foreach(QString eff, effects) {
	    std::string s = eff.toStdString();
	    size_t pos = s.find('-');
	    if (pos != std::string::npos) {
                toPush.push_back(Effect(atoi(s.c_str())+8000, eff.mid(pos+1)));
	    } else {
                toPush.push_back(Effect(atoi(s.c_str())+8000));
	    }
	}
	m_BerryEffects.push_back(toPush);
    }

    temp.clear();
    fill_container_with_file(temp, trFile(path("item_messages")));
    foreach (QString eff, temp) {
	m_RegMessages.push_back(eff.split('|'));
    }

    temp.clear();
    fill_container_with_file(temp, trFile(path("berry_messages")));
    foreach (QString eff, temp) {
	m_BerryMessages.push_back(eff.split('|'));
    }

    fill_container_with_file(m_Powers, path("items_pow.txt"));
}

QList<ItemInfo::Effect> ItemInfo::Effects(int item, int gen)
{
    if (!Exists(item, gen)) {
	return QList<ItemInfo::Effect>();
    } else {
        return isBerry(item) ? m_BerryEffects[item-8000] : m_RegEffects[gen-GEN_MIN][item];
    }
}

QString ItemInfo::Message(int effect, int part)
{
    if (effect < 8000) {
	if (m_RegMessages.size() <= effect || m_RegMessages[effect].size() <= part) {
	    return "";
	}
	return m_RegMessages[effect][part];
    } else {
	effect = effect-8000;
	if (m_BerryMessages.size() <= effect || m_BerryMessages[effect].size() <= part) {
	    return "";
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
    if ( itemnum < 0 || (itemnum < 8000 && m_RegItemNames.size() <= itemnum) || (itemnum >= 8000 && m_BerryNames.size() + 8000 <= itemnum) ) {
	return 0;
    }
    if (itemnum < 8000) {
	return m_RegItemNames[itemnum];
    } else {
	return m_BerryNames[itemnum-8000];
    }
}

bool ItemInfo::Exists(int itemnum, int gen)
{
    return m_GenItems[gen-GEN_MIN].contains(itemnum);
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

QList<QString> ItemInfo::SortedNames(int gen)
{
    return m_SortedNames[gen-GEN_MIN];
}

QList<QString> ItemInfo::SortedUsefulNames(int gen)
{
    return m_SortedUsefulNames[gen-GEN_MIN];
}

void TypeInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("types")));
    fill_container_with_file(m_Categories, path("category.txt"));
}

QString TypeInfo::path(const QString& file)
{
    return m_Directory+file;
}

void TypeInfo::loadEff()
{
    QStringList temp;

    fill_container_with_file(temp, path("typestable.txt"));

    foreach (QString l, temp) {
	QStringList l2 = l.split(' ');
	foreach (QString l3, l2) {
	    m_TypeVsType.push_back(l3.toInt());
	}
    }
}

void TypeInfo::init(const QString &dir)
{
    if (NumberOfTypes() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadEff();
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
        case Hail: return "hailstorm";
        case Rain: return "rain";
        case SandStorm: return "sandstorm";
        case Sunny: return "sunny";
        default: return "normal";
    }
}

int TypeInfo::Eff(int type_attack, int type_defend)
{
    return m_TypeVsType[type_attack * NumberOfTypes() + type_defend];
}


int TypeInfo::Number(const QString &pokename)
{
    return (qFind(m_Names.begin(), m_Names.end(), pokename)-m_Names.begin()) % (NumberOfTypes());
}


QString TypeInfo::Name(int typenum)
{
    if (typenum >= m_Names.size())
        return Name(0);
    return m_Names[typenum];
}

int TypeInfo::NumberOfTypes()
{
    return m_Names.size();
}

int TypeInfo::Category(int type)
{
    return m_Categories[type];
}

void TypeInfo::modifyTypeChart(int type_attack, int type_defend, int value)
{
    m_TypeVsType[type_attack * NumberOfTypes() + type_defend] = value;
}

void NatureInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("nature")));
}

QString NatureInfo::path(const QString &filename)
{
    return m_Directory + filename;
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

QString NatureInfo::Name(int naturenum)
{
    return m_Names[naturenum];
}

int NatureInfo::NumberOfNatures()
{
    return m_Names.size();
}

int NatureInfo::Number(const QString &pokename)
{
    return (qFind(m_Names.begin(), m_Names.end(), pokename)-m_Names.begin()) % (NumberOfNatures());
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


void CategoryInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("categories")));
}

QString CategoryInfo::path(const QString& file)
{
    return m_Directory+file;
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

QString CategoryInfo::Name(int catnum)
{
    return m_Names[catnum];
}

int CategoryInfo::NumberOfCategories()
{
    return m_Names.size();
}

void AbilityInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("abilities")));
}

QString AbilityInfo::Message(int ab, int part) {
    if (ab < 0 || ab >= m_Messages.size() || part < 0 || part >= m_Messages[ab].size()) {
        return "";
    }

    return m_Messages[ab][part];
}

QString AbilityInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

void AbilityInfo::init(const QString &dir)
{
    if (NumberOfAbilities() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadEffects();

    QStringList temp;
    fill_container_with_file(temp, path("ability_messages.txt"));
    fill_container_with_file(m_OldAbilities, path("oldabilities.txt"));
    foreach (QString eff, temp) {
        m_Messages.push_back(eff.split('|'));
    }
}

bool AbilityInfo::Exists(int ability, int gen)
{
    return gen <= 3 ? ability <= Ability::AirLock : (gen ==4 ? ability <=  Ability::BadDreams : true);
}

void AbilityInfo::loadEffects()
{
    for (int i = GEN_MIN_ABILITIES; i <= GEN_MAX; i++) {
        QStringList m_temp;
        fill_container_with_file(m_temp,path("ability_effects_%1G.txt").arg(i));

        foreach(QString str, m_temp) {
            QStringList content = str.split('#').front().split('-');
            if (content.size() == 1) {
                m_Effects[i-GEN_MIN].push_back(Effect(content[0].toInt()));
            } else {
                m_Effects[i-GEN_MIN].push_back(Effect(content[0].toInt(), content[1].toInt()));
            }
        }
    }
}

AbilityInfo::Effect AbilityInfo::Effects(int abnum, int gen) {
    return m_Effects[gen-GEN_MIN][abnum];
}

QString AbilityInfo::Desc(int ab)
{
    return get_line(trFile(path("ability_desc")), ab);
}

QString AbilityInfo::EffectDesc(int abnum)
{
    return get_line(trFile(path("ability_battledesc")), abnum);
}

int AbilityInfo::ConvertFromOldAbility(int oldability)
{
    return m_OldAbilities[oldability];
}

int AbilityInfo::Number(const QString &pokename)
{
    return (qFind(m_Names.begin(), m_Names.end(), pokename)-m_Names.begin()) % (NumberOfAbilities());
}

QString AbilityInfo::Name(int abnum)
{
    if (abnum >=0 && abnum < NumberOfAbilities())
	return m_Names[abnum];
    else
	return 0;
}

int AbilityInfo::NumberOfAbilities()
{
    return m_Names.size();
}


void GenderInfo::loadNames()
{
    fill_container_with_file(m_Names, path("genders_en.txt"));
}

QString GenderInfo::path(const QString &filename)
{
    return m_Directory + filename;
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

QString GenderInfo::Name(int abnum)
{
    return m_Names[abnum];
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

int HiddenPowerInfo::Type(int gen, quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 satt_dv, quint8 sdef_dv, quint8 speed_dv)
{
    if (gen >= 3)
        return (((hp_dv%2) + (att_dv%2)*2 + (def_dv%2)*4 + (speed_dv%2)*8 + (satt_dv%2)*16 + (sdef_dv%2)*32)*15)/63 + 1;
    else
        return (att_dv%4)*4+(def_dv%4)+1;
}

int HiddenPowerInfo::Power(int gen, quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 satt_dv, quint8 sdef_dv, quint8 speed_dv)
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

    fill_container_with_file(m_stats, trFile(path("stats")));
    fill_container_with_file(m_status, trFile(path("status")));
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
    case Pokemon::Koed: return "Ko";
    case Pokemon::Fine: return "";
    case Pokemon::Paralysed: return "Par";
    case Pokemon::Asleep: return "Slp";
    case Pokemon::Frozen: return "Frz";
    case Pokemon::Burnt: return "Brn";
    case Pokemon::Poisoned: return "Psn";
    case Pokemon::Confused: return "Cfs";
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

QString PokemonInfo::readModDirectory(const QString &modName)
{
    if (m_CurrentMode == FillMode::Server) {
        return ""; // from current directory, mod_db/*
    }

    QSettings s_mod(PoModLocalPath + "mods.ini", QSettings::IniFormat);
    int mod_id = s_mod.value(modName + "/id", 0).toInt();
    if (mod_id == 0) {
        return "";
    } else {
        QString result = PoModLocalPath + QString::number(mod_id) + "/";
        if (QDir(result).exists()) {
            return result;
        } else {
            return "";
        }
    }
}
