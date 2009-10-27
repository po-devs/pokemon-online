#include "pokemoninfo.h"
#include "zip.h"

/*initialising static variables */
QString PokemonInfo::m_Directory;
QStringList PokemonInfo::m_Names;

QString MoveInfo::m_Directory;
QStringList MoveInfo::m_Names;

QString ItemInfo::m_Directory;
QStringList ItemInfo::m_Names;
QStringList ItemInfo::m_SortedNames;

QStringList TypeInfo::m_Names;
QList<QColor> TypeInfo::m_Colors;
QString TypeInfo::m_Directory;

QStringList NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QStringList CategoryInfo::m_Names;
QList<QColor> CategoryInfo::m_Colors;
QString CategoryInfo::m_Directory;

QStringList AbilityInfo::m_Names;
QString AbilityInfo::m_Directory;

QStringList GenderInfo::m_Names;
QList<QPixmap> GenderInfo::m_Pictures;
QString GenderInfo::m_Directory;

QString HiddenPowerInfo::m_Directory;

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

template <class T>
void fill_container_with_file(T &container, const QString & filename)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    while (!filestream.atEnd())
    {
        container << filestream.readLine();
    }
}

PokeBaseStats::PokeBaseStats(quint8 base_hp, quint8 base_att, quint8 base_def, quint8 base_spd, quint8 base_spAtt, quint8 base_spDef)
{
    setBaseHp(base_hp);
    setBaseAttack(base_att);
    setBaseDefense(base_def);
    setBaseSpeed(base_spd);
    setBaseSpAttack(base_spAtt);
    setBaseSpDefense(base_spDef);
}

quint8 PokeBaseStats::baseStat(int stat) const
{
    return m_BaseStats[stat];
}

quint8 PokeBaseStats::baseHp() const
{
    return baseStat(Hp);
}

quint8 PokeBaseStats::baseAttack() const
{
    return baseStat(Attack);
}

quint8 PokeBaseStats::baseDefense() const
{
    return baseStat(Defense);
}

quint8 PokeBaseStats::baseSpeed() const
{
    return baseStat(Speed);
}

quint8 PokeBaseStats::baseSpAttack() const
{
    return baseStat(SpAttack);
}

quint8 PokeBaseStats::baseSpDefense() const
{
    return baseStat(SpDefense);
}

void PokeBaseStats::setBaseHp(quint8 hp)
{
    setBaseStat(Hp, hp);
}

void PokeBaseStats::setBaseAttack(quint8 att)
{
    setBaseStat(Attack, att);
}

void PokeBaseStats::setBaseDefense(quint8 def)
{
    setBaseStat(Defense, def);
}

void PokeBaseStats::setBaseSpeed(quint8 speed)
{
    setBaseStat(Speed, speed);
}

void PokeBaseStats::setBaseSpAttack(quint8 spAtt)
{
    setBaseStat(SpAttack, spAtt);
}

void PokeBaseStats::setBaseSpDefense(quint8 spDef)
{
    setBaseStat(SpDefense, spDef);
}

void PokeBaseStats::setBaseStat(int stat, quint8 base)
{
    m_BaseStats[stat] = base;
}

PokeGeneral::PokeGeneral(): m_num(0)
{
}

void PokeGeneral::setNum(int num)
{
    m_num = num;
}

int PokeGeneral::num() const
{
    return m_num;
}

void PokeGeneral::loadBaseStats()
{
    setBaseStats(PokemonInfo::BaseStats(num()));
}

void PokeGeneral::loadMoves()
{
    m_moves = PokemonInfo::Moves(num());
}

void PokeGeneral::loadTypes()
{
}

void PokeGeneral::loadAbilities()
{
    m_abilities = PokemonInfo::Abilities(num());
}

void PokeGeneral::loadGenderAvail()
{
    m_genderAvail = PokemonInfo::Gender(num());
}

void PokeGeneral::load()
{
    loadBaseStats();
    loadMoves();
    loadTypes();
    loadAbilities();
    loadGenderAvail();
}

const QList<int> &PokeGeneral::moves() const
{
    return m_moves;
}

const QList<int> &PokeGeneral::abilities() const
{
    return m_abilities;
}

int PokeGeneral::genderAvail() const
{
    return m_genderAvail;
}

void PokeGeneral::setBaseStats(const PokeBaseStats &stats)
{
    m_stats = stats;
}

const PokeBaseStats & PokeGeneral::baseStats() const
{
    return m_stats;
}

PokePersonal::PokePersonal()
{
    reset();
}

void PokePersonal::setNickname(const QString &nick)
{
    m_nickname = nick;
}

void PokePersonal::setItem(int item)
{
    m_item = item;
}

void PokePersonal::setAbility(int ability)
{
    m_ability = ability;
}

void PokePersonal::setNature(int nature)
{
    m_nature = nature;
}

void PokePersonal::setGender(int gender)
{
    m_gender = gender;
}

void PokePersonal::setShininess(bool shiny)
{
    m_shininess = shiny;
}

void PokePersonal::setHappiness(quint8 happiness)
{
    m_happiness = happiness;
}

void PokePersonal::setLevel(int level)
{
    m_level = level;
}

void PokePersonal::setMove(int moveNum, int moveSlot)
{
    if (moveNum == move(moveSlot))
        return;
    if (moveNum != 0 && hasMove(moveNum))
        throw QObject::tr("%1 already has move %2.").arg(PokemonInfo::Name(num()), MoveInfo::Name(moveNum));

    m_moves[moveSlot] = moveNum;
}

int PokePersonal::addMove(int moveNum)
{
    for (int i = 0; i < 4; i++)
        if (move(i) == 0) {
            setMove(moveNum, i);
            return i;
        }
    throw QObject::tr("No free move available!");
}

bool PokePersonal::hasMove(int moveNum)
{
    for (int i = 0; i < 4; i++)
        if (move(i) == moveNum)
            return true;
    return false;
}

void PokePersonal::setNum(int num)
{
    m_num = num;
}

void PokePersonal::setDV(int stat, quint8 val)
{
    m_DVs[stat] = val;
}

void PokePersonal::setHpDV(quint8 val)
{
    setDV(Hp, val);
}

void PokePersonal::setAttackDV(quint8 val)
{
    setDV(Attack, val);
}

void PokePersonal::setDefenseDV(quint8 val)
{
    setDV(Defense, val);
}

void PokePersonal::setSpeedDV(quint8 val)
{
    setDV(Speed, val);
}

void PokePersonal::setSpAttackDV(quint8 val)
{
    setDV(SpAttack, val);
}

void PokePersonal::setSpDefenseDV(quint8 val)
{
    setDV(SpDefense, val);
}

void PokePersonal::controlEVs(int stat)
{
    int sum = EVSum();

    //if overflow we set it back to the limit
    if (sum > 510)
    {
        /* why do something so complicated? in case it's way over the limit and not simply because of stat ,
            we don't want something nasty induced by negative overflow */
        if (sum - 510 > m_EVs[stat])
            m_EVs[stat] = 0;
        else
            m_EVs[stat] -=  sum - 510;
    }
}

void PokePersonal::setEV(int stat, quint8 val)
{
    m_EVs[stat] = val;
    controlEVs(stat);
}

void PokePersonal::setHpEV(quint8 val)
{
    setEV(Hp, val);
}

void PokePersonal::setAttackEV(quint8 val)
{
    setEV(Attack, val);
}

void PokePersonal::setDefenseEV(quint8 val)
{
    setEV(Defense, val);
}

void PokePersonal::setSpeedEV(quint8 val)
{
    setEV(Speed, val);
}

void PokePersonal::setSpAttackEV(quint8 val)
{
    setEV(SpAttack, val);
}

void PokePersonal::setSpDefenseEV(quint8 val)
{
    setEV(SpDefense, val);
}

quint8 PokePersonal::DV(int stat) const
{
    return m_DVs[stat];
}

quint8 PokePersonal::hpDV() const
{
    return DV(Hp);
}

quint8 PokePersonal::attackDV() const
{
    return DV(Attack);
}

quint8 PokePersonal::defenseDV() const
{
    return DV(Defense);
}

quint8 PokePersonal::speedDV() const
{
    return DV(Speed);
}

quint8 PokePersonal::spAttackDV() const
{
    return DV(SpAttack);
}

quint8 PokePersonal::spDefenseDV() const
{
    return DV(SpDefense);
}

int PokePersonal::EVSum() const
{
    return hpEV() + attackEV() + defenseEV() + speedEV() + spAttackEV() + spDefenseEV();
}

quint8 PokePersonal::EV(int stat) const
{
    return m_EVs[stat];
}

quint8 PokePersonal::hpEV() const
{
    return EV(Hp);
}

quint8 PokePersonal::attackEV() const
{
    return EV(Attack);
}

quint8 PokePersonal::defenseEV() const
{
    return EV(Defense);
}

quint8 PokePersonal::speedEV() const
{
    return EV(Speed);
}

quint8 PokePersonal::spAttackEV() const
{
    return EV(SpAttack);
}

quint8 PokePersonal::spDefenseEV() const
{
    return EV(SpDefense);
}

QString PokePersonal::nickname() const
{
    return m_nickname;
}

int PokePersonal::num() const
{
    return m_num;
}

int PokePersonal::item() const
{
    return m_item;
}

int PokePersonal::ability() const
{
    return m_ability;
}

int PokePersonal::nature() const
{
    return m_nature;
}

int PokePersonal::gender() const
{
    return m_gender;
}

bool PokePersonal::shiny() const
{
    return m_shininess;
}

quint8 PokePersonal::happiness() const
{
    return m_happiness;
}

int PokePersonal::level() const
{
    return m_level;
}

int PokePersonal::move(int moveSlot) const
{
    return m_moves[moveSlot];
}

int PokePersonal::natureBoost(int stat) const
{
    return NatureInfo::Boost(nature(), stat);
}

void PokePersonal::reset()
{
    setNum(0);
    setLevel(100);
    for (int i = 0; i < 4; i++)
        setMove(0,i);
    setHappiness(255);
    setShininess(false);
    setGender(0);
    setAbility(0);
    setNickname("");
    setNature(0);
    setItem(0);

    setHpDV(31);
    setAttackDV(31);
    setDefenseDV(31);
    setSpeedDV(31);
    setSpAttackDV(31);
    setSpDefenseDV(31);

    setHpEV(0);
    setAttackEV(0);
    setDefenseEV(0);
    setSpeedEV(0);
    setSpAttackEV(0);
    setSpDefenseEV(0);
}

PokeGraphics::PokeGraphics()
        : m_num(0), m_uptodate(false)
{
}

void PokeGraphics::setNum(int num)
{
    m_num = num;
    setUpToDate(false);
}

void PokeGraphics::setUpToDate(bool uptodate)
{
    m_uptodate = uptodate;
}

bool PokeGraphics::upToDate() const
{
    return m_uptodate;
}

void PokeGraphics::load(int gender, bool shiny)
{
    if (upToDate() && gender==m_storedgender && shiny == m_storedshininess)
        return;

    m_storedgender = gender;
    m_storedshininess = shiny;
    m_picture = PokemonInfo::Picture(num(), gender, shiny);
    setUpToDate(true);
}

QPixmap PokeGraphics::picture()
{
    return m_picture;
}

QPixmap PokeGraphics::picture(int gender, bool shiny)
{
    load(gender, shiny);
    return picture();
}

int PokeGraphics::num() const
{
    return m_num;
}

PokeTeam::PokeTeam()
{
    setNum(0);
}

void PokeTeam::setNum(int num)
{
    PokeGeneral::setNum(num);
    PokePersonal::setNum(num);
    PokeGraphics::setNum(num);
}

int PokeTeam::num() const
{
    return PokeGeneral::num();
}

void PokeTeam::load()
{
    PokeGeneral::load();
    /*set the default gender & ability */
    if (genderAvail() == Pokemon::NeutralAvail)
    {
        setGender(Pokemon::Neutral);
    }
    else if (genderAvail() == Pokemon::FemaleAvail)
    {
        setGender(Pokemon::Female);
    }
    else
    {
        setGender(Pokemon::Male);
    }
    setAbility(abilities()[0]);
    setNickname(PokemonInfo::Name(num()));
    PokeGraphics::load(gender(), false);
}

int PokeTeam::calc_stat(quint8 basestat, int level, quint8 ev, quint8 dv) const
{
    return ((2*basestat + dv+ ev/4)*level)/100 + 5;
}

QPixmap PokeTeam::picture()
{
    return PokeGraphics::picture(gender(), shiny());
}

int PokeTeam::calc_stat_F(int stat) const
{
    return calc_stat(baseStats().baseStat(stat), level(), EV(stat), DV(stat)) * (10+natureBoost(stat))/10;
}

int PokeTeam::stat(int statno) const
{
    if (statno == Hp)
        return hp();
    else
        return calc_stat_F(statno);
}

int PokeTeam::hp() const
{
    return calc_stat(baseStats().baseHp(), level(), hpEV(), hpDV()) + level() + 5;
}

int PokeTeam::attack() const
{
    return calc_stat_F(Attack);
}

int PokeTeam::defense() const
{
    return calc_stat_F(Defense);
}

int PokeTeam::speed() const
{
    return calc_stat_F(Speed);
}

int PokeTeam::spAttack() const
{
    return calc_stat_F(SpAttack);
}

int PokeTeam::spDefense() const
{
    return calc_stat_F(SpDefense);
}

Team::Team()
{
}

const PokeTeam & Team::poke(int index) const
{
    return m_pokes[index];
}

PokeTeam & Team::poke(int index)
{
    return m_pokes[index];
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
}

int PokemonInfo::NumberOfPokemons()
{
    return m_Names.size();
}

QString PokemonInfo::Name(int pokenum)
{
    return m_Names[pokenum];
}

int PokemonInfo::Number(const QString &pokename)
{
    return (qFind(m_Names.begin(), m_Names.end(), pokename)-m_Names.begin()) % (NumberOfPokemons());
}

int PokemonInfo::Gender(int pokenum)
{
    return Pokemon::GenderAvail(get_line(path("poke_gender.txt"), pokenum).toInt());
}

QPixmap PokemonInfo::Picture(int pokenum, int gender, bool shiney)
{
    QString archive = path("poke_img.zip");

    QString file = QString("%2/DP%3%4.png").arg(pokenum).arg((gender==Pokemon::Female)?"f":"m", shiney?"s":"");

    QByteArray data = readZipFile(archive.toLocal8Bit(), file.toLocal8Bit());

    if (data.length()==0)
    {
        /* We change the gender/shininess to try to find a valid file */
        if (shiney)
        {
            file = QString("%2/DP%3.png").arg(pokenum).arg((gender==Pokemon::Female)?"f":"m");
            data = readZipFile(archive.toLocal8Bit(), file.toLocal8Bit());
        }
        if (data.length()==0)
        {
            file = QString("%2/DP%3.png").arg(pokenum).arg((gender==Pokemon::Female)?"m":"f");
            data = readZipFile(archive.toLocal8Bit(), file.toLocal8Bit());
        }
        if (data.length()==0)
            return QPixmap();
    }

    QPixmap ret;
    ret.loadFromData(data, "png");

    return ret;
}

QList<int> PokemonInfo::Moves(int pokenum)
{
    return LevelMoves(pokenum) << EggMoves(pokenum) << TutorMoves(pokenum) << TMMoves(pokenum) << SpecialMoves(pokenum);
}

QList<int> PokemonInfo::EggMoves(int pokenum)
{
    return getMoves("pokes_DP_eggmoves.txt", pokenum);
}

QList<int> PokemonInfo::LevelMoves(int pokenum)
{
    return getMoves("pokes_DP_lvmoves.txt", pokenum);
}

QList<int> PokemonInfo::TutorMoves(int pokenum)
{
    return getMoves("pokes_DP_MTmoves.txt", pokenum);
}

QList<int> PokemonInfo::TMMoves(int pokenum)
{
    return getMoves("pokes_DP_TMmoves.txt", pokenum);
}

QList<int> PokemonInfo::SpecialMoves(int pokenum)
{
    return getMoves("pokes_DP_specialmoves.txt", pokenum);
}

QList<int> PokemonInfo::Abilities(int pokenum)
{
    QList<int> ret;
    ret << get_line(path("poke_ability.txt"), pokenum).toInt() << get_line(path("poke_ability2.txt"), pokenum).toInt();

    return ret;
}

PokeBaseStats PokemonInfo::BaseStats(int pokenum)
{
    QString stats = get_line(path("poke_stats.txt"), pokenum);
    QTextStream statsstream(&stats, QIODevice::ReadOnly);

    int hp, att, def, spd, satt, sdef;

    statsstream >> hp >> att >> def >> spd >> satt >> sdef;

    return PokeBaseStats(hp, att, def, spd, satt, sdef);
}

void PokemonInfo::loadNames()
{
    fill_container_with_file(m_Names, path("pokemons_en.txt"));
}

QString PokemonInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

QList<int> PokemonInfo::getMoves(const QString &filename, int pokenum)
{
    QList<int> return_value;

    /* getting the line we want */
    QString interesting_line = get_line(path(filename), pokenum);

    /* extracting the moves */
    for (int i = 0; i + 3 <= interesting_line.length(); i+=3)
    {
        return_value << interesting_line.mid(i,3).toUInt();
    }

    return return_value;
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
}

int MoveInfo::NumberOfMoves()
{
    return m_Names.size();
}

int MoveInfo::Number(const QString &movename)
{
    return (qFind(m_Names.begin(), m_Names.end(), movename)-m_Names.begin()) % (NumberOfMoves());
}


void MoveInfo::loadNames()
{
    fill_container_with_file(m_Names, path("moves_en.txt"));
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}

QString MoveInfo::Name(int movenum)
{
    return m_Names[movenum];
}

int MoveInfo::Power(int movenum)
{
    return get_line(path("move_power.txt"), movenum).toInt();
}

int MoveInfo::Type(int movenum)
{
    return get_line(path("move_type.txt"), movenum).toInt();
}

int MoveInfo::Category(int movenum)
{
    return get_line(path("move_category.txt"), movenum).toInt();
}

int MoveInfo::PP(int movenum)
{
    return get_line(path("move_pp.txt"), movenum).toInt();
}

QString MoveInfo::AccS(int movenum)
{
    return get_line(path("move_accuracy.txt"), movenum);
}

QString MoveInfo::PowerS(int movenum)
{
    return get_line(path("move_power.txt"), movenum);
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
    fill_container_with_file(m_Names, path("items_en.txt"));
    fill_container_with_file(m_Names, path("berries_en.txt"));

    m_SortedNames = m_Names;
    qSort(m_SortedNames);
}

QString ItemInfo::path(const QString &file)
{
    return m_Directory + file;
}

int ItemInfo::NumberOfItems()
{
    return m_Names.size();
}

QString ItemInfo::Name(int itemnum)
{
    return m_Names[itemnum];
}

int ItemInfo::Number(const QString &itemname)
{
    return (qFind(m_Names.begin(), m_Names.end(), itemname)-m_Names.begin()) % (NumberOfItems());
}

int ItemInfo::SortedNumber(const QString &itemname)
{
    return (qFind(m_SortedNames.begin(), m_SortedNames.end(), itemname)-m_SortedNames.begin()) % (NumberOfItems());
}

QStringList ItemInfo::Names()
{
    return m_Names;
}

QStringList ItemInfo::SortedNames()
{
    return m_SortedNames;
}

void TypeInfo::loadNames()
{
    fill_container_with_file(m_Names, path("types_en.txt"));
}

QString TypeInfo::path(const QString& file)
{
    return m_Directory+file;
}

void TypeInfo::loadColors()
{
    fill_container_with_file(m_Colors, path("type_colors.txt"));
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
    fill_container_with_file(m_Names, path("nature_en.txt"));
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

int NatureInfo::Boost(int nature, int stat)
{
    return -(nature%5 == stat-1) + (nature/5 == stat-1);
}


void CategoryInfo::loadNames()
{
    fill_container_with_file(m_Names, path("categories_en.txt"));
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
    fill_container_with_file(m_Names, path("abilities_en.txt"));
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
}

QString AbilityInfo::Name(int abnum)
{
    return m_Names[abnum];
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

QPixmap GenderInfo::Picture(int gender)
{
    return m_Pictures[gender];
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
    QList<QString> fileLines;

    fill_container_with_file(fileLines, path(QString("type%1_hp.txt").arg(type)));

    QList<QStringList> ret;

    foreach (QString line, fileLines)
        ret.push_back(line.split(' '));

    return ret;
}

TrainerTeam::TrainerTeam()
{
}

QString TrainerTeam::trainerInfo() const
{
    return m_trainerInfo;
}

QString TrainerTeam::trainerLose() const
{
    return m_trainerLose;
}

QString TrainerTeam::trainerWin() const
{
    return m_trainerWin;
}

QString TrainerTeam::trainerNick() const
{
    return m_trainerNick;
}


void TrainerTeam::setTrainerInfo(const QString &newinfo)
{
    m_trainerInfo = newinfo;
}

void TrainerTeam::setTrainerWin(const QString &newwin)
{
    m_trainerWin = newwin;
}

void TrainerTeam::setTrainerLose(const QString &newlose)
{
    m_trainerLose = newlose;
}

void TrainerTeam::setTrainerNick(const QString &newnick)
{
    m_trainerNick = newnick;
}

const Team & TrainerTeam::team() const
{
    return m_team;
}

Team & TrainerTeam::team()
{
    return m_team;
}
