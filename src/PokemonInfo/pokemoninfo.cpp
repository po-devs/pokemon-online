#include "pokemon.h"

#include <QPixmapCache>
#include <stdexcept>

#include "pokemoninfo.h"
#include "pokemonstructs.h"

#ifdef _WIN32
#include "../../SpecialIncludes/zip.h"
#else
#include <zip.h>
#endif
#include "../Utilities/functions.h"
#include "../Utilities/coreclasses.h"

/*initialising static variables */
QString PokemonInfo::m_Directory;
QMap<Pokemon::uniqueId, QString> PokemonInfo::m_Names;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Weights;
QHash<int, QHash<int, QString> > PokemonInfo::m_Desc;
QHash<int, QString> PokemonInfo::m_Classification;
QHash<int, int> PokemonInfo::m_GenderRates;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Height;

QHash<Pokemon::uniqueId, int> PokemonInfo::m_Genders;
QVector<QHash<Pokemon::uniqueId, int> > PokemonInfo::m_Type1;
QVector<QHash<Pokemon::uniqueId, int> > PokemonInfo::m_Type2;
QVector<QHash<Pokemon::uniqueId, int> > PokemonInfo::m_Abilities[3];
QVector<QHash<Pokemon::uniqueId, PokeBaseStats> > PokemonInfo::m_BaseStats;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_LevelBalance;
QHash<int, quint16> PokemonInfo::m_MaxForme;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Options;
int PokemonInfo::m_trueNumberOfPokes;
QSet<Pokemon::uniqueId> PokemonInfo::m_AestheticFormes;
QHash<Pokemon::gen, PokemonInfo::Gen> PokemonInfo::gens;

QHash<int, QList<int> > PokemonInfo::m_Evolutions;
QHash<int, int> PokemonInfo::m_OriginalEvos;
QHash<int, QList<int> > PokemonInfo::m_DirectEvos;
QHash<int, int> PokemonInfo::m_PreEvos;

QString MoveInfo::m_Directory;
QHash<Pokemon::gen, MoveInfo::Gen> MoveInfo::gens;
QHash<int,QString> MoveInfo::m_Names;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;
QHash<int, QStringList> MoveInfo::m_MoveMessages;
QHash<int,QString> MoveInfo::m_Details;
QHash<int,int> MoveInfo::m_OldMoves;
QVector<QSet<int> > MoveInfo::m_GenMoves;

QString ItemInfo::m_Directory;
QHash<int,QString> ItemInfo::m_BerryNames;
QHash<int,QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QVector<QList<QString> > ItemInfo::m_SortedNames;
QVector<QList<QString> > ItemInfo::m_SortedUsefulNames;
QVector<QHash<int,QList<ItemInfo::Effect> > > ItemInfo::m_RegEffects;
QHash<int,QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QHash<int, QStringList> ItemInfo::m_RegMessages;
QHash<int, QStringList> ItemInfo::m_BerryMessages;
QHash<int,int> ItemInfo::m_Powers;
QHash<int,int> ItemInfo::m_BerryPowers;
QHash<int,int> ItemInfo::m_BerryTypes;
QHash<int, bool> ItemInfo::m_UsefulItems, ItemInfo::m_UsefulBerries;
QVector<QSet<int> > ItemInfo::m_GenItems;

QHash<int, QString> TypeInfo::m_Names;
QString TypeInfo::m_Directory;
QVector<QHash<int, QVector<int> > > TypeInfo::m_TypeVsType;
QHash<int, int> TypeInfo::m_Categories;

QHash<int, QString> NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QHash<int, QString> CategoryInfo::m_Names;
QString CategoryInfo::m_Directory;

QHash<int,QString> AbilityInfo::m_Names, AbilityInfo::m_Desc, AbilityInfo::m_BattleDesc;
QString AbilityInfo::m_Directory;
QVector<QHash<int,AbilityInfo::Effect> > AbilityInfo::m_Effects;
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
QHash<int, int> GenInfo::m_NumberOfSubgens;
int GenInfo::genMin, GenInfo::genMax;

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

namespace PokemonInfoConfig {
static QString transPath, modPath;
static bool noWholeGen = false;

FillMode::FillModeType fillMode;

void setLastSubgenToWhole(bool yes) {
    noWholeGen = yes;
}

void setFillMode(FillMode::FillModeType mode) {
    fillMode = mode;
}

FillMode::FillModeType getFillMode() {
    return fillMode;
}

void changeTranslation(const QString &ts)
{
    if (ts.length() > 0) {
        transPath = QString("trans/%1/").arg(ts);
    } else {
        transPath.clear();
    }
}

void changeMod(const QString &mod)
{
    if (mod.length() == 0 || fillMode == FillMode::NoMod) {
        modPath.clear();
    } else {
        //QString cleanMod = QString::fromUtf8(QUrl::toPercentEncoding(mod));
        if (fillMode == FillMode::Client) {
            modPath = appDataPath("Mods") + "/" + mod + "/";
        } else {
            modPath = QString(dataRepo()+"Mods/%1/").arg(mod);
        }
        if (!QDir(modPath).exists()) {
            modPath.clear();
        }
    }
}

QString m_dataRepo = "./";

const QString& dataRepo() {
    return m_dataRepo;
}

void setDataRepo(const QString &s)
{
    m_dataRepo = s;
}

QString currentModPath()
{
    return modPath;
}

QString currentMod()
{
    if (modPath.length() == 0) {
        return QString();
    }

    return QDir(modPath).dirName();
}

QStringList allFiles(const QString &filename, bool trans) {
    QStringList ret;

    if (QFile::exists(dataRepo()+filename)) {
        ret << (dataRepo()+filename);
    }

    if (trans && transPath.length() > 0 && QFile::exists(dataRepo()+transPath+filename)) {
        ret << (dataRepo() + transPath + filename);
    }

    if (modPath.length() > 0 && QFile::exists(modPath+filename)) {
        ret << (modPath + filename);
    }

    return ret;
}

QStringList availableMods()
{
    QStringList ret;

    if (fillMode == FillMode::NoMod) {
        return ret;
    }

    QDir modDir(fillMode == FillMode::Client ? appDataPath("Mods") : (dataRepo() + "Mods"));

    if (modDir.exists()) {
        QStringList dirs = modDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot,QDir::Name);

        foreach(QString dir, dirs) {
            modDir.cd(dir);

            if (modDir.exists("mod.ini")) {
                ret << modDir.dirName();
            }

            modDir.cdUp();
        }
    }

    return ret;
}
}

using namespace PokemonInfoConfig;

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
            filestream >> line;

            if (filestream.atEnd() || filestream.status() == QTextStream::ReadCorruptData) {
                break;
            }

            filestream >> var;
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
            filestream >> var1;
            if (!filestream.atEnd()) {
                filestream >> var2;
            }
            container.insert(var1, var2);
        }
    }
}

template <class T>
static void fill_int_str(T &container, const QString &filename, bool trans = false)
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
    if (m_Type1[gen.num-GEN_MIN].contains(pokeid)) {
        return m_Type1[gen.num-GEN_MIN].value(pokeid);
    } else {
        return m_Type1[gen.num-GEN_MIN].value(pokeid.original(), Type::Curse);
    }
}

int PokemonInfo::Type2(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    if (m_Type2[gen.num-GEN_MIN].contains(pokeid)) {
        return m_Type2[gen.num-GEN_MIN].value(pokeid);
    } else {
        return m_Type2[gen.num-GEN_MIN].value(pokeid.original());
    }
}

int PokemonInfo::calc_stat(int gen, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    return ((2*basestat + dv * (1 + (gen <= 2) ) + ev/4)*level)/100 + 5;
}

int PokemonInfo::Stat(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int stat, int level, quint8 dv, quint8 ev)
{
    quint8 basestat = PokemonInfo::BaseStats(pokeid, gen).baseStat(stat);

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

static istringmap<Pokemon::uniqueId> pokenamesToIds;

void PokemonInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

    // Load db/pokes data.
    loadNames();
    loadEvos();

    gens.clear();

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        /* -1 stands for whole gen */
        loadGen(Pokemon::gen(i, -1));

        /* Server loads everything, every subgens, from the get go */
        if (fillMode == FillMode::Server) {
            for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
                loadGen(Pokemon::gen(i, j));
            }
        }
    }

    fill_uid_int(m_Genders, path("gender.txt"));

    m_Type1.clear();
    m_Type2.clear();
    m_Abilities[0].clear();
    m_Abilities[1].clear();
    m_Abilities[2].clear();
    m_BaseStats.clear();

    const int numGens = GenInfo::NumberOfGens();

    m_Type1.resize(numGens);
    m_Type2.resize(numGens);
    m_Abilities[0].resize(numGens);
    m_Abilities[1].resize(numGens);
    m_Abilities[2].resize(numGens);
    m_BaseStats.resize(numGens);

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        Pokemon::gen gen(i, -1);

        fill_uid_int(m_Type1[i-GenInfo::GenMin()], path(QString("type1.txt"),gen));
        fill_uid_int(m_Type2[i-GenInfo::GenMin()], path(QString("type2.txt"),gen));

        if (gen >= 3) {
            for (int j = 0; j < 3; j++) {
                fill_uid_int(m_Abilities[j][i-GenInfo::GenMin()], path(QString("ability%1.txt").arg(j+1), gen));
            }
        }

        QHash<Pokemon::uniqueId, QString> temp;
        fill_uid_str(temp, path("stats.txt", gen));

        QHashIterator<Pokemon::uniqueId, QString> it(temp);

        while (it.hasNext()) {
            it.next();
            QString text_stats = it.value();
            QTextStream statsstream(&text_stats, QIODevice::ReadOnly);

            int hp, att, def, spd, satt, sdef;
            statsstream >> hp >> att >> def >> spd >> satt >> sdef;
            m_BaseStats[i-GenInfo::GenMin()][it.key()] = PokeBaseStats(hp, att, def, spd, satt, sdef);
        }
    }
    
    fill_uid_int(m_LevelBalance, path("level_balance.txt"));
    loadClassifications();
    loadGenderRates();
    loadHeights();
    loadDescriptions();

    makeDataConsistent();
}

void PokemonInfo::loadStadiumTradebacks()
{
    loadGen(::Gen::StadiumWithTradebacks);
    loadGen(Pokemon::gen(2, Pokemon::gen::wholeGen));

    gens[::Gen::StadiumWithTradebacks].addTradebacks(&gens[Pokemon::gen(2, Pokemon::gen::wholeGen)]);
}

void PokemonInfo::loadGen(Pokemon::gen g)
{
    if (gens.contains(g)) {
        return;
    }

    if (g.subnum == g.wholeGen || g.subnum == 0) {
        /* 3 for Advance, there's a break between advance / GSC */
        if (g.num != 3 && g.num != GenInfo::GenMin()) {
            loadGen(Pokemon::gen(g.num-1, g.wholeGen));
            gens[g].load(m_Directory, g, &gens[Pokemon::gen(g.num-1, g.wholeGen)]);
        } else {
            gens[g].load(m_Directory, g, NULL);
        }
    } else {
        loadGen(Pokemon::gen(g.num, g.subnum-1));
        gens[g].load(m_Directory, g, &gens[Pokemon::gen(g.num, g.subnum-1)]);
    }

    if (g == ::Gen::StadiumWithTradebacks && MoveInfo::isInit()) {
        loadGen(Pokemon::gen(2, g.wholeGen));

        gens[g].addTradebacks(&gens[Pokemon::gen(2, g.wholeGen)]);
    }
}

void PokemonInfo::retranslate()
{
    loadNames();
    loadClassifications();
    loadDescriptions();
}

void PokemonInfo::loadClassifications()
{
    fill_int_str(m_Classification, path("classification.txt"), true);
}

void PokemonInfo::loadGenderRates()
{
    fill_double(m_GenderRates, path("gender_rate.txt"));
}

void PokemonInfo::Gen::load(const QString &_path, const Pokemon::gen &_gen, Gen *parent)
{
    gen = _gen;

    if (gen.subnum == static_cast<decltype(gen.subnum)>(-1)) {
        dir = QString("%1%2G/").arg(_path).arg(gen.num);
    } else {
        dir = QString("%1%2G/Subgen %3/").arg(_path).arg(gen.num).arg(gen.subnum);
    }

    loadMoves(parent);
    loadMinLevels(parent);
    loadReleased(parent);
}

bool PokemonInfo::Gen::isReleased(const Pokemon::uniqueId &id)
{
    return m_Released.empty() || m_Released.contains(id);
}

QString PokemonInfo::Gen::path(const QString &fileName)
{
    return dir + fileName;
}

void PokemonInfo::Gen::loadMoves(Gen *parent)
{
    QStringList fileNames = QStringList() << path("tm_and_hm_moves.txt") << path("level_moves.txt") << path("special_moves.txt") << path("pre_evo_moves.txt");
    fileNames << path("egg_moves.txt") << path("tutor_moves.txt");

    if (gen >= 5) {
        fileNames << path("dw_moves.txt");
    }

    for (int i = 0; i < fileNames.count(); i++) {
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

            QSet<int> *refs[] = {
                &moves.TMMoves, &moves.levelMoves, &moves.specialMoves, &moves.preEvoMoves, &moves.eggMoves,
                &moves.tutorMoves, &moves.dreamWorldMoves
            };

            *refs[i] = data_set;
        }
    }

    QMutableHashIterator<Pokemon::uniqueId, PokemonMoves> it(m_Moves);
    while(it.hasNext()) {
        it.next();
        PokemonMoves &moves = it.value();

        moves.regularMoves = moves.TMMoves;
        moves.regularMoves.unite(moves.levelMoves).unite(moves.tutorMoves);
        moves.genMoves = moves.regularMoves;
        moves.genMoves.unite(moves.specialMoves).unite(moves.eggMoves).unite(moves.preEvoMoves);

        if (gen.num >= 5) {
            moves.genMoves.unite(moves.dreamWorldMoves);
        }

        if (parent) {
            if (parent->gen.num == gen.num) {
                if (parent->m_Moves.contains(it.key())) {
                    const PokemonMoves &pmoves = parent->m_Moves.value(it.key());

                    moves.regularMoves.unite(pmoves.regularMoves);
                    moves.genMoves.unite(pmoves.genMoves);
                    moves.eggMoves.unite(pmoves.eggMoves);
                    moves.preEvoMoves.unite(pmoves.preEvoMoves);
                    moves.specialMoves.unite(pmoves.specialMoves);
                    moves.TMMoves.unite(pmoves.TMMoves);
                    moves.levelMoves.unite(pmoves.levelMoves);
                    moves.tutorMoves.unite(pmoves.tutorMoves);
                    moves.dreamWorldMoves.unite(pmoves.dreamWorldMoves);
                }
            } else {
                if (parent->m_Moves.contains(it.key())) {
                    const PokemonMoves &pmoves = parent->m_Moves.value(it.key());

                    moves.genMoves.unite(pmoves.genMoves);
                }
            }
        }
    }

    foreach(Pokemon::uniqueId id, m_Moves.keys()) {
        if (id.isForme()) {
            continue;
        }
        foreach(Pokemon::uniqueId id, Formes(id, gen)) {
            if(!m_Moves.contains(id)) {
                m_Moves[id] = m_Moves.value(id.original());
            }
        }
    }
}

void PokemonInfo::Gen::addTradebacks(Gen *parent)
{
    QMutableHashIterator<Pokemon::uniqueId, PokemonMoves> it(m_Moves);
    while(it.hasNext()) {
        it.next();
        PokemonMoves &moves = it.value();
        PokemonMoves &moves2 = parent->m_Moves[it.key()];

        QSet<int> *refs[] = {
            &moves.TMMoves, &moves.levelMoves, &moves.specialMoves, &moves.preEvoMoves, &moves.eggMoves,
            &moves.tutorMoves, &moves.dreamWorldMoves
        };

        QSet<int> *refs2[] = {
            &moves2.TMMoves, &moves2.levelMoves, &moves2.specialMoves, &moves2.preEvoMoves, &moves2.eggMoves,
            &moves2.tutorMoves, &moves2.dreamWorldMoves
        };

        for (int i = 0; i < 7; i++) {
            foreach(int move, *refs2[i]) {
                if (MoveInfo::Exists(move, gen)) {
                    refs[i]->insert(move);
                }
            }
        }
    }
}

void PokemonInfo::RunMovesSanityCheck(int gen)
{
    qDebug() << "Running sanity check for gen " << gen;

    Pokemon::gen mg = Pokemon::gen(gen, -1);
    Pokemon::gen sg = Pokemon::gen(gen, GenInfo::NumberOfSubgens(gen)-1);

    loadGen(mg);
    loadGen(sg);

    Gen &_1 = gens[mg];
    Gen &_2 = gens[sg];

    foreach(Pokemon::uniqueId id, _1.m_Moves.keys()) {
        if (id.isForme() || !PokemonInfo::Exists(id,gen) || id.pokenum == Pokemon::Smeargle) {
            continue;
        }

        PokemonMoves &m1 = _1.m_Moves[id];
        PokemonMoves &m2 = _2.m_Moves[id];

        QSet<int> t1, t2;
        t1 = m1.regularMoves;
        //t1.unite(m1.dreamWorldMoves).unite(m1.specialMoves);
        t2 = m2.regularMoves;
        //t2.unite(m2.dreamWorldMoves).unite(m2.specialMoves);

        QSet<int> s1(t1), s2(t2);

        s1.subtract(t2);
        s2.subtract(t1);

        if (!s1.empty()) {
            foreach(int m, s1) {
                qDebug() << "Whole gen contains " << MoveInfo::Name(m) << " for " << PokemonInfo::Name(id) << " while subgens don't";
            }
        }
        if (!s2.empty()) {
            foreach(int m, s2) {
                qDebug() << "Subgens contains " << MoveInfo::Name(m) << " for " << PokemonInfo::Name(id) << " while whole gen doesn't";
            }
        }
    }
}

void PokemonInfo::Gen::loadReleased(Gen *parent)
{
    fill_container_with_file(m_Released, path("released.txt"));
    if (parent && parent->gen.num == gen.num) {
        m_Released.unite(parent->m_Released);
    }
}

void PokemonInfo::Gen::loadMinLevels(Gen *parent)
{
    if (parent && parent->gen.num == gen.num) {
        m_MinLevels = parent->m_MinLevels;
        m_MinEggLevels = parent->m_MinEggLevels;
    }

    QHash<Pokemon::uniqueId, QString> temp;
    fill_uid_str(temp, path("minlevels.txt"));

    QHashIterator<Pokemon::uniqueId, QString> it(temp);

    while (it.hasNext()) {
        it.next();
        m_MinLevels[it.key()] = it.value().section('/', 0, 0).toInt();
        m_MinEggLevels[it.key()] = it.value().section('/', -1, -1).toInt();
    }

    foreach(Pokemon::uniqueId id, m_MinLevels.keys()) {
        if (id.isForme()) {
            continue;
        }
        foreach(Pokemon::uniqueId id, PokemonInfo::Formes(id, gen)) {
            if (!m_MinLevels.contains(id)) {
                m_MinLevels[id] = m_MinLevels.value(id.original(), 100);
                m_MinEggLevels[id] = m_MinEggLevels.value(id.original(), 100);
            }
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


int PokemonInfo::TrueCount()
{
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
    if (pokeid == 0) {
        return true;
    }
    if(m_Names.contains(pokeid))
    {
        if (pokeid.isForme()) {
            return Exists(pokeid.original()) && Released(pokeid, gen);
        } else {
            return Exists(pokeid) && Released(pokeid, gen) && !PokemonInfo::Moves(pokeid, gen).empty();
        }
    } else {
        return false;
    }
}

bool PokemonInfo::Exists(const Pokemon::uniqueId &pokeid)
{
    return m_Names.contains(pokeid);
}

bool PokemonInfo::Released(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return PokemonInfo::gen(gen).isReleased(pokeid);
}

Pokemon::uniqueId PokemonInfo::Number(const QString &pokename)
{
    try {
        return pokenamesToIds.at(pokename);
    } catch (const std::out_of_range &) {
        return 0;
    }
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

    Pokemon::gen gen = GenInfo::GenMax();
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

    QPixmap ret;
    if (substitute) {
        ret = Sub(gen, back);
    } else {
        ret = Picture(num, gen, gender, shiny, back);
    }

    if (cropped) {
        QImage img = ret.toImage();
        cropImage(img);
        ret = QPixmap::fromImage(img);
    }

    return ret;
}

QPixmap PokemonInfo::Picture(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int gender, bool shiney, bool back, bool mod)
{
    QString archive = path("%1G/sprites.zip").arg(gen.num);

    if (mod && modPath.length() > 0 && QFile::exists(modPath+archive)) {
        archive.prepend(modPath);
    } else {
        mod = false;
    }

    QString file;

    if (gen.num == 1)
        file = QString("yellow/%2%1.png").arg(pokeid.toString(), back?"back/":"");
    else if (gen.num == 2)
        file = QString("crystal/%2%4%1.png").arg(pokeid.toString(), back?"back/":"", shiney?"shiny/":"");
    else if (gen.num ==3)
        file = QString("firered-leafgreen/%2%4%1.png").arg(pokeid.toString(), back?"back/":"", shiney?"shiny/":"");
    else if (gen.num == 4)
        file = QString("heartgold-soulsilver/%2%4%3%1.png").arg(pokeid.toString(), back?"back/":"", (gender==Pokemon::Female)?"female/":"", shiney?"shiny/":"");
    else if (gen.num == 5)
        file = QString("black-white/%2%4%3%1.png").arg(pokeid.toString(), back?"back/":"", (gender==Pokemon::Female)?"female/":"", shiney?"shiny/":"");
    else
        file = QString("x-y/%2%4%3%1.png").arg(pokeid.toString(), back?"back/":"", (gender==Pokemon::Female)?"female/":"", shiney?"shiny/":"");

    QPixmap ret;

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
    {
        /* temporary fix until we have all gen 6 sprites */
        if (gen.num == 6 && PokemonInfo::Exists(pokeid, Pokemon::gen(5,1))) {
            return PokemonInfo::Picture(pokeid, 5, gender, shiney, back);
        }

        if (gender == Pokemon::Female) {
            return PokemonInfo::Picture(pokeid, gen, Pokemon::Male, shiney, back);
        }
        if (mod) {
            return PokemonInfo::Picture(pokeid, gen, gender, shiney, back, false);
        }
        if (shiney) {
            return PokemonInfo::Picture(pokeid, gen, gender, false, back);
        }
        if (gen.num == 1) {
            return PokemonInfo::Picture(pokeid, 2, gender, shiney, back);
        } else if (gen.num == 2) {
            return PokemonInfo::Picture(pokeid, 3, gender, shiney, back);
        } else if (gen.num == 3) {
            return PokemonInfo::Picture(pokeid, 4, gender, shiney, back);
        } else if (gen.num == 4 || gen.num == 6) {
            return PokemonInfo::Picture(pokeid, 5, gender, shiney, back);
        }
        return ret;
    }

    ret.loadFromData(data, file.section(".", -1).toLatin1().data());
    QPixmapCache::insert(archive+file, ret);

    return ret;
}

QPixmap PokemonInfo::Sub(Pokemon::gen gen, bool back)
{
    QString archive = path("%1G/sprites.zip").arg(gen.num);

    QString file;

    if (gen <= 3) {
        file = QString("firered-leafgreen/%1substitute.png").arg(back?"back/":"");
    } else if (gen <= 4) {
        file = QString("heartgold-soulsilver/%1substitute.png").arg(back?"back/":"");
    } else if (gen <= 5) {
        file = QString("black-white/%1substitute.png").arg(back?"back/":"");
    } else {
        file = QString("x-y/%1substitute.png").arg(back?"back/":"");
    }

    QPixmap ret;

    if (QPixmapCache::find(archive+file, &ret)) {
        return ret;
    }

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0) {
        if (gen.num < GenInfo::GenMax()) {
            return Sub(gen.num + 1, back);
        } else {
            return ret;
        }
    }

    ret.loadFromData(data, "png");
    QPixmapCache::insert(archive+file, ret);

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
    QPixmapCache::insert(archive+file, p);
    return p;
}

QByteArray PokemonInfo::Cry(const Pokemon::uniqueId &pokeid, bool mod)
{
    quint16 num = pokeid.pokenum;
    QString archive = path("cries.zip");

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

QSet<int> PokemonInfo::Moves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).genMoves;
}

bool PokemonInfo::HasMoveInGen(const Pokemon::uniqueId &pokeid, int move, Pokemon::gen g)
{
    return gen(g).m_Moves[pokeid].regularMoves.contains(move) || gen(g).m_Moves[pokeid].specialMoves.contains(move)
            || gen(g).m_Moves[pokeid].eggMoves.contains(move) || gen(g).m_Moves[pokeid].preEvoMoves.contains(move);
}

QSet<int> PokemonInfo::RegularMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).regularMoves;
}

QSet<int> PokemonInfo::EggMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).eggMoves;
}

QSet<int> PokemonInfo::LevelMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).levelMoves;
}

QSet<int> PokemonInfo::TutorMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).tutorMoves;
}

QSet<int> PokemonInfo::TMMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).TMMoves;
}

QSet<int> PokemonInfo::SpecialMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).specialMoves;
}

QSet<int> PokemonInfo::PreEvoMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).preEvoMoves;
}

QSet<int> PokemonInfo::dreamWorldMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_Moves.value(pokeid).dreamWorldMoves;
}

PokemonInfo::Gen &PokemonInfo::gen(Pokemon::gen gen)
{
    /* Last gen of gen 1 (tradebacks) is special */
    if (!noWholeGen && gen.num != 1 && gen.subnum == GenInfo::NumberOfSubgens(gen.num)-1) {
        gen.subnum = gen.wholeGen;
    }
    /* Todo: load gens if needed somewhere smarter (for example the initialization of PokePersonal / PokeTeam with setGen).
      Instead of doing the checks every time we ask for the pokemon data... */
    loadGen(gen);

    return gens[gen];
}

AbilityGroup PokemonInfo::Abilities(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    AbilityGroup ret;

    for (int i = 0; i < 3; i++) {
        ret._ab[i] = m_Abilities[i][gen.num-GEN_MIN].value(pokeid);
    }

    return ret;
}

int PokemonInfo::Ability(const Pokemon::uniqueId &pokeid, int slot, Pokemon::gen gen)
{
    if (gen.num < GEN_MIN || gen.num-GEN_MIN >= m_Abilities[slot].size()) {
        return 0;
    }
    int ab = m_Abilities[slot][gen.num-GEN_MIN].value(pokeid, -1);
    return ab != -1 ? ab : m_Abilities[slot][gen.num-GEN_MIN].value(pokeid.original());
}


PokeBaseStats PokemonInfo::BaseStats(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    return m_BaseStats[gen.num-GEN_MIN].value(pokeid);
}

void PokemonInfo::loadNames()
{
    /* The need for options prevents us from using fill_uid_str */
    QStringList temp;
    fill_container_with_file(temp, path("pokemons.txt"), true);

    m_Names.clear();
    pokenamesToIds.clear();
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
            pokenamesToIds[name] = id;
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

bool PokemonInfo::IsMegaEvo(const Pokemon::uniqueId &pokeid)
{
    return m_Options.value(pokeid).contains('M');
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

int PokemonInfo::MinLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_MinLevels.value(pokeid, 100);
}

int PokemonInfo::MinEggLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen g)
{
    return gen(g).m_MinEggLevels.value(pokeid, 100);
}

int PokemonInfo::AbsoluteMinLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen)
{
    int limit = (gen >= 3 ? 3 : GEN_MIN);

    int min = 100;

    do {
        int level = MinLevel(pokeid, gen);

        if (level < min) {
            min = level;
        }

        gen = Pokemon::gen(gen.num - 1, gen.wholeGen);
    } while (gen >= limit);

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

QString PokemonInfo::path(const QString &filename, const Pokemon::gen &g)
{
    if (g != 0) {
        return QString("%1/%2G/%3").arg(m_Directory).arg(g.num).arg(filename);
    } else {
        return m_Directory + filename;
    }
}

QList<Pokemon::uniqueId> PokemonInfo::AllIds()
{
    return m_Names.keys();
}

void PokemonInfo::loadEvos()
{
    QHash<int, QList<int> > &evos = m_Evolutions;

    QStringList files = allFiles(path("evos.txt"));

    foreach(QString file, files) {
        foreach(QString s, QString::fromUtf8(getFileContent(file)).trimmed().split('\n')) {
            QStringList evs = s.split(' ');
            int num = evs[0].toInt();

            /* It's normal to start from 0 */
            foreach(QString ev, evs) {
                int n = ev.toInt();

                if (n != num)
                    m_PreEvos[n] = num;

                evos[num].push_back(n);
            }
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
        // Other.
        if(!m_LevelBalance.contains(id)) {
            m_LevelBalance[id] = m_LevelBalance.value(OriginalForme(id), 1);
        }
        if(!m_Genders.contains(id)) {
            m_Genders[id] = m_Genders.value(OriginalForme(id), Pokemon::NeutralAvail);
        }

        for (int gen = GEN_MIN; gen <= GenInfo::GenMax(); gen++) {
            int i = gen-GEN_MIN;

            if (!Exists(id, Pokemon::gen(gen, -1)))
                continue;

            for (int j = 0; j < 3; j++) {
                if(!m_Abilities[j][i].contains(id)) {
                    m_Abilities[j][i][id] = m_Abilities[j][i].value(id.original(), Ability::NoAbility);
                }
            }

            // Base stats.
            if(!m_BaseStats[i].contains(id)) {
                m_BaseStats[i][id] = m_BaseStats[i].value(OriginalForme(id));
                if (id != OriginalForme(id))
                    m_AestheticFormes.insert(id);
            }

            if(!m_Type1[i].contains(id)) {
                m_Type1[i][id] = m_Type1[i].value(id.original(), Pokemon::Normal);
            }
            if(!m_Type2[i].contains(id)) {
                m_Type2[i][id] = m_Type2[i].value(id.original(), Pokemon::Curse);
            }
        }
    }
}

Pokemon::uniqueId PokemonInfo::getRandomPokemon(Pokemon::gen gen)
{
    int total = TrueCount();
    Pokemon::uniqueId poke;

    while (poke == 0) {
        int random = (true_rand() % (total-1)) + 1;

        poke = Pokemon::uniqueId (random, 0);
        if (!PokemonInfo::Exists(poke, gen) || !PokemonInfo::Released(poke, gen)) {
            poke = 0;
        }
    }

    if(HasFormes(poke)) {
        QList<Pokemon::uniqueId> formesList = VisibleFormes(poke, gen);
        /* The pokemon doesn't always have visible formes */
        if (formesList.count() > 0)
            poke = formesList.value(true_rand() %  formesList.count());
    }

    return Pokemon::uniqueId(poke);
}

void MoveInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

    loadNames();
    loadMoveMessages();
    loadDetails();

    m_GenMoves.clear();
    m_GenMoves.resize(GenInfo::NumberOfGens());

    fill_double(m_OldMoves, path("oldmoves.txt"));

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        fill_container_with_file(m_GenMoves[i-GenInfo::GenMin()], path("%1G/moves.txt").arg(i));

        for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
            Pokemon::gen g (i, j);
            gens[g].load(dir, g);

            if (g.subnum != 0) {
                gens[g].parent = &gens[Pokemon::gen(g.num, g.subnum-1)];
            }
        }
    }
}

void MoveInfo::Gen::load(const QString &dir, Pokemon::gen gen)
{
    this->gen = gen;

    if (gen.subnum == 0) {
        this->dir = QString("%1%2G/").arg(dir).arg(gen.num);
    } else {
        this->dir = QString("%1%2G/Subgen %3/").arg(dir).arg(gen.num).arg(gen.subnum);
    }

    fill_int_char(accuracy, path("accuracy.txt"));
    fill_int_char(category, path("category.txt"));
    fill_int_char(causedEffect, path("caused_effect.txt"));
    fill_int_char(critRate, path("crit_rate.txt"));
    fill_int_char(damageClass, path("damage_class.txt"));
    fill_int_str(effect, path("effect.txt"), true);
    fill_int_str(specialEffect, path("special_effect.txt"));
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
    fill_int_bool(kingRock, path("king_rock.txt"));

    /* Removing comments, aka anything starting from '#' */
    QMutableHashIterator<int,QString> it(specialEffect);
    while(it.hasNext()) {
        it.next();
        it.value() = it.value().section('#', 0, 0);
    }

    /* Not needed because HM pokemon can be traded between gens got gen 1 & 2*/
    //    if (gen == 1) {
    //        HMs << Move::Cut << Move::Flash << Move::Surf << Move::Strength <<Move::Fly;
    //    } else if (gen == 2) {
    //        HMs << Move::Cut << Move::Flash << Move::Surf << Move::Strength << Move::Whirlpool
    //                        << Move::Waterfall << Move::Fly;
    //    }

    if (gen.num == 3) {
        HMs << Move::Cut << Move::Flash << Move::Surf << Move::RockSmash << Move::Strength << Move::Dive
            << Move::Waterfall << Move::Fly;
    } else if (gen.num == 4) {
        HMs << Move::Cut << Move::Surf << Move::RockSmash << Move::Strength
            << Move::Waterfall << Move::Fly << Move::RockClimb;
    } else if (gen.num == 5) {
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

bool MoveInfo::isInit() {
    return !m_Names.empty();
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

QString MoveInfo::Name(int movenum)
{
    return m_Names.value(movenum, m_Names.value(0));
}

#define move_find(var, mv, g) do {\
    Gen *G = &gens[g]; \
    while (!G->var.contains(mv) && G->parent != 0) { \
    G = G->parent; \
    } \
    return G->var.value(mv);\
    } while(0)

#define move_find2(type, res, var, mv, g) \
    Gen *G = &gens[g]; \
    while (!G->var.contains(mv) && G->parent != 0) { \
    G = G->parent; \
    } \
    type res = G->var.value(mv)

int MoveInfo::Type(int mv, Pokemon::gen g)
{
    move_find(type, mv, g);
}

int MoveInfo::ConvertFromOldMove(int oldmovenum)
{
    return m_OldMoves[oldmovenum];
}

int MoveInfo::Category(int movenum, Pokemon::gen g)
{
    if (g >= 4)
        move_find(damageClass, movenum, g);

    if (Power(movenum, g) == 0)
        return Move::Other;

    return TypeInfo::Category(Type(movenum, g));
}

int MoveInfo::Classification(int movenum, Pokemon::gen g)
{
    move_find(category, movenum, g);
}

bool MoveInfo::FlinchByKingRock(int movenum, Pokemon::gen gen)
{
    move_find(kingRock, movenum, gen);
}

int MoveInfo::Number(const QString &movename)
{
    return m_LowerCaseMoves.value(movename.toLower());
}

int MoveInfo::NumberOfMoves()
{
    return m_Names.count();
}

int MoveInfo::NumberOfMoves(Pokemon::gen g)
{
    return m_GenMoves[g.num-GenInfo::GenMin()].count();
}

int MoveInfo::FlinchRate(int movenum, Pokemon::gen g)
{
    move_find(flinchChance, movenum, g);;
}

int MoveInfo::Recoil(int movenum, Pokemon::gen g)
{
    move_find(recoil, movenum, g);
}

QString MoveInfo::Description(int movenum, Pokemon::gen g)
{
    move_find2(QString, r, effect, movenum, g);
    r.replace("$effect_chance", QString::number(EffectRate(movenum, g)));

    return r;
}

int MoveInfo::Power(int movenum, Pokemon::gen g)
{
    move_find(power, movenum, g);
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
    move_find(pp, movenum, g);
}

int MoveInfo::Acc(int movenum, Pokemon::gen g)
{
    move_find(accuracy, movenum, g);
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
    move_find(critRate, movenum, g);
}

int MoveInfo::RepeatMin(int movenum, Pokemon::gen g)
{
    move_find2(int, res, minMaxHits, movenum, g);
    return res & 0xF;
}

int MoveInfo::RepeatMax(int movenum, Pokemon::gen g)
{
    move_find2(int, res, minMaxHits, movenum, g);
    return res >> 4;
}

int MoveInfo::SpeedPriority(int movenum, Pokemon::gen g)
{
    move_find(priority, movenum, g);
}

int MoveInfo::Flags(int movenum, Pokemon::gen g)
{
    move_find(flags, movenum, g);
}

bool MoveInfo::Exists(int movenum, Pokemon::gen g)
{
    return m_GenMoves[g.num-GenInfo::GenMin()].contains(movenum);
}

bool MoveInfo::isOHKO(int movenum, Pokemon::gen gen)
{
    return Classification(movenum, gen) == Move::OHKOMove;
}

bool MoveInfo::isHM(int movenum, Pokemon::gen g)
{
    return gens[g].HMs.contains(movenum);
}

int MoveInfo::EffectRate(int movenum, Pokemon::gen g)
{
    move_find(effectChance, movenum, g);
}

quint32 MoveInfo::StatAffected(int movenum, Pokemon::gen g)
{
    move_find(stataffected, movenum, g);
}

quint32 MoveInfo::BoostOfStat(int movenum, Pokemon::gen g)
{
    move_find(statboost, movenum, g);
}

quint32 MoveInfo::RateOfStat(int movenum, Pokemon::gen g)
{
    move_find(statrate, movenum, g);
}

int MoveInfo::Target(int movenum, Pokemon::gen g)
{
    move_find(range, movenum, g);
}

int MoveInfo::Healing(int movenum, Pokemon::gen g)
{
    move_find(healing, movenum, g);
}

int MoveInfo::MinTurns(int movenum, Pokemon::gen g)
{
    move_find(minTurns, movenum, g);
}

int MoveInfo::MaxTurns(int movenum, Pokemon::gen g)
{
    move_find(maxTurns, movenum, g);
}

int MoveInfo::Status(int movenum, Pokemon::gen g)
{
    move_find(causedEffect, movenum, g);
}

int MoveInfo::StatusKind(int movenum, Pokemon::gen g)
{
    move_find(status, movenum, g);
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
    move_find(specialEffect, movenum, gen);
}

QSet<int> MoveInfo::Moves(Pokemon::gen gen)
{
    return m_GenMoves[gen.num-GenInfo::GenMin()];
}

QString MoveInfo::DetailedDescription(int movenum)
{
    return m_Details[movenum];
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}

#undef move_find
#undef move_find2

///////////////////////////////////////////////////////
/////////////// ITEMINFO //////////////////////////////
///////////////////////////////////////////////////////

void ItemInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

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
    fill_int_bool(m_UsefulItems, path("item_useful.txt"));
    fill_int_bool(m_UsefulBerries, path("berry_useful.txt"));

    m_GenItems.clear();
    m_GenItems.resize(GenInfo::NumberOfGens());

    for (int g = GEN_MIN_ITEMS; g <= GenInfo::GenMax(); g++) {
        int i = g-GEN_MIN;
        fill_container_with_file(m_GenItems[i], path(QString("%1G/released_items.txt").arg(g)));

        QList<int> tempb;
        fill_container_with_file(tempb, path(QString("%1G/released_berries.txt").arg(g)));
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

    QHashIterator<int, QString> it2(m_BerryNames);
    while (it2.hasNext()) {
        it2.next();
        m_BerryNamesH.insert(it2.value(), it2.key()+8000);
    }

    m_SortedNames.clear();
    m_SortedUsefulNames.clear();
    m_SortedNames.resize(GenInfo::NumberOfGens());
    m_SortedUsefulNames.resize(GenInfo::NumberOfGens());

    QList<QString> sortedNames;
    sortedNames << m_RegItemNames.values() << m_BerryNames.values();
    qSort(sortedNames);

    QList<QString> sortedUsefulNames;

    {
        QHashIterator<int, QString> it(m_RegItemNames);
        while (it.hasNext()) {
            it.next();
            if (isUseful(it.key()))
                sortedUsefulNames.push_back(it.value());
        }
    }
    {
        QHashIterator<int, QString> it(m_BerryNames);
        while (it.hasNext()) {
            it.next();
            if (isUseful(it.key()+8000))
                sortedUsefulNames.push_back(it.value());
        }
    }
    qSort(sortedUsefulNames);

    for (int j = GenInfo::GenMax(); j >= GEN_MIN_ITEMS; j--) {
        int g = j-GEN_MIN;
        for (int i = 0; i < sortedNames.size(); i++) {
            if (Exists(Number(sortedNames[i]), j))
                m_SortedNames[g].push_back(sortedNames[i]);
        }

        for (int i = 0; i < sortedUsefulNames.size(); i++) {
            if (Exists(Number(sortedUsefulNames[i]), j))
                m_SortedUsefulNames[g].push_back(sortedUsefulNames[i]);
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
    m_RegEffects.clear();
    m_RegEffects.resize(GenInfo::NumberOfGens());

    for (int i = GEN_MIN_ITEMS; i <= GenInfo::GenMax(); i++) {
        QMultiHash<int,QString> temp;
        fill_int_str(temp, path("%1G/item_effects.txt").arg(i));

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
            m_RegEffects[i-GEN_MIN][it.key()] += toPush;
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
                toPush.push_back(Effect(atoi(s.c_str())+8000, eff.mid(pos+1)));
            } else {
                toPush.push_back(Effect(atoi(s.c_str())+8000));
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
    return m_SortedNames[GenInfo::GenMax()-GEN_MIN].size();
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
    QPixmapCache::insert(archive+file, ret);
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
    QPixmapCache::insert(archive+file, ret);
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

bool ItemInfo::Exists(int itemnum)
{
    return isBerry(itemnum) ? m_BerryNames.contains(itemnum-8000) : m_RegItemNames.contains(itemnum);
}

bool ItemInfo::isBerry(int itemnum)
{
    return itemnum >= 8000;
}

bool ItemInfo::isPlate(int itemnum)
{
    return ((itemnum >= 185 && itemnum <= 202 && itemnum != 190 && itemnum != 200) || itemnum==330);
}

bool ItemInfo::isMegaStone(int itemnum)
{
    return itemnum >= 2000 && itemnum < 3000;
}

bool ItemInfo::isDrive(int itemnum)
{
    return itemnum == Item::DouseDrive || itemnum == Item::BurnDrive || itemnum == Item::ChillDrive || itemnum == Item::ShockDrive;
}

bool ItemInfo::isGem(int itemnum)
{
    return itemnum == Item::NormalGem || itemnum == Item::FightGem || itemnum == Item::SteelGem || itemnum == Item::PsychicGem || itemnum == Item::DarkGem || itemnum == Item::FireGem || itemnum == Item::WaterGem || itemnum == Item::ElectricGem || itemnum == Item::IceGem || itemnum == Item::FlightGem || itemnum == Item::PoisonGem || itemnum == Item::GhostGem || itemnum == Item::BugGem || itemnum == Item::GrassGem || itemnum == Item::RockGem || itemnum == Item::EarthGem || itemnum == Item::DragonGem;
}

bool ItemInfo::isMail(int itemnum)
{
    return (itemnum >= 214 && itemnum <= 226);
}

bool ItemInfo::isUseful(int itemnum)
{
    if (isBerry(itemnum)) {
        return m_UsefulBerries.isEmpty() || m_UsefulBerries.value(itemnum - 8000) == true;
    } else {
        return m_UsefulItems.isEmpty() || m_UsefulItems.value(itemnum) == true;
    }
}

int ItemInfo::PlateType(int itemnum)
{
    const auto &effects = Effects(itemnum, GenInfo::GenMax());
    if (effects.size() == 0) {
        return 0;
    }
    return effects.front().args.toInt();
}

Pokemon::uniqueId ItemInfo::MegaStoneForme(int itemnum)
{
    const auto &effects = Effects(itemnum, GenInfo::GenMax());
    if (effects.size() == 0) {
        return 0;
    }
    return Pokemon::uniqueId(effects.front().args);
}

int ItemInfo::PlateForType(int type)
{
    static const int plates[] = {
        Item::NoItem,
        Item::FistPlate,
        Item::SkyPlate,
        Item::ToxicPlate,
        Item::EarthPlate,
        Item::StonePlate,
        Item::InsectPlate,
        Item::SpookyPlate,
        Item::IronPlate,
        Item::FlamePlate,
        Item::SplashPlate,
        Item::MeadowPlate,
        Item::ZapPlate,
        Item::MindPlate,
        Item::IciclePlate,
        Item::DracoPlate,
        Item::DreadPlate,
        Item::PixiePlate,
        Item::NoItem
    };
    return plates[type];
}

int ItemInfo::DriveType(int itemnum)
{
    return Effects(itemnum, GenInfo::GenMax()).front().args.toInt();
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

int ItemInfo::DriveForForme(int forme)
{
    switch(forme) {
    case 1: return Item::DouseDrive;
    case 2: return Item::ShockDrive;
    case 3: return Item::BurnDrive;
    case 4: return Item::ChillDrive;
    case 0: default: return Item::NoItem;
    }
}

bool ItemInfo::IsBattleItem(int itemnum, Pokemon::gen gen)
{
    if (isBerry(itemnum))  {
        return true;
    }
    if (!Exists(itemnum, gen)) {
        return false;
    }

    QList<Effect> l = m_RegEffects[gen.num-GenInfo::GenMin()].value(itemnum);

    if (l.length() == 0) {
        return false;
    }

    foreach(const Effect &e, l) {
        int num = e.num;

        if (num >= 1000 && num <= 3999) {
            return true;
        }
    }

    return false;
}

int ItemInfo::Target(int itemnum, Pokemon::gen gen)
{
    if (isBerry(itemnum)) {
        return Item::TeamPokemon;
    }
    if (!IsBattleItem(itemnum, gen)) {
        return Item::NoTarget;
    }
    QList<Effect> l = m_RegEffects[gen.num-GenInfo::GenMin()].value(itemnum);

    foreach(const Effect &e, l) {
        int num = e.num;

        if (!(num >= 1000 && num <= 3999)) {
            continue;
        }

        /* Dire Hit, X Attack, ... */
        if (num >= 2000 && num < 3000) {
            return Item::FieldPokemon;
        } else if (num >= 3000) {
            /* Poke Ball */
            return Item::Opponent;
        } else {
            /* Sacred Ash */
            if (num == 1999) {
                return Item::Team;
            } else {
                /* Ether */
                if (num == 1004) {
                    return Item::Attack;
                } else {
                    return Item::TeamPokemon;
                }
            }
        }
    }

    return Item::NoTarget;
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
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

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
    m_TypeVsType.clear();
    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++)
    {
        QHash<int, QString> temp;

        fill_int_str(temp, path("%1G/typestable.txt").arg(i));

        QHashIterator<int, QString> it(temp);
        m_TypeVsType.push_back(QHash<int, QVector<int> >());
        while (it.hasNext()) {
            it.next();
            QStringList l2 = it.value().split(' ');
            foreach (QString l3, l2) {
                m_TypeVsType.back()[it.key()].push_back(l3.toInt());
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
    return m_TypeVsType[gen.num-GenInfo::GenMin()][type_attack][type_defend];
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

void NatureInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

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
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

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
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

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
    m_Effects.clear();
    m_Effects.resize(GenInfo::NumberOfGens());

    for (int i = GEN_MIN_ABILITIES; i <= GenInfo::GenMax(); i++) {
        QHash<int, QString> m_temp;
        fill_int_str(m_temp,path("ability_effects_%1G.txt").arg(i));

        QHashIterator<int, QString> it(m_temp);
        while (it.hasNext()) {
            it.next();
            QStringList content = it.value().section('#', 0, 0).split('-');
            if (content.size() == 1) {
                m_Effects[i-GEN_MIN].insert(it.key(), Effect(content[0].toInt()));
            } else {
                m_Effects[i-GEN_MIN].insert(it.key(), Effect(content[0].toInt(), content[1]));
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

    if (g == GenInfo::GenMax()) {
        return total;
    } else {
        int hc;
        if (g <= 2) {
            hc = 1; //No Ability
        } else if (g <= 3) {
            hc = 77; //Air lock
        } else if (g <= 4 || 1) {
            hc = 124; //Bad dreams
        }
        return std::min(hc, total); //safety check, in case db was edited
    }
}

void GenderInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

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
    else if (gen <= 5)
        return (((satt_dv>>3) + (speed_dv>>3)*2 + (def_dv>>3)*4 + (att_dv>>3)*8)*5+ std::min(int(satt_dv),3))/2 + 31;
    else
        return 60;
}

QList<QStringList> HiddenPowerInfo::PossibilitiesForType(int type, Pokemon::gen gen)
{
    QList<QStringList> ret;

    for (int i = 63; i >= 0; i--) {
        int gt = Type(gen, i & 1, (i & 2)!=0, (i & 4)!=0, (i & 8)!=0, (i & 16)!=0, (i & 32)!=0);
        if (gt == type) {
            ret.push_back(QString("%1 %2 %3 %4 %5 %6")
                          .arg(((i&1)!=0)+30).arg(((i&2)!=0)+30).arg(((i&4)!=0)+30)
                          .arg(((i&8)!=0)+30).arg(((i&16)!=0)+30).arg(((i&32)!=0)+30).split(' '));
        }
    }

    return ret;
}

QPair<quint8, quint8> HiddenPowerInfo::AttDefDVsForGen2(int type)
{
    return QPair<quint8, quint8>(12+ (type-1)/4, 12 + ((type-1)%4));
}

void StatInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

    fill_int_str(m_stats, path("stats.txt"), true);
    fill_int_str(m_status, path("status.txt"), true);
}

void StatInfo::retranslate()
{
    fill_int_str(m_stats, path("stats.txt"), true);
    fill_int_str(m_status, path("status.txt"), true);
}

QString StatInfo::Stat(int stat, const Pokemon::gen & gen)
{
    if ( (stat == SpAttack || stat == SpDefense) && gen.num == 1) {
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

void GenInfo::init(const QString &dir)
{
    m_Directory = dir;

#ifndef QT5
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

    fill_gen_string(m_versions, path("versions.txt"), true);
    fill_int_str(m_gens, path("gens.txt"), true);

    m_NumberOfSubgens.clear();
    genMax = genMin = m_gens.begin().key();

    foreach(Pokemon::gen g, m_versions.keys()) {
        m_NumberOfSubgens[g.num] += 1;

        if (genMax < g.num) {
            genMax = g.num;
        }

        if (genMin > g.num) {
            genMin = g.num;
        }
    }
}

int GenInfo::NumberOfGens() {
    return m_gens.size();
}

bool GenInfo::Exists(const Pokemon::gen &gen) {
    return m_versions.contains(gen);
}

int GenInfo::NumberOfSubgens(int gen) {
    return m_NumberOfSubgens.value(gen);
}

QList<int> GenInfo::AllGens()
{
    return m_gens.keys();
}

QList<Pokemon::gen> GenInfo::AllSubGens()
{
    return m_versions.keys();
}

void GenInfo::retranslate()
{
    fill_gen_string(m_versions, path("versions.txt"), true);
    fill_int_str(m_gens, path("gens.txt"), true);
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
