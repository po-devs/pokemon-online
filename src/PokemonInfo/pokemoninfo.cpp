#include "pokemoninfo.h"
#include "pokemonstructs.h"

#include "../../SpecialIncludes/zip.h"

PokemonInfoConfig::Config PokemonInfoConfig::_config = PokemonInfoConfig::Gui;

void PokemonInfoConfig::setConfig(Config cf) {
    _config = cf;
}

PokemonInfoConfig::Config PokemonInfoConfig::config() {
    return _config;
}

/*initialising static variables */
QString PokemonInfo::m_Directory;
QList<QString> PokemonInfo::m_Names;
QList<QString> PokemonInfo::m_Weights;
QList<int> PokemonInfo::m_Genders;
QList<int> PokemonInfo::m_Type1;
QList<int> PokemonInfo::m_Type2;
QList<int> PokemonInfo::m_Ability1;
QList<int> PokemonInfo::m_Ability2;
QList<PokeBaseStats> PokemonInfo::m_BaseStats;
QList<int> PokemonInfo::m_LevelBalance;
QList<PokemonMoves> PokemonInfo::m_Moves;
QHash<int, QList<int> > PokemonInfo::m_AlternateFormes;
QHash<int, QPair<int,int> > PokemonInfo::m_AestheticFormes;
QHash<int, bool > PokemonInfo::m_AestheticFormesHidden;
QHash<int, QString> PokemonInfo::m_AestheticFormesDescs;
int PokemonInfo::m_trueNumberOfPokes;
QHash<int,QList<int> > PokemonInfo::m_Evolutions;
QList<int> PokemonInfo::m_OriginalEvos;

QString MoveInfo::m_Directory;
QList<QString> MoveInfo::m_Names;
QList<QString> MoveInfo::m_AccS;
QList<QString> MoveInfo::m_PowerS;
QList<QString> MoveInfo::m_SpecialEffects;
QList<char> MoveInfo::m_Type;
QList<char> MoveInfo::m_PP;
QList<char> MoveInfo::m_Category;
QList<QString> MoveInfo::m_Effects;
QList<char> MoveInfo::m_Critical;
QList<char> MoveInfo::m_EffectRate;
QList<bool> MoveInfo::m_Physical;
QList<bool> MoveInfo::m_KingRock;
QList<char> MoveInfo::m_Speeds;
QList<int> MoveInfo::m_Flinch;
QList<int> MoveInfo::m_Recoil;
QList<int> MoveInfo::m_Targets;
QList<QStringList> MoveInfo::m_MoveMessages;
QList<QPair<char, char> > MoveInfo::m_Repeat;
QList<QString> MoveInfo::m_Descriptions;
QList<QString> MoveInfo::m_Details;
QHash<QString, int> MoveInfo::m_LowerCaseMoves;

QString ItemInfo::m_Directory;
QList<QString> ItemInfo::m_BerryNames;
QList<QString> ItemInfo::m_RegItemNames;
QHash<QString, int> ItemInfo::m_BerryNamesH;
QHash<QString, int> ItemInfo::m_ItemNamesH;
QList<QString> ItemInfo::m_SortedNames;
QList<QList<ItemInfo::Effect> > ItemInfo::m_RegEffects;
QList<QList<ItemInfo::Effect> > ItemInfo::m_BerryEffects;
QList<QStringList> ItemInfo::m_RegMessages;
QList<QStringList> ItemInfo::m_BerryMessages;
QList<int> ItemInfo::m_Powers;
QList<int> ItemInfo::m_BerryPowers;
QList<int> ItemInfo::m_BerryTypes;
QList<int> ItemInfo::m_UsefulItems;
QList<QString> ItemInfo::m_SortedUsefulNames;

QList<QString> TypeInfo::m_Names;
QList<QColor> TypeInfo::m_Colors;
QString TypeInfo::m_Directory;
QList<int> TypeInfo::m_TypeVsType;

QList<QPixmap> TypeInfo::m_Pics;

QList<QString> NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QList<QString> CategoryInfo::m_Names;
QList<QColor> CategoryInfo::m_Colors;
QString CategoryInfo::m_Directory;

QList<QString> AbilityInfo::m_Names;
QString AbilityInfo::m_Directory;
QList<AbilityInfo::Effect> AbilityInfo::m_Effects;
QList<QStringList> AbilityInfo::m_Messages;

QList<QString> GenderInfo::m_Names;
QList<QPixmap> GenderInfo::m_Pictures;
QList<QPixmap> GenderInfo::m_BattlePictures;
QString GenderInfo::m_Directory;

QString HiddenPowerInfo::m_Directory;

QString StatInfo::m_Directory;
QList<QString> StatInfo::m_stats;
QList<QString> StatInfo::m_status;
QHash<int, QPixmap> StatInfo::m_statusIcons;
QHash<int, QPixmap> StatInfo::m_battleIcons;

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

static void fill_container_with_file(QList<QColor> &container, const QString &filename)
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

static void fill_container_with_file(QList<bool> &container, const QString & filename)
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

static void fill_container_with_file(QList<char> &container, const QString & filename)
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

QString PokemonInfo::Desc(int poke, int cartridge)
{
    int orpoke = OriginalForme(poke);
    return get_line(trFile(path("description_%1").arg(cartridge)), orpoke);
}

QString PokemonInfo::Classification(int poke)
{
    int orpoke = OriginalForme(poke);
    return get_line(trFile(path("classification")), orpoke);
}

QString PokemonInfo::Height(int poke)
{
    return get_line(path("height.txt"), poke);
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

int PokemonInfo::Stat(int poke, int stat, int level, quint8 dv, quint8 ev)
{
    quint8 basestat = PokemonInfo::BaseStats(poke).baseStat(stat);
    if (stat == Hp) {
        /* Shedinja */
        if (poke == Pokemon::Shedinja)
            return 1;
        else
            return calc_stat(basestat, level, dv, ev) + level + 5;
    }
    else
	return calc_stat(basestat, level, dv, ev);
}

int PokemonInfo::FullStat(int poke, int nature, int stat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp) {
        return Stat(poke, stat, level, dv, ev);
    }
    else {
        return Stat(poke, stat, level, dv, ev) * (10+NatureInfo::Boost(nature, stat)) / 10;
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
    loadFormes();
    loadEvos();
    loadMoves();
    fill_container_with_file(m_Type1, path("poke_type1.txt"));
    fill_container_with_file(m_Type2, path("poke_type2.txt"));
    fill_container_with_file(m_Genders, path("poke_gender.txt"));
    fill_container_with_file(m_Ability1, path("poke_ability.txt"));
    fill_container_with_file(m_Ability2, path("poke_ability2.txt"));
    fill_container_with_file(m_LevelBalance, path("level_balance.txt"));
    loadBaseStats();
}

void PokemonInfo::loadEvos()
{
    QFile in(path("evolutions.txt"));
    in.open(QIODevice::ReadOnly);
    QList<QString> l = QString::fromUtf8(in.readAll()).split('\n');
    for (int i = 0; i < l.size(); i++) {
        m_OriginalEvos.push_back(0);
    }
    for (int i = 0; i < l.size(); i++) {
        if (l[i].length() > 0) {
            int preEvo = l[i].toInt();
            int orEvo = m_OriginalEvos[preEvo] == 0 ? preEvo : m_OriginalEvos[preEvo];
            m_OriginalEvos[i] = orEvo;
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

int PokemonInfo::TrueCount()
{
    return m_trueNumberOfPokes;
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

int PokemonInfo::AestheticFormeId(int pokenum)
{
    return m_AestheticFormes.value(pokenum).first;
}

QPixmap PokemonInfo::Picture(int pokenum, int forme, int gender, bool shiney, bool back)
{
    pokenum = forme == 0 ? pokenum : AestheticFormeId(pokenum) + forme;

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

QByteArray PokemonInfo::Cry(int num)
{
    QString archive = path("cries.zip");
    QString file = QString("%1.wav").arg(num).rightJustified(7, '0');

    QByteArray data = readZipFile(archive.toUtf8(),file.toUtf8());
    if(data.length() == 0)
    {
        qDebug() << "error loading pokemon cry " << num;
    }

    return data;
}

QSet<int> PokemonInfo::Moves(int pokenum)
{
    QSet<int> moves;
    return moves.unite(RegularMoves(pokenum,3)).unite(RegularMoves(pokenum, 4)).unite(EggMoves(pokenum,3)).unite(EggMoves(pokenum,4))
            .unite(SpecialMoves(pokenum,3)).unite(SpecialMoves(pokenum,4));
}

QSet<int> PokemonInfo::RegularMoves(int pokenum, int gen)
{
    return m_Moves[pokenum].regularMoves[gen-3];
}

QSet<int> PokemonInfo::EggMoves(int pokenum, int gen)
{
    return m_Moves[pokenum].eggMoves[gen-3];
}

QSet<int> PokemonInfo::LevelMoves(int pokenum, int gen)
{
    return m_Moves[pokenum].levelMoves[gen-3];
}

QSet<int> PokemonInfo::TutorMoves(int pokenum, int gen)
{
    return m_Moves[pokenum].tutorMoves[gen-3];
}

QSet<int> PokemonInfo::TMMoves(int pokenum)
{
    return m_Moves[pokenum].TMMoves;
}

QSet<int> PokemonInfo::SpecialMoves(int pokenum, int gen)
{
    return m_Moves[pokenum].specialMoves[gen-3];
}

QSet<int> PokemonInfo::PreEvoMoves(int pokenum, int gen)
{
    return m_Moves[pokenum].preEvoMoves[gen-3];
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

void PokemonInfo::loadFormes()
{
    QFile in(path("poke_formes.txt"));
    in.open(QIODevice::ReadOnly);
    QList<QString> l = QString::fromUtf8(in.readAll()).split('\n');
    for (int i = 0; i < l.size(); i++) {
        if (l[i].length() > 0) {
            if (i < TrueCount()) {
                bool hidden = l[i].leftRef(2) == "H-";
                QStringList begEnd = hidden?l[i].mid(2).split('~') : l[i].split('~');
                m_AestheticFormes[i] = QPair<int,int>(begEnd[0].toInt(), begEnd[1].toInt());
                m_AestheticFormesHidden[i] = hidden;
            } else {
                bool hidden = l[i].leftRef(2) == "H-";
                int pok = hidden ? l[i].mid(2).toInt() : l[i].toInt();

                if (!hidden) {
                    if (m_AlternateFormes[pok].empty())
                        m_AlternateFormes[pok].push_back(pok);
                    m_AlternateFormes[pok].push_back(i);
                }
                m_AlternateFormes[i].push_back(pok);
            }
        }
    }
    in.close();
    in.setFileName(trFile(path("formes_desc")));
    in.open(QIODevice::ReadOnly);

    l = QString::fromUtf8(in.readAll()).split('\n');
    foreach(QString s, l) {
        m_AestheticFormesDescs[s.section('-',0,0).toInt()] = s.section('-',1);
    }
}

QString PokemonInfo::AestheticDesc(int pokenum, int forme)
{
    return m_AestheticFormesDescs.value(AestheticFormeId(pokenum) + forme);
}

void PokemonInfo::loadNames()
{
    QFile in (trFile(path("pokemons")));
    in.open(QIODevice::ReadOnly);

    m_trueNumberOfPokes = QString::fromUtf8(in.readLine()).trimmed().toInt();
    m_Names = QString::fromUtf8(in.readAll()).split('\n');

    fill_container_with_file(m_Weights, path("poke_weight.txt"));
}

bool PokemonInfo::HasFormes(int pokenum)
{
    return IsForme(pokenum) ? HasFormes(OriginalForme(pokenum)) : m_AlternateFormes.contains(pokenum);
}

bool PokemonInfo::HasAestheticFormes(int pokenum)
{
    return m_AestheticFormes.contains(pokenum);
}

bool PokemonInfo::AFormesShown(int pokenum)
{
    return !m_AestheticFormesHidden.value(pokenum);
}

int PokemonInfo::NumberOfAFormes(int pokenum)
{
    return m_AestheticFormes.value(pokenum).second - m_AestheticFormes.value(pokenum).first + 1;
}

bool PokemonInfo::IsForme(int pokenum)
{
    return pokenum >= TrueCount();
}

int PokemonInfo::OriginalForme(int pokenum)
{
    if (!IsForme(pokenum))
        return pokenum;
    else
        return m_AlternateFormes[pokenum].front();
}

QList<int> PokemonInfo::Formes(int pokenum)
{
    if (!HasFormes(pokenum))
        return QList<int>();
    else if (IsForme(pokenum))
        return Formes(OriginalForme(pokenum));
    else
        return m_AlternateFormes[pokenum];
}

int PokemonInfo::OriginalEvo(int pokenum)
{
    return m_OriginalEvos[pokenum];
}

QList<int> PokemonInfo::Evos(int pokenum)
{
    return m_Evolutions.value(OriginalEvo(pokenum));
}

bool PokemonInfo::IsInEvoChain(int pokenum)
{
    return OriginalEvo(pokenum) != 0;
}

void PokemonInfo::loadMoves()
{
    static const int filesize = 10;
    QFile files[filesize];

    QString fileNames[filesize] = {
        path("tm_and_hm_moves.txt"), path("3G_egg_moves.txt"), path("3G_level_moves.txt"),
        path("3G_tutor_moves.txt"), path("3G_special_moves.txt"), path("4G_pre_evo_moves.txt"),
        path("4G_egg_moves.txt"), path("4G_level_moves.txt"), path("4G_tutor_moves.txt"),
        path("4G_special_moves.txt")
    };

    for (int i = 0; i < filesize; i++) {
        files[i].setFileName(fileNames[i]);
        files[i].open(QIODevice::ReadOnly);
    }

    for (int i = 0; i < NumberOfPokemons(); i++) {
        PokemonMoves moves;

        QSet<int> *refs[filesize] = {
            &moves.TMMoves, &moves.eggMoves[0], &moves.levelMoves[0], &moves.tutorMoves[0], &moves.specialMoves[0],
            &moves.preEvoMoves[1], &moves.eggMoves[1], &moves.levelMoves[1], &moves.tutorMoves[1], &moves.specialMoves[1]
        };

        for (int j = 0; j < filesize; j++) {
            QList<QByteArray> line = files[j].readLine().trimmed().split(' ');
            foreach(QByteArray data, line) {
                if (data.length() > 0)
                    refs[j]->insert(data.toInt());
            }
        }

        moves.regularMoves[0] = moves.TMMoves;
        moves.regularMoves[0].unite(moves.levelMoves[0]).unite(moves.tutorMoves[0]);
        moves.regularMoves[1] = moves.TMMoves;
        moves.regularMoves[1].unite(moves.preEvoMoves[1]).unite(moves.levelMoves[1]).unite(moves.tutorMoves[1]);

        m_Moves.push_back(moves);
    }
}

QString PokemonInfo::path(const QString &filename)
{
    return m_Directory + filename;
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

    m_SortedNames << m_RegItemNames << m_BerryNames;
    qSort(m_SortedNames);

    m_SortedUsefulNames << m_BerryNames;

    for (int i = 0; i < m_RegItemNames.size(); i++) {
        if (isUseful(i))
            m_SortedUsefulNames.push_back(m_RegItemNames[i]);
    }
    qSort(m_SortedUsefulNames);

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
    return Effects(itemnum).front().args.toInt();
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
    return (m_Weights[pokenum].toFloat()-0.02f)/2.2f; /* the -0.02 is just a trick to compensate the poor precision of floats, for moves like grass knot */
}

QString PokemonInfo::WeightS(int pokenum)
{
    return m_Weights[pokenum];
}

int PokemonInfo::BaseGender(int pokenum)
{
    int avail = Gender(pokenum);

    return (avail == Pokemon::MaleAvail || avail == Pokemon::MaleAndFemaleAvail) ?
            Pokemon::Male : (avail == Pokemon::NeutralAvail ? Pokemon::Neutral : Pokemon::Female);
}

int ItemInfo::SortedNumber(const QString &itemname)
{
    return (qLowerBound(m_SortedNames, itemname) - m_SortedNames.begin()) % (NumberOfItems());
}


QList<QString> ItemInfo::SortedNames()
{
    return m_SortedNames;
}

QList<QString> ItemInfo::SortedUsefulNames()
{
    return m_SortedUsefulNames;
}

void TypeInfo::loadNames()
{
    fill_container_with_file(m_Names, trFile(path("types")));

    if (PokemonInfoConfig::config() == PokemonInfoConfig::Gui) {
        for (int i = 0; i < NumberOfTypes();i++) {
            m_Pics.push_back(QPixmap(path(QString("type%1.png").arg(i))));
        }
    }
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

QPixmap TypeInfo::Picture(int type)
{
    return (type >= 0 && type < NumberOfTypes()) ? m_Pics[type] : QPixmap();
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

    if (PokemonInfoConfig::config() == PokemonInfoConfig::Gui) {
        m_statusIcons[-2] = QPixmap(path("status-2.png"));
        m_battleIcons[-2] = QPixmap(path("battle_status-2.png"));

        for (int i = 0; i < 7; i++) {
            m_statusIcons[i] = QPixmap(path("status%1.png").arg(i));
            m_battleIcons[i] = QPixmap(path("battle_status%1.png").arg(i));
        }
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
