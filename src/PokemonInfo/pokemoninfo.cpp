#include "pokemoninfo.h"
#include "pokemonstructs.h"

#ifdef CLIENT_SIDE
# include "../../SpecialIncludes/zip.h"
#endif

/*initialising static variables */
QString PokemonInfo::m_Directory;
QTSList<QString> PokemonInfo::m_Names;
QTSList<float> PokemonInfo::m_Weights;
QTSList<int> PokemonInfo::m_Genders;
QTSList<int> PokemonInfo::m_Type1;
QTSList<int> PokemonInfo::m_Type2;
QTSList<int> PokemonInfo::m_Ability1;
QTSList<int> PokemonInfo::m_Ability2;
QTSList<PokeBaseStats> PokemonInfo::m_BaseStats;
QTSList<int> PokemonInfo::m_LevelBalance;

QString MoveInfo::m_Directory;
QTSList<QString> MoveInfo::m_Names;
QTSList<QString> MoveInfo::m_AccS;
QTSList<QString> MoveInfo::m_PowerS;
QTSList<QString> MoveInfo::m_SpecialEffects;
QTSList<char> MoveInfo::m_Type;
QTSList<char> MoveInfo::m_PP;
QTSList<char> MoveInfo::m_Category;
QTSList<QString> MoveInfo::m_Effects;
QTSList<char> MoveInfo::m_Critical;
QTSList<char> MoveInfo::m_EffectRate;
QTSList<bool> MoveInfo::m_Physical;
QTSList<bool> MoveInfo::m_KingRock;
QTSList<char> MoveInfo::m_Speeds;
QTSList<int> MoveInfo::m_Flinch;
QTSList<int> MoveInfo::m_Recoil;
QTSList<int> MoveInfo::m_Targets;
QTSList<QStringList> MoveInfo::m_MoveMessages;
QTSList<QPair<char, char> > MoveInfo::m_Repeat;
QTSList<QString> MoveInfo::m_Descriptions;
QTSList<QString> MoveInfo::m_Details;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;

QString ItemInfo::m_Directory;
QTSList<QString> ItemInfo::m_BerryNames;
QTSList<QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QTSList<QString> ItemInfo::m_SortedNames;
QTSList<QList<ItemInfo::Effect> > ItemInfo::m_RegEffects;
QTSList<QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QTSList<QStringList> ItemInfo::m_RegMessages;
QTSList<QStringList> ItemInfo::m_BerryMessages;
QTSList<int> ItemInfo::m_Powers;
QTSList<int> ItemInfo::m_BerryPowers;
QTSList<int> ItemInfo::m_BerryTypes;

QTSList<QString> TypeInfo::m_Names;
QTSList<QColor> TypeInfo::m_Colors;
QString TypeInfo::m_Directory;
QTSList<int> TypeInfo::m_TypeVsType;

QTSList<QString> NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QTSList<QString> CategoryInfo::m_Names;
QTSList<QColor> CategoryInfo::m_Colors;
QString CategoryInfo::m_Directory;

QTSList<QString> AbilityInfo::m_Names;
QString AbilityInfo::m_Directory;
QTSList<AbilityInfo::Effect> AbilityInfo::m_Effects;
QTSList<QStringList> AbilityInfo::m_Messages;

QTSList<QString> GenderInfo::m_Names;
QTSList<QPixmap> GenderInfo::m_Pictures;
QTSList<QPixmap> GenderInfo::m_BattlePictures;
QString GenderInfo::m_Directory;

QString HiddenPowerInfo::m_Directory;

QString StatInfo::m_Directory;
QTSList<QString> StatInfo::m_stats;
QTSList<QString> StatInfo::m_status;
QHash<int, QPixmap> StatInfo::m_statusIcons;
QHash<int, QPixmap> StatInfo::m_battleIcons;

#ifdef CLIENT_SIDE
QByteArray readZipFile(const char *archiveName, const char *fileName)
{
    int error = 0;
    zip *archive;
    zip_file *file;
    char buffer[1024];
    int readsize = 0;
    QByteArray ret;

    archive = zip_open(archiveName, 0, &error);

    if (!archive)
    {
        qDebug() << "Error when opening archive" << archiveName;
        return ret;
    }

    file = zip_fopen(archive, fileName, 0);

    if (!file)
    {
        qDebug() << "Error when opening file "<< fileName <<" in archive: " << archiveName <<" : " << zip_strerror(archive);
        zip_close(archive);
        return ret;
    }

    do
    {
        ret.append(buffer, readsize);

        readsize = zip_fread(file, buffer, 1024);
    } while (readsize > 0) ;

    if (readsize < 0)
    {
        qDebug() << "Error when reading file "<< fileName <<" in archive: " << archiveName <<" : " << zip_file_strerror(file);
    }

    zip_fclose(file);
    zip_close(archive);

    return ret;
}
#endif

void fill_container_with_file(QStringList &container, const QString & filename)
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

void fill_container_with_file(QTSList<QString> &container, const QString & filename)
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

void fill_container_with_file(QTSList<QColor> &container, const QString &filename)
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

void fill_container_with_file(QTSList<bool> &container, const QString & filename)
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

void fill_container_with_file(QTSList<char> &container, const QString & filename)
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

template <class T>
void fill_container_with_file(T &container, const QString & filename)
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

QString get_line(const QString & filename, int linenum)
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

int PokemonInfo::Type1(int pokenum)
{
    return m_Type1[pokenum];
}

int PokemonInfo::Type2(int pokenum)
{
    return m_Type2[pokenum];
}

int PokemonInfo::calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev)
{
    return ((2*basestat + dv+ ev/4)*level)/100 + 5;
}

int PokemonInfo::Stat(int poke, int stat, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp) {
        /* Shedinja */
        if (poke == 292)
            return 1;
        else
            return calc_stat(basestat, level, dv, ev) + level + 5;
    }
    else
	return calc_stat(basestat, level, dv, ev);
}

int PokemonInfo::FullStat(int poke, int nature, int stat, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp) {
        return Stat(poke, stat, basestat, level, dv, ev);
    }
    else {
        return Stat(poke, stat, basestat, level, dv, ev) * (10+NatureInfo::Boost(nature, stat)) / 10;
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
    fill_container_with_file(m_Type1, path("poke_type1.txt"));
    fill_container_with_file(m_Type2, path("poke_type2.txt"));
    fill_container_with_file(m_Genders, path("poke_gender.txt"));
    fill_container_with_file(m_Ability1, path("poke_ability.txt"));
    fill_container_with_file(m_Ability2, path("poke_ability2.txt"));
    fill_container_with_file(m_LevelBalance, path("level_balance.txt"));
    loadBaseStats();
}

int PokemonInfo::NumberOfPokemons()
{
    return m_Names.size();
}

QString PokemonInfo::Name(int pokenum)
{
    return Exist(pokenum) ? m_Names[pokenum] : m_Names[0];
}

bool PokemonInfo::Exist(int n)
{
    return n < NumberOfPokemons() && n>=0;
}

int PokemonInfo::Number(const QString &pokename)
{
    return (qFind(m_Names.begin(), m_Names.end(), pokename)-m_Names.begin()) % (NumberOfPokemons());
}

int PokemonInfo::LevelBalance(int pokenum)
{
    return m_LevelBalance[pokenum];
}

int PokemonInfo::Gender(int pokenum)
{
    return m_Genders[pokenum];
}

#ifdef CLIENT_SIDE
QPixmap PokemonInfo::Picture(int pokenum, int gender, bool shiney, bool back)
{
    QString archive = path("poke_img.zip");

    QString file = QString("%2/DP%3%4%5.png").arg(pokenum).arg(back?"b":"",(gender==Pokemon::Female)?"f":"m", shiney?"s":"");

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
	return QPixmap();

    QPixmap ret;
    ret.loadFromData(data, "png");

    return ret;
}

QPixmap PokemonInfo::Sub(bool back)
{
    QString archive = path("poke_img.zip");

    QString file = QString("sub%1.png").arg(back?"b":"");

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());

    if (data.length()==0)
        return QPixmap();

    QPixmap ret;
    ret.loadFromData(data, "png");

    return ret;
}

QPixmap PokemonInfo::Icon(int index)
{
    QString archive = path("icons.zip");
    QString file = QString("%1.PNG").arg(index);

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
#endif

QSet<int> PokemonInfo::Moves(int pokenum)
{
    QSet<int> tmMoves = TMMoves(pokenum);
    for (int i = 3; i <= 4; i++) {
         tmMoves.unite(LevelMoves(pokenum, i)).unite(EggMoves(pokenum, i)).unite(TutorMoves(pokenum, i)).unite(SpecialMoves(pokenum,i)).unite(PreEvoMoves(pokenum,i));
    }
    return tmMoves;
}

QSet<int> PokemonInfo::EggMoves(int pokenum, int gen)
{
    return getMoves(QString::number(gen)+"G_egg_moves.txt", pokenum);
}

QSet<int> PokemonInfo::LevelMoves(int pokenum, int gen)
{
    return getMoves(QString::number(gen)+"G_level_moves.txt", pokenum);
}

QSet<int> PokemonInfo::TutorMoves(int pokenum, int gen)
{
    return getMoves(QString::number(gen)+"G_tutor_moves.txt", pokenum);
}

QSet<int> PokemonInfo::TMMoves(int pokenum)
{
    return getMoves("tm_and_hm_moves.txt", pokenum);
}

QSet<int> PokemonInfo::SpecialMoves(int pokenum, int gen)
{
    return getMoves(QString::number(gen)+"G_special_moves.txt", pokenum);
}

QSet<int> PokemonInfo::PreEvoMoves(int pokenum, int gen)
{
    return getMoves(QString::number(gen)+"G_pre_evo_moves.txt", pokenum);
}

QList<int> PokemonInfo::Abilities(int pokenum)
{
    QList<int> ret;
    ret << m_Ability1[pokenum] << m_Ability2[pokenum];

    return ret;
}

void PokemonInfo::loadBaseStats()
{
    QStringList temp;
    fill_container_with_file(temp, path("poke_stats.txt"));

    for (int i = 0; i < temp.size(); i++) {
        QTextStream statsstream(&temp[i], QIODevice::ReadOnly);

        int hp, att, def, spd, satt, sdef;

        statsstream >> hp >> att >> def >> spd >> satt >> sdef;

        m_BaseStats.push_back(PokeBaseStats(hp, att, def, spd, satt, sdef));
    }
}

PokeBaseStats PokemonInfo::BaseStats(int pokenum)
{
    return m_BaseStats[pokenum];
}

void PokemonInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("pokemons")));
    fill_container_with_file(m_Weights, path("poke_weight.txt"));
}

QString PokemonInfo::path(const QString &filename)
{
#ifdef MULTI_THREADED_ACCESS
    static QMutex m;
    QMutexLocker a(&m);
#endif
    return m_Directory + filename;
}

QSet<int> PokemonInfo::getMoves(const QString &filename, int pokenum)
{
    QSet<int> return_value;

    /* getting the line we want */
    QString interesting_line = get_line(path(filename), pokenum);

    /* extracting the moves */
    QTextStream ss(&interesting_line, QIODevice::ReadOnly);
    while (!ss.atEnd())
    {
        int val;
        ss >> val;

        if (val != 0)
            return_value.insert(val);
    }

    return return_value;
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

int MoveInfo::Recoil(int num)
{
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

int MoveInfo::Target(int movenum)
{
    return m_Targets[movenum];
}

void MoveInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("moves")));
    for (int i = 0; i < m_Names.size(); i++) {
        m_LowerCaseMoves.insert(m_Names[i].toLower(),i);
    }
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
    fill_container_with_file(m_PP, path("move_pp.txt"));
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
    fill_container_with_file(m_PowerS, path("move_power.txt"));
}

void MoveInfo::loadAccs()
{
    fill_container_with_file(m_AccS, path("move_accuracy.txt"));
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}

QString MoveInfo::Name(int movenum)
{
    return Exist(movenum) ? m_Names[movenum] : m_Names[0];
}

bool MoveInfo::Exist(int movenum)
{
    return movenum >= 0 && movenum < NumberOfMoves();
}

int MoveInfo::Power(int movenum)
{
    return m_PowerS[movenum].toInt();
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

int MoveInfo::Category(int movenum)
{
    return m_Category[movenum];
}

int MoveInfo::PP(int movenum)
{
    return m_PP[movenum];
}

int MoveInfo::Acc(int movenum)
{
    int ret = AccS(movenum).toInt();
    return ret == 0 ? 65535 : ret;
}

QString MoveInfo::AccS(int movenum)
{
    return m_AccS[movenum];
}

QString MoveInfo::PowerS(int movenum)
{
    if (m_PowerS[movenum] == "1") {
	return "???";
    } else {
	return m_PowerS[movenum];
    }
}

QString MoveInfo::Effect(int movenum)
{
    return m_Effects[movenum];
}

int MoveInfo::CriticalRaise(int num)
{
    return m_Critical[num];
}

int MoveInfo::EffectRate(int num)
{
    return m_EffectRate[num];
}

bool MoveInfo::PhysicalContact(int num)
{
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

int MoveInfo::FlinchRate(int num)
{
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
	m_Effects.push_back(eff.split('#').front());
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

    m_SortedNames << m_RegItemNames << m_BerryNames;
    qSort(m_SortedNames);

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
	m_RegEffects.push_back(toPush);
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
    fill_container_with_file(temp, path("item_messages_en.txt"));
    foreach (QString eff, temp) {
	m_RegMessages.push_back(eff.split('|'));
    }

    temp.clear();
    fill_container_with_file(temp, path("berry_messages_en.txt"));
    foreach (QString eff, temp) {
	m_BerryMessages.push_back(eff.split('|'));
    }

    fill_container_with_file(m_Powers, path("items_pow.txt"));
}

QList<ItemInfo::Effect> ItemInfo::Effects(int item)
{
    if (!Exist(item)) {
	return QList<ItemInfo::Effect>();
    } else {
	return isBerry(item) ? m_BerryEffects[item-8000] : m_RegEffects[item];
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
    return m_SortedNames.size();
}

int ItemInfo::Power(int itemnum) {
    if (isBerry(itemnum)) {
	return 10;
    } else if (Exist(itemnum)) {
	return m_Powers[itemnum];
    } else return 0;
}

int ItemInfo::BerryPower(int itemnum)
{
    if (!isBerry(itemnum) || !Exist(itemnum)) {
        return 0;
    }

    return m_BerryPowers[itemnum-8000];
}

int ItemInfo::BerryType(int itemnum)
{
    if (!isBerry(itemnum) || !Exist(itemnum)) {
        return 0;
    }

    return m_BerryTypes[itemnum-8000];
}

QString ItemInfo::Name(int itemnum)
{
    if (!Exist(itemnum)) {
	return 0;
    }
    if (itemnum < 8000) {
	return m_RegItemNames[itemnum];
    } else {
	return m_BerryNames[itemnum-8000];
    }
}

bool ItemInfo::Exist(int itemnum)
{
    return !(itemnum < 8000 && itemnum >= m_RegItemNames.size()) && !(itemnum >= 8000 + m_BerryNames.size());
}

bool ItemInfo::isBerry(int itemnum)
{
    return itemnum >= 8000;
}

bool ItemInfo::isPlate(int itemnum)
{
    return (itemnum >= 185 && itemnum <= 202 && itemnum != 190 && itemnum != 200);
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

float PokemonInfo::Weight(int pokenum) {
    return (m_Weights[pokenum]-0.02f)/2.2f; /* the -0.02 is just a trick to compensate the poor precision of floats, for moves like grass knot */
}

int ItemInfo::SortedNumber(const QString &itemname)
{
    return (qLowerBound(m_SortedNames, itemname) - m_SortedNames.begin()) % (NumberOfItems());
}

QTSList<QString> ItemInfo::SortedNames()
{
    return m_SortedNames;
}

void TypeInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("types")));
}

QString TypeInfo::path(const QString& file)
{
    return m_Directory+file;
}

void TypeInfo::loadColors()
{
    fill_container_with_file(m_Colors, path("type_colors.txt"));
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
    loadColors();
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

QColor TypeInfo::Color(int typenum)
{
    return m_Colors[typenum];
}

int TypeInfo::NumberOfTypes()
{
    return m_Names.size();
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


int NatureInfo::Boost(int nature, int stat)
{
    return -(nature%5 == stat-1) + (nature/5 == stat-1);
}


void CategoryInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("categories")));
}

QString CategoryInfo::path(const QString& file)
{
    return m_Directory+file;
}

void CategoryInfo::loadColors()
{
    fill_container_with_file(m_Colors, path("category_colors.txt"));
}

void CategoryInfo::init(const QString &dir)
{
    if (NumberOfCategories() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadColors();
}

QString CategoryInfo::Name(int catnum)
{
    return m_Names[catnum];
}

QColor CategoryInfo::Color(int catnum)
{
    return m_Colors[catnum];
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

void GenderInfo::loadPixmaps()
{
    for (int i = 0; i < NumberOfGenders(); i++) {
        m_Pictures << QPixmap(path(QString("gender%1.png").arg(i)));
        m_BattlePictures << QPixmap(path(QString("battle_gender%1.png").arg(i)));
    }
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
    loadPixmaps();
}

QString GenderInfo::Name(int abnum)
{
    return m_Names[abnum];
}

QPixmap GenderInfo::Picture(int gender, bool battle)
{
    return battle ? m_BattlePictures[gender] : m_Pictures[gender];
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
#ifdef MULTI_THREADED_ACCESS
    static QMutex m;
    QMutexLocker a(&m);
#endif
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

    m_statusIcons[-2] = QPixmap(path("status-2.png"));
    m_battleIcons[-2] = QPixmap(path("battle_status-2.png"));

    for (int i = 0; i < 7; i++) {
        m_statusIcons[i] = QPixmap(path("status%1.png").arg(i));
        m_battleIcons[i] = QPixmap(path("battle_status%1.png").arg(i));
    }
}

QPixmap StatInfo::Icon(int status) {
    return m_statusIcons[status];
}

QPixmap StatInfo::BattleIcon(int status) {
    return m_battleIcons[status];
}

QString StatInfo::Stat(int stat)
{
    return m_stats[stat];
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

QColor StatInfo::StatusColor(int status)
{
    switch (status) {
    case -2: return "#171b1a";
    case 0: return TypeInfo::Color(Pokemon::Normal);
    case 1: return TypeInfo::Color(Pokemon::Electric);
    case 2: return TypeInfo::Color(Pokemon::Fire);
    case 3: return TypeInfo::Color(Pokemon::Ice);
    case 4: return TypeInfo::Color(Pokemon::Psychic);
    case 5: case 6: return TypeInfo::Color(Pokemon::Poison);
    default: return QColor();
    }
}

QString StatInfo::path(const QString &filename)
{
    return m_Directory + filename;
}
