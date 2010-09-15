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
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Type1;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Type2;
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Ability1[2];
QHash<Pokemon::uniqueId, int> PokemonInfo::m_Ability2[2];
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
QList<QString> MoveInfo::m_Names;
QVector<int> MoveInfo::m_Acc[2];
QVector<int> MoveInfo::m_Power[2];
QList<QString> MoveInfo::m_SpecialEffects;
QVector<char> MoveInfo::m_Type;
QVector<char> MoveInfo::m_PP[2];
QVector<char> MoveInfo::m_Category;
QList<QString> MoveInfo::m_Effects[2];
QVector<char> MoveInfo::m_Critical;
QVector<char> MoveInfo::m_EffectRate;
QVector<bool> MoveInfo::m_Physical;
QVector<bool> MoveInfo::m_KingRock;
QVector<char> MoveInfo::m_Speeds;
QVector<int> MoveInfo::m_Flinch;
QVector<int> MoveInfo::m_Recoil;
QVector<int> MoveInfo::m_Targets;
QList<QStringList> MoveInfo::m_MoveMessages;
QList<QPair<char, char> > MoveInfo::m_Repeat;
QList<QString> MoveInfo::m_Descriptions;
QList<QString> MoveInfo::m_Details;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;
QSet<int> MoveInfo::m_3rdGenMoves;

QString ItemInfo::m_Directory;
QList<QString> ItemInfo::m_BerryNames;
QList<QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QList<QString> ItemInfo::m_SortedNames[2];
QList<QString> ItemInfo::m_SortedUsefulNames[2];
QList<QList<ItemInfo::Effect> > ItemInfo::m_RegEffects[2];
QList<QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QList<QStringList> ItemInfo::m_RegMessages;
QList<QStringList> ItemInfo::m_BerryMessages;
QList<int> ItemInfo::m_Powers;
QList<int> ItemInfo::m_BerryPowers;
QList<int> ItemInfo::m_BerryTypes;
QList<int> ItemInfo::m_UsefulItems;
QSet<int> ItemInfo::m_3rdGenItems;

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
QList<AbilityInfo::Effect> AbilityInfo::m_Effects;
QList<QStringList> AbilityInfo::m_Messages;
QSet<int> AbilityInfo::m_3rdGenAbilities;

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

static void fill_container_with_file(QVector<bool> &container, const QString & filename)
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

int PokemonInfo::Type1(const Pokemon::uniqueId &pokeid)
{
    return m_Type1.value(pokeid);
}

int PokemonInfo::Type2(const Pokemon::uniqueId &pokeid)
{
    return m_Type2.value(pokeid);
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
    fill_uid_int(m_Type1, path("poke_type1.txt"));
    fill_uid_int(m_Type2, path("poke_type2.txt"));
    fill_uid_int(m_Genders, path("poke_gender.txt"));
    fill_uid_int(m_Ability1[0], path("poke_ability_3G.txt"));
    fill_uid_int(m_Ability2[0], path("poke_ability2_3G.txt"));
    fill_uid_int(m_Ability1[1], path("poke_ability_4G.txt"));
    fill_uid_int(m_Ability2[1], path("poke_ability2_4G.txt"));
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
void PokemonInfo::loadEvos()
{
   	QStringList temp;
    fill_container_with_file(temp, "evolutions.txt");
    for(int i = 0; i < temp.size(); i++) {
        QString current = temp[i].trimmed();
		if (current.length() == 0)
			continue;

        QString preEvoS;
        quint16 pokenum;
        bool ok = Pokemon::uniqueId::extract_short(current, pokenum, preEvoS);
        if(ok) {
            int preEvo = preEvoS.toInt();
			int i = pokenum;

            int orEvo = m_OriginalEvos[preEvo] == 0 ? preEvo : m_OriginalEvos[preEvo];
            m_OriginalEvos[i] = orEvo;
            m_PreEvos[i] = preEvo;
            m_OriginalEvos[orEvo] = orEvo;

            if (!m_Evolutions.contains(orEvo)) {
                m_Evolutions[orEvo].push_back(orEvo);
            }

            if (m_Evolutions.contains(i)) {
                m_Evolutions[orEvo].append(m_Evolutions[i]);

                foreach (int poke, m_Evolutions[i]) {
                    m_OriginalEvos[poke] = orEvo;
                }

                m_Evolutions.remove(i);
            } else {
                m_Evolutions[orEvo].push_back(i);
            }
        }
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
    return gen == 4 ? m_trueNumberOfPokes : 387;
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
    QString archive = path("poke_img.zip");

    QString file;

    if (gen ==3)
        file = QString("%1/%2%3.png").arg(pokeid.toString(), back?"3Gback":"RFLG", shiney?"s":"");
    else if (gen == 4)
        file = QString("%1/DP%2%3%4.png").arg(pokeid.toString(), back?"b":"", (gender==Pokemon::Female)?"f":"m", shiney?"s":"");

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

    QString file = QString("sub%1%2.png").arg(back?"b":"").arg(gen==4?"":"3G");

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
    QSet<int> moves;
    moves.unite(RegularMoves(pokeid,3)).unite(SpecialMoves(pokeid,3)).unite(EggMoves(pokeid,3));

    if (gen >= 4)
        moves.unite(SpecialMoves(pokeid,4)).unite(RegularMoves(pokeid, 4)).unite(EggMoves(pokeid,4));

    return moves;
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

    ret.ab1 = m_Ability1[gen-3].value(pokeid);
    ret.ab2 = m_Ability2[gen-3].value(pokeid);

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
    return m_OriginalEvos[pokenum];
}

QList<int> PokemonInfo::Evos(int pokenum)
{
    return m_Evolutions.value(OriginalEvo(pokenum).pokenum);
}

bool PokemonInfo::IsInEvoChain(const Pokemon::uniqueId &pokeid)
{
    return OriginalEvo(pokeid).pokenum != pokeid.pokenum;
}

void PokemonInfo::loadMoves()
{
    static const int filesize = 11;

    QString fileNames[filesize] = {
        path("3G_tm_and_hm_moves.txt"), path("4G_tm_and_hm_moves.txt"), path("3G_egg_moves.txt"), path("3G_level_moves.txt"),
        path("3G_tutor_moves.txt"), path("3G_special_moves.txt"), path("4G_pre_evo_moves.txt"),
        path("4G_egg_moves.txt"), path("4G_level_moves.txt"), path("4G_tutor_moves.txt"),
        path("4G_special_moves.txt")
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
                    &moves.TMMoves[0], &moves.TMMoves[1], &moves.eggMoves[0], &moves.levelMoves[0], &moves.tutorMoves[0], &moves.specialMoves[0],
                    &moves.preEvoMoves[1], &moves.eggMoves[1], &moves.levelMoves[1], &moves.tutorMoves[1], &moves.specialMoves[1]
                };
                *refs[i] = data_set;
            }
        }
    }

    QHashIterator<Pokemon::uniqueId, PokemonMoves> it(m_Moves);
    while(it.hasNext()) {
        it.next();
        PokemonMoves moves = it.value();

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

void PokemonInfo::makeDataConsistent()
{
    // Count base forms. We no longer need to save it in a file.
    m_trueNumberOfPokes = 0;
    // Also adds data to pokemon that do not have data set explicitely (some formes).
    QMap<Pokemon::uniqueId, QString>::const_iterator it = m_Names.constBegin();
    while (it != m_Names.constEnd()) {
        Pokemon::uniqueId id = it.key();
        if(id.subnum == 0) {
            // Count base forms.
            m_trueNumberOfPokes++;
            // Original evolutions.
            if(!m_OriginalEvos.contains(id.pokenum)) {
                m_OriginalEvos[id.pokenum] = id.pokenum;
            }
            // m_Evolutions initial filler data.
            m_Evolutions[id.pokenum] = QList<int>();
        }
        // Weight
        if(!m_Weights.contains(id)) {
            m_Weights[id] = m_Weights.value(OriginalForme(id), "0.0");
        }
        // Base stats.
        if(!m_BaseStats.contains(id)) {
            m_BaseStats[id] = m_BaseStats.value(OriginalForme(id), PokeBaseStats());
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
        if(!m_Type1.contains(id)) {
            m_Type1[id] = m_Type1.value(OriginalForme(id), Pokemon::Normal);
        }
        if(!m_Type2.contains(id)) {
            m_Type2[id] = m_Type2.value(OriginalForme(id), Pokemon::Curse);
        }
        if(!m_Genders.contains(id)) {
            m_Genders[id] = m_Genders.value(OriginalForme(id), Pokemon::NeutralAvail);
        }
        if(!m_Ability1[0].contains(id)) {
            m_Ability1[0][id] = m_Ability1[0].value(OriginalForme(id), Ability::NoAbility);
        }
        if(!m_Ability2[0].contains(id)) {
            m_Ability2[0][id] = m_Ability2[0].value(OriginalForme(id), Ability::NoAbility);
        }
        if(!m_Ability1[1].contains(id)) {
            m_Ability1[1][id] = m_Ability1[1].value(OriginalForme(id), Ability::NoAbility);
        }
        if(!m_Ability2[1].contains(id)) {
            m_Ability2[1][id] = m_Ability2[1].value(OriginalForme(id), Ability::NoAbility);
        }
        // Next.
        ++it;
    }
    // Calculate m_Evolutions.
    QHash<int, int>::const_iterator eit = m_OriginalEvos.constBegin();
    while (eit != m_OriginalEvos.constEnd()) {
        m_Evolutions[eit.value()].append(eit.key());
        // Next.
        ++eit;
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

void MoveInfo::loadCritics()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_critical.txt"));

    foreach(QString str, temp) { m_Critical.push_back(str.toInt());}
}

void MoveInfo::loadTargets()
{
    fill_container_with_file(m_Targets, path("move_target.txt"));
}

void MoveInfo::loadEffectRates()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_effect_rate.txt"));

    foreach(QString str, temp) {m_EffectRate.push_back(str.toInt());}
}

void MoveInfo::loadPhysics()
{
    fill_container_with_file(m_Physical, path("move_physical_contact.txt"));
}

void MoveInfo::loadKingRocks()
{
    fill_container_with_file(m_KingRock, path("move_kingrock.txt"));
}

void MoveInfo::loadRepeats()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_repeat.txt"));

    foreach(QString str, temp) {
	m_Repeat.push_back(QPair<char, char>(str[0].toAscii()-'0', str[2].toAscii()-'0'));
    }
}

void MoveInfo::loadMoveMessages()
{
    QStringList temp;
    fill_container_with_file(temp, trFile(path("move_message")));

    foreach(QString str, temp) {
	m_MoveMessages.push_back(str.split('|'));
    }
}

void MoveInfo::loadSpeeds()
{
    fill_container_with_file(m_Speeds, path("move_speed_priority.txt"));
}

void MoveInfo::loadRecoil()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_recoil.txt"));

    foreach(QString str, temp) {m_Recoil.push_back(str.toInt());}
}

int MoveInfo::Recoil(int num, int gen)
{
    if (gen < 4 && num == Move::Struggle) {
        return 2;
    }
    return m_Recoil[num];
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
    loadTypes();
    loadPPs();
    loadAccs();
    loadPowers();
    loadCategorys();
    loadEffects();
    loadRepeats();
    loadTargets();
    loadCritics();
    loadPhysics();
    loadKingRocks();
    loadEffectRates();
    loadSpeeds();
    loadFlinchs();
    loadMoveMessages();
    loadRecoil();
    loadSpecialEffects();
    loadDescriptions();
    loadDetails();
}

int MoveInfo::NumberOfMoves()
{
    return m_Names.size();
}

QString MoveInfo::MoveMessage(int moveeffect, int part)
{
    if (moveeffect < 0 || moveeffect >= m_MoveMessages.size() || part < 0 || part >= m_MoveMessages[moveeffect].size()) {
	return "";
    }
    return m_MoveMessages[moveeffect][part];
}

int MoveInfo::Number(const QString &movename)
{
    return m_LowerCaseMoves.value(movename.toLower());
}

QString MoveInfo::SpecialEffect(int movenum)
{
    return m_SpecialEffects[movenum];
}

int MoveInfo::Target(int movenum, int gen)
{
    if (gen == 3 && movenum == Move::Surf)
        return Move::Opponents;

    return m_Targets[movenum];
}

void MoveInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("moves")));
    for (int i = 0; i < m_Names.size(); i++) {
        m_LowerCaseMoves.insert(m_Names[i].toLower(),i);
    }

    fill_container_with_file(m_3rdGenMoves, path("gen3.txt"));
}

void MoveInfo::loadDescriptions()
{
    fill_container_with_file(m_Descriptions, trFile(path("move_description")));
}

void MoveInfo::loadDetails()
{
    fill_container_with_file(m_Details, trFile(path("move_effect")));
}

QString MoveInfo::Description(int movenum)
{
    return m_Descriptions[movenum];
}

QString MoveInfo::DetailedDescription(int movenum)
{
    return m_Details[movenum];
}

void MoveInfo::loadPPs()
{
    fill_container_with_file(m_PP[0], path("move_pp_3G.txt"));
    fill_container_with_file(m_PP[1], path("move_pp.txt"));
}

void MoveInfo::loadTypes()
{
    fill_container_with_file(m_Type, path("move_type.txt"));
}

void MoveInfo::loadCategorys()
{
    fill_container_with_file(m_Category, path("move_category.txt"));
}

void MoveInfo::loadPowers()
{
    QList<QString> temp;
    fill_container_with_file(temp, path("move_power_3G.txt"));

    foreach (QString s, temp) {
        m_Power[0].push_back(s.toInt());
    }

    temp.clear();

    fill_container_with_file(temp, path("move_power.txt"));

    foreach (QString s, temp) {
        m_Power[1].push_back(s.toInt());
    }
}

void MoveInfo::loadAccs()
{
    QList<QString> temp;
    fill_container_with_file(temp, path("move_accuracy_3G.txt"));

    foreach (QString s, temp) {
        m_Acc[0].push_back(s.toInt());
    }

    temp.clear();

    fill_container_with_file(temp, path("move_accuracy.txt"));

    foreach (QString s, temp) {
        m_Acc[1].push_back(s.toInt());
    }
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}

QString MoveInfo::Name(int movenum)
{
    return Exists(movenum, 4) ? m_Names[movenum] : m_Names[0];
}

bool MoveInfo::Exists(int movenum, int gen)
{
    if (movenum < 0 || movenum >= NumberOfMoves())
        return false;

    if (gen == 4)
        return true;

    return m_3rdGenMoves.contains(movenum);
}

int MoveInfo::Power(int movenum, int gen)
{
    return m_Power[gen-3][movenum];
}

bool MoveInfo::isOHKO(int movenum)
{
    //Fissure, Guillotine, Horndrill, Sheer Cold
    return (movenum == 133 || movenum == 166 || movenum == 186 || movenum == 353);
}

int MoveInfo::Type(int movenum)
{
    return m_Type[movenum];
}

int MoveInfo::Category(int movenum, int gen)
{
    if (gen >= 4)
        return m_Category[movenum];
    else {
        int power = MoveInfo::Power(movenum, gen);

        if (power == 0) {
            return Move::Other;
        }

        return TypeInfo::Category(Type(movenum));
    }
}

int MoveInfo::PP(int movenum, int gen)
{
    return m_PP[gen-3][movenum];
}

int MoveInfo::Acc(int movenum, int gen)
{
    int ret = m_Acc[gen-3][movenum];
    return ret == 0 ? 65535 : ret;
}

QString MoveInfo::AccS(int movenum, int gen)
{
    return m_Acc[gen-3][movenum] == 0 ? "--" : QString::number(m_Acc[gen-3][movenum]);
}

QString MoveInfo::PowerS(int movenum, int gen)
{
    int pow = Power(movenum, gen);

    if (pow == 0) {
        return "--";
    } else if (pow == 1) {
	return "???";
    } else {
        return QString::number(pow);
    }
}

QString MoveInfo::Effect(int movenum, int gen)
{
    return m_Effects[gen-3][movenum];
}

int MoveInfo::CriticalRaise(int num)
{
    return m_Critical[num];
}

int MoveInfo::EffectRate(int num)
{
    return m_EffectRate[num];
}

bool MoveInfo::PhysicalContact(int num, int gen)
{
    if (gen <= 3 && num == Move::Overheat)
        return true;

    return m_Physical[num];
}

bool MoveInfo::KingRock(int num)
{
    return m_KingRock[num];
}

int MoveInfo::RepeatMin(int num)
{
    return m_Repeat[num].first;
}

int MoveInfo::RepeatMax(int num)
{
    return m_Repeat[num].second;
}

int MoveInfo::SpeedPriority(int num)
{
    return m_Speeds[num];
}

int MoveInfo::FlinchRate(int num, int gen)
{
    if (gen <= 3 && num == Move::Waterfall)
        return 0;
    return m_Flinch[num];
}

void MoveInfo::loadFlinchs()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_flinch.txt"));

    foreach(QString s, temp) {
	m_Flinch.push_back(s.toInt());
    }
}

void MoveInfo::loadEffects()
{
    QStringList temp;
    fill_container_with_file(temp, path("moveeffects.txt"));

    /* Removing comments, aka anything starting from '#' */
    foreach (QString eff, temp) {
        m_Effects[1].push_back(eff.split('#').front());
    }

    temp.clear();
    fill_container_with_file(temp, path("moveeffects_3G.txt"));

    /* Removing comments, aka anything starting from '#' */
    foreach (QString eff, temp) {
        m_Effects[0].push_back(eff.split('#').front());
    }
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
    fill_container_with_file(m_3rdGenItems, path("items_gen3.txt"));

    QList<int> tempb;
    fill_container_with_file(tempb, path("berries_gen3.txt"));
    foreach(int b, tempb) {
        m_3rdGenItems.insert(b+8000);
    }

    m_SortedNames[1] << m_RegItemNames << m_BerryNames;
    qSort(m_SortedNames[1]);

    m_SortedUsefulNames[1] << m_BerryNames;

    for (int i = 0; i < m_RegItemNames.size(); i++) {
        if (isUseful(i))
            m_SortedUsefulNames[1].push_back(m_RegItemNames[i]);
    }
    qSort(m_SortedUsefulNames[1]);

    for (int i = 0; i < m_SortedNames[1].size(); i++) {
        if (Exists(Number(m_SortedNames[1][i]), 3))
            m_SortedNames[0].push_back(m_SortedNames[1][i]);
    }

    for (int i = 0; i < m_SortedUsefulNames[1].size(); i++) {
        if (Exists(Number(m_SortedUsefulNames[1][i]), 3))
            m_SortedUsefulNames[0].push_back(m_SortedUsefulNames[1][i]);
    }

    QStringList temp;
    fill_container_with_file(temp, path("item_effects.txt"));

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
        m_RegEffects[1].push_back(toPush);
    }

    temp.clear();

    fill_container_with_file(temp, path("item_effects_3G.txt"));

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
        m_RegEffects[0].push_back(toPush);
    }

    temp.clear();

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
    return m_SortedNames[4-3].size();
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
    if ((itemnum < 8000 && itemnum >= m_RegItemNames.size()) && !(itemnum >= 8000 + m_BerryNames.size()))
        return false;
    if (gen == 4)
        return true;

    return m_3rdGenItems.contains(itemnum);
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
    return (statUp-1) * 5 + statDown-1;
}

int NatureInfo::Boost(int nature, int stat)
{
    return -(nature%5 == stat-1) + (nature/5 == stat-1);
}

int NatureInfo::StatBoosted(int nature)
{
    return Boost(nature, nature/5+1) == 0 ? 0 : nature/5+1;
}

int NatureInfo::StatHindered(int nature)
{
    return Boost(nature, (nature%5)+1) == 0 ? 0 : (nature%5)+1;
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

    fill_container_with_file(m_3rdGenAbilities, path("gen3.txt"));
}

bool AbilityInfo::Exists(int ability, int gen)
{
    if (ability < 0 || ability >= NumberOfAbilities())
        return false;
    if (gen == 4)
        return true;
    return m_3rdGenAbilities.contains(ability);
}

void AbilityInfo::loadEffects()
{
    QStringList m_temp;
    fill_container_with_file(m_temp,path("ability_effects.txt"));

    foreach(QString str, m_temp) {
        QStringList content = str.split('#').front().split('-');
        if (content.size() == 1) {
            m_Effects.push_back(Effect(content[0].toInt()));
        } else {
            m_Effects.push_back(Effect(content[0].toInt(), content[1].toInt()));
        }
    }
}

AbilityInfo::Effect AbilityInfo::Effects(int abnum) {
    return m_Effects[abnum];
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

int HiddenPowerInfo::Type(quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 speed_dv, quint8 satt_dv, quint8 sdef_dv)
{
    return (((hp_dv%2) + (att_dv%2)*2 + (def_dv%2)*4 + (speed_dv%2)*8 + (satt_dv%2)*16 + (sdef_dv%2)*32)*15)/63 + 1;
}

int HiddenPowerInfo::Power(quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 speed_dv, quint8 satt_dv, quint8 sdef_dv)
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
