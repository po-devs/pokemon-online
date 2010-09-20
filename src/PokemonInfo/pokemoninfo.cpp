namespace Pokemon {
    class uniqueId;
}
unsigned int qHash (const Pokemon::uniqueId &key);

#include "pokemoninfo.h"
#include "pokemonstructs.h"

#include "../../SpecialIncludes/zip.h"
#include "../Utilities/functions.h"

/*initialising static variables */
QString PokemonInfo::m_Directory;
QMap<Pokemon::uniqueId, QString> PokemonInfo::m_Names;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Weights;
QHash<int, QHash<quint16, QString> > PokemonInfo::m_Desc;
QHash<int, QString> PokemonInfo::m_Classification;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Height;

QHash<Pokemon::uniqueId, int> PokemonInfo::m_Genders;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Type1[3];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Type2[3];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Abilities[3][3];
QHash<Pokemon::uniqueId, PokeBaseStats> PokemonInfo::m_BaseStats;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_LevelBalance;
QHash<Pokemon::uniqueId, PokemonMoves> PokemonInfo::m_Moves;
QHash<int, quint16> PokemonInfo::m_MaxForme;
QHash<Pokemon::uniqueId, QString> PokemonInfo::m_Options;
int PokemonInfo::m_trueNumberOfPokes;
QSet<Pokemon::uniqueId> PokemonInfo::m_AestheticFormes;

QHash<int, QList<int> > PokemonInfo::m_Evolutions;
QHash<int, int> PokemonInfo::m_OriginalEvos;
QList<Pokemon::uniqueId> PokemonInfo::m_VisiblePokesPlainList;
QHash<int, int> PokemonInfo::m_PreEvos;

QString MoveInfo::m_Directory;
MoveInfo::Gen MoveInfo::gens[Version::NumberOfGens];
QList<QString> MoveInfo::m_Names;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;
QList<QStringList> MoveInfo::m_MoveMessages;
QList<QString> MoveInfo::m_Details;
QList<QString> MoveInfo::m_SpecialEffects;

QString ItemInfo::m_Directory;
QList<QString> ItemInfo::m_BerryNames;
QList<QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QList<QString> ItemInfo::m_SortedNames[3];
QList<QString> ItemInfo::m_SortedUsefulNames[3];
QList<QList<ItemInfo::Effect> > ItemInfo::m_RegEffects[3];
QList<QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QList<QStringList> ItemInfo::m_RegMessages;
QList<QStringList> ItemInfo::m_BerryMessages;
QList<int> ItemInfo::m_Powers;
QList<int> ItemInfo::m_BerryPowers;
QList<int> ItemInfo::m_BerryTypes;
QList<int> ItemInfo::m_UsefulItems;
QSet<int> ItemInfo::m_GenItems[3];

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
QList<AbilityInfo::Effect> AbilityInfo::m_Effects[3];
QList<QStringList> AbilityInfo::m_Messages;

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

static void fill_container_with_file(QStringList &container, const QString & filename)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
    {
        container << filestream.readLine();
    }
}

static void fill_container_with_file(QList<QString> &container, const QString & filename)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    while (!filestream.atEnd() && filestream.status() != QTextStream::ReadCorruptData)
    {
        container << filestream.readLine();
    }
}

static void fill_container_with_file(QVector<char> &container, const QString & filename)
{
    QFile file(filename);

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

static void fill_container_with_file(QVector<unsigned char> &container, const QString & filename)
{
    QFile file(filename);

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

static void fill_uid_int(QHash<Pokemon::uniqueId, int> &container, const QString &filename)
{
    QFile file(filename);
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

template <class T>
static void fill_container_with_file(T &container, const QString & filename)
{
    QFile file(filename);

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

QString PokemonInfo::Height(const Pokemon::uniqueId &pokeid)
{
    return m_Height.value(pokeid, "0.0");
}

int PokemonInfo::Type1(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Type1[gen-3].value(pokeid);
}

int PokemonInfo::Type2(const Pokemon::uniqueId &pokeid,int gen)
{
    return m_Type2[gen-3].value(pokeid);
}

int PokemonInfo::calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev)
{
    return ((2*basestat + dv+ ev/4)*level)/100 + 5;
}

int PokemonInfo::Stat(const Pokemon::uniqueId &pokeid, int stat, int level, quint8 dv, quint8 ev)
{
    quint8 basestat = PokemonInfo::BaseStats(pokeid).baseStat(stat);
    if (stat == Hp) {
        /* Formely direct check for Shedinja */
        if(m_Options.value(pokeid).contains('1')) {
            return 1;
        }else{
            return calc_stat(basestat, level, dv, ev) + level + 5;
        }
    }
	return calc_stat(basestat, level, dv, ev);
}

int PokemonInfo::FullStat(const Pokemon::uniqueId &pokeid, int nature, int stat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp) {
        return Stat(pokeid, stat, level, dv, ev);
    }
    else {
        return Stat(pokeid, stat, level, dv, ev) * (10+NatureInfo::Boost(nature, stat)) / 10;
    }
}

void PokemonInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfPokemons() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadEvos();
    loadMoves();

    fill_uid_int(m_Genders, path("poke_gender.txt"));

    for (int i = 0; i < 3; i++) {
        int gen = i+3;

        fill_uid_int(m_Type1[i], path(QString("poke_type1-%1G.txt").arg(gen)));
        fill_uid_int(m_Type2[i], path(QString("poke_type2-%1G.txt").arg(gen)));
        for (int j = 0; j < 3; j++) {
            fill_uid_int(m_Abilities[i][j], path(QString("poke_ability%1_%2G.txt").arg(j+1).arg(gen)));
        }
    }

    fill_uid_int(m_LevelBalance, path("level_balance.txt"));
    loadClassifications();
    loadHeights();
    loadDescriptions();
    loadBaseStats();
    makeDataConsistent();
}

void PokemonInfo::loadClassifications()
{
    QStringList temp;
    fill_container_with_file(temp, trFile(path("classification")));
    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
        QString description;
        quint16 pokenum;
        bool ok = Pokemon::uniqueId::extract_short(current, pokenum, description);
        if(ok) m_Classification[pokenum] = description;
    }
}

void PokemonInfo::loadHeights()
{
    QStringList temp;
    fill_container_with_file(temp, path("height.txt"));
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
        fill_container_with_file(temp, trFile(path("description_%1").arg(carts[i])));
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
    if(Exists(pokeid))
    {
        return m_Names.value(pokeid);
    }else{
        return m_Names.value(Pokemon::uniqueId());
    }
}

bool PokemonInfo::Exists(const Pokemon::uniqueId &pokeid, int gen)
{
    if(m_Names.contains(pokeid))
    {
        switch(gen)
        {
        case 3:
            return pokeid.pokenum <= 386;
        }
        return true;
    }else{
        return false;
    }
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

QPixmap PokemonInfo::Picture(const Pokemon::uniqueId &pokeid, int gen, int gender, bool shiney, bool back)
{
    QString archive;
    if (gen <= 4)
        archive = path("poke_img.zip");
    else
        archive = path("black_white.zip");

    QString file;

    if (gen ==3)
        file = QString("%1/%2%3.png").arg(pokeid.toString(), back?"3Gback":"RFLG", shiney?"s":"");
    else if (gen == 4)
        file = QString("%1/DP%2%3%4.png").arg(pokeid.toString(), back?"b":"", (gender==Pokemon::Female)?"f":"m", shiney?"s":"");
    else
        file = QString("%1/%2%3%4.png").arg(pokeid.toString(), back?"back":"front", (gender==Pokemon::Female)?"f":"", shiney?"s":"");

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
        return QPixmap();
    }

    QPixmap ret;
    ret.loadFromData(data, "png");

    return ret;
}

QPixmap PokemonInfo::Sub(int gen, bool back)
{
    QString archive = path("poke_img.zip");

    QString file = QString("sub%1%2.png").arg(back?"b":"").arg(gen>=4?"":"3G");

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
        return QPixmap();

    QPixmap ret;
    ret.loadFromData(data, "png");

    return ret;
}

QPixmap PokemonInfo::Icon(const Pokemon::uniqueId &pokeid)
{
    QString archive = path("icons.zip");
    QString file = QString("%1.png").arg(pokeid.toString());

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        if (IsForme(pokeid)) {
            return Icon(OriginalForme(pokeid));
        }

        qDebug() << "error loading icon";
        return QPixmap();
    }
    QPixmap p;
    p.loadFromData(data,"png");
    return p;
}

QByteArray PokemonInfo::Cry(const Pokemon::uniqueId &pokeid)
{
    quint16 num = pokeid.pokenum;
    QString archive = path("cries.zip");
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
    return m_Moves.value(pokeid).genMoves[gen-3];
}

QSet<int> PokemonInfo::RegularMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).regularMoves[gen-3];
}

QSet<int> PokemonInfo::EggMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).eggMoves[gen-3];
}

QSet<int> PokemonInfo::LevelMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).levelMoves[gen-3];
}

QSet<int> PokemonInfo::TutorMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).tutorMoves[gen-3];
}

QSet<int> PokemonInfo::TMMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).TMMoves[gen-3];
}

QSet<int> PokemonInfo::SpecialMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).specialMoves[gen-3];
}

QSet<int> PokemonInfo::PreEvoMoves(const Pokemon::uniqueId &pokeid, int gen)
{
    return m_Moves.value(pokeid).preEvoMoves[gen-3];
}

AbilityGroup PokemonInfo::Abilities(const Pokemon::uniqueId &pokeid, int gen)
{
    AbilityGroup ret;

    for (int i = 0; i < 3; i++) {
        ret._ab[i] = m_Abilities[gen-3][i].value(pokeid);
    }

    return ret;
}

void PokemonInfo::loadBaseStats()
{
    QStringList temp;
    fill_container_with_file(temp, path("poke_stats.txt"));

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

void PokemonInfo::loadNames()
{
    QStringList temp;
    fill_container_with_file(temp, trFile(path("pokemons")));

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
    fill_container_with_file(temp, path("poke_weight.txt"));
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

QList<Pokemon::uniqueId> PokemonInfo::Formes(const Pokemon::uniqueId &pokeid)
{
    QList<Pokemon::uniqueId> result;
    for(quint16 i = 0; i <= NumberOfAFormes(pokeid); i++) {
        result.append(Pokemon::uniqueId(pokeid.pokenum, i));
    }
    return result;
}

Pokemon::uniqueId PokemonInfo::OriginalEvo(const Pokemon::uniqueId &pokeid)
{
    return Pokemon::uniqueId(m_OriginalEvos.value(pokeid.pokenum), 0);
}

int PokemonInfo::PreEvo(int pokenum)
{
    return m_PreEvos.value(pokenum);
}

QList<int> PokemonInfo::Evos(int pokenum)
{
    return m_Evolutions.value(OriginalEvo(Pokemon::uniqueId(pokenum, 0)).pokenum);
}

bool PokemonInfo::IsInEvoChain(const Pokemon::uniqueId &pokeid)
{
    return Evos(pokeid.pokenum).size() > 1;
}

void PokemonInfo::loadMoves()
{
    static const int filesize = 17;

    QString fileNames[filesize] = {
        path("3G_tm_and_hm_moves.txt"), path("3G_egg_moves.txt"), path("3G_level_moves.txt"),
        path("3G_tutor_moves.txt"), path("3G_special_moves.txt"), path("4G_tm_and_hm_moves.txt"),
        path("4G_pre_evo_moves.txt"), path("4G_egg_moves.txt"), path("4G_level_moves.txt"),
        path("4G_tutor_moves.txt"), path("4G_special_moves.txt"), path("5G_tm_and_hm_moves.txt"),
        path("5G_pre_evo_moves.txt"), path("5G_egg_moves.txt"), path("5G_level_moves.txt"),
        path("5G_tutor_moves.txt"), path("5G_special_moves.txt")
    };

    for (int i = 0; i < filesize; i++) {
        QStringList temp;
        fill_container_with_file(temp, fileNames[i]);
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
                    &moves.TMMoves[0], &moves.eggMoves[0], &moves.levelMoves[0], &moves.tutorMoves[0], &moves.specialMoves[0],
                    &moves.TMMoves[1], &moves.preEvoMoves[1], &moves.eggMoves[1], &moves.levelMoves[1], &moves.tutorMoves[1], &moves.specialMoves[1],
                    &moves.TMMoves[2], &moves.preEvoMoves[2], &moves.eggMoves[2], &moves.levelMoves[2], &moves.tutorMoves[2], &moves.specialMoves[2]
                };
                *refs[i] = data_set;
            }
        }
    }

    QHashIterator<Pokemon::uniqueId, PokemonMoves> it(m_Moves);
    while(it.hasNext()) {
        it.next();
        PokemonMoves moves = it.value();

        for (int i = 0; i < 3; i++) {
            moves.regularMoves[i] = moves.TMMoves[i];
            moves.regularMoves[i].unite(moves.preEvoMoves[i]).unite(moves.levelMoves[i]).unite(moves.tutorMoves[i]);
            moves.genMoves[i] = moves.regularMoves[i];
            moves.genMoves[i].unite(moves.specialMoves[i]).unite(moves.eggMoves[i]);

            if (i > 0) {
                moves.genMoves[i].unite(moves.genMoves[i-1]);
            }
        }
        moves.regularMoves[0] = moves.TMMoves[0];
        moves.regularMoves[0].unite(moves.levelMoves[0]).unite(moves.tutorMoves[0]);
        moves.regularMoves[1] = moves.TMMoves[1];
        moves.regularMoves[1].unite(moves.preEvoMoves[1]).unite(moves.levelMoves[1]).unite(moves.tutorMoves[1]);

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

        for (int i = 0; i < 3; i++) {
            int gen = i+3;

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
        }
    }
}

Pokemon::uniqueId PokemonInfo::getRandomPokemon()
{
    int random = true_rand() % (NumberOfVisiblePokes());
    if(m_VisiblePokesPlainList[random] == Pokemon::NoPoke) {
        if(random == (NumberOfVisiblePokes() - 1)) {
            random--;
        }else{
            random++;
        };
    }
    return m_VisiblePokesPlainList[random];
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
    fill_container_with_file(effect, path("effect.txt"));
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
    fill_container_with_file(m_Details, trFile(path("move_effect")));
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
    return Exists(movenum, 5) ? m_Names[movenum] : m_Names[0];
}

int MoveInfo::Type(int movenum, int g)
{
    return gen(g).type[movenum];
}

int MoveInfo::Category(int movenum, int g)
{
    return gen(g).damageClass[movenum];
}

int MoveInfo::Classification(int movenum, int g)
{
    return gen(g).category[movenum];
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

int MoveInfo::EffectRate(int movenum, int g)
{
    return gen(g).effectChance[movenum];
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

    for (int i = 0; i < 3; i++) {
        fill_container_with_file(m_GenItems[i], path(QString("items_gen%1.txt").arg(i+3)));

        QList<int> tempb;
        fill_container_with_file(tempb, path(QString("berries_gen%1.txt").arg(i+3)));
        foreach(int b, tempb) {
            m_GenItems[i].insert(b+8000);
        }
    }

    m_SortedNames[2] << m_RegItemNames << m_BerryNames;
    qSort(m_SortedNames[2]);

    m_SortedUsefulNames[2] << m_BerryNames;
    for (int i = 0; i < m_RegItemNames.size(); i++) {
        if (isUseful(i))
            m_SortedUsefulNames[2].push_back(m_RegItemNames[i]);
    }
    qSort(m_SortedUsefulNames[2]);

    for (int j = 1; j >= 0; j--) {
        for (int i = 0; i < m_SortedNames[j+1].size(); i++) {
            if (Exists(Number(m_SortedNames[j+1][i]), j+3))
                m_SortedNames[j].push_back(m_SortedNames[j+1][i]);
        }

        for (int i = 0; i < m_SortedUsefulNames[j+1].size(); i++) {
            if (Exists(Number(m_SortedUsefulNames[j+1][i]), j+3))
                m_SortedUsefulNames[j].push_back(m_SortedUsefulNames[j+1][i]);
        }
    }

    for (int i = 0; i < 3; i++) {
        QStringList temp;
        fill_container_with_file(temp, path("item_effects_%1G.txt").arg(i+3));

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
            m_RegEffects[i].push_back(toPush);
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
    if (!Exists(item)) {
	return QList<ItemInfo::Effect>();
    } else {
        return isBerry(item) ? m_BerryEffects[item-8000] : m_RegEffects[gen-3][item];
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
    return m_SortedNames[5-3].size();
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
    if (itemnum == 0)
        return QPixmap();

    QString archive = path("Items.zip");
    if (isBerry(itemnum)) {
        itemnum -= 7999;
        archive = path("Berries.zip");
    }

    QString file = QString("%1.png").arg(itemnum);

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        qDebug() << "error loading icon";
        return QPixmap();
    }
    QPixmap p;
    p.loadFromData(data,"png");
    return p;
}

QString ItemInfo::Name(int itemnum)
{
    if (!Exists(itemnum)) {
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
    return m_GenItems[gen-3].contains(itemnum);
}

bool ItemInfo::isBerry(int itemnum)
{
    return itemnum >= 8000;
}

bool ItemInfo::isPlate(int itemnum)
{
    return (itemnum >= 185 && itemnum <= 202 && itemnum != 190 && itemnum != 200);
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
    return Effects(itemnum, 4).front().args.toInt();
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

float PokemonInfo::Weight(const Pokemon::uniqueId &pokeid) {
    return (m_Weights.value(pokeid).toFloat()-0.02f)/2.2f; /* the -0.02 is just a trick to compensate the poor precision of floats, for moves like grass knot */
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
    return m_SortedNames[gen-3];
}

QList<QString> ItemInfo::SortedUsefulNames(int gen)
{
    return m_SortedUsefulNames[gen-3];
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
    return ConvertToStat(Boost(nature, nature/5+1) == 0 ? 0 : nature/5+1);
}

int NatureInfo::StatHindered(int nature)
{
    return ConvertToStat(Boost(nature, (nature%5)+1) == 0 ? 0 : (nature%5)+1);
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
    for (int i = 0; i < 3; i++) {
        QStringList m_temp;
        fill_container_with_file(m_temp,path("ability_effects_%1G.txt").arg(i+3));

        foreach(QString str, m_temp) {
            QStringList content = str.split('#').front().split('-');
            if (content.size() == 1) {
                m_Effects[i].push_back(Effect(content[0].toInt()));
            } else {
                m_Effects[i].push_back(Effect(content[0].toInt(), content[1].toInt()));
            }
        }
    }
}

AbilityInfo::Effect AbilityInfo::Effects(int abnum, int gen) {
    return m_Effects[gen-3][abnum];
}

QString AbilityInfo::Desc(int ab)
{
    return get_line(trFile(path("ability_desc")), ab);
}

QString AbilityInfo::EffectDesc(int abnum)
{
    return get_line(trFile(path("ability_battledesc")), abnum);
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

int HiddenPowerInfo::Type(quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 satt_dv, quint8 sdef_dv, quint8 speed_dv)
{
    return (((hp_dv%2) + (att_dv%2)*2 + (def_dv%2)*4 + (speed_dv%2)*8 + (satt_dv%2)*16 + (sdef_dv%2)*32)*15)/63 + 1;
}

int HiddenPowerInfo::Power(quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 satt_dv, quint8 sdef_dv, quint8 speed_dv)
{
    return (((hp_dv%4>1) + (att_dv%4>1)*2 + (def_dv%4>1)*4 + (speed_dv%4>1)*8 + (satt_dv%4>1)*16 + (sdef_dv%4>1)*32)*40)/63 + 30;
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

QString StatInfo::Stat(int stat)
{
    if (stat >= 0 && stat <= Accuracy)
        return m_stats[stat];
    else
        return "";
}

QString StatInfo::Status(int stat)
{
    return m_status[stat];
}

QString StatInfo::ShortStatus(int stat)
{
    switch (stat) {
    case -2: return "Ko";
    case 0: return "";
    case 1: return "Par";
    case 2: return "Brn";
    case 3: return "Frz";
    case 4: return "Slp";
    case 5: return "Psn";
    case 6: return "Tox";
    default:
        return "";
    }
}

QString StatInfo::path(const QString &filename)
{
    return m_Directory + filename;
}
