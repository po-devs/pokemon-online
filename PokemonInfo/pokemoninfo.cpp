#include "pokemoninfo.h"
#include <zip.h>

/*initialising static variables */
QString PokemonInfo::m_Directory;
QStringList PokemonInfo::m_Names;

QString MoveInfo::m_Directory;
QStringList MoveInfo::m_Names;

QString ItemInfo::m_Directory;
QStringList ItemInfo::m_Names;

QStringList TypeInfo::m_Names;
QList<QColor> TypeInfo::m_Colors;
QString TypeInfo::m_Directory;

QString __get_line(QString filename, int linenum)
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
    return Pokemon::GenderAvail(__get_line(m_Directory + "poke_gender.txt", pokenum).toInt());
}

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
	qDebug() << "Error when opening archive";
	return ret;
    }

    file = zip_fopen(archive, fileName, 0);

    if (!file)
    {
	qDebug() << "Error when opening file in archive: " << zip_strerror(archive);
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
	qDebug() << "Error when reading file in archive: " << zip_file_strerror(file);
    }

    zip_fclose(file);
    zip_close(archive);

    return ret;
}

QPixmap PokemonInfo::Picture(int pokenum, int gender, bool shiney)
{
    QString archive = m_Directory + "poke_img.zip";

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
    ret << __get_line("poke_ability.txt", pokenum).toInt() << __get_line("poke_ability2.txt", pokenum).toInt();

    return ret;
}

PokeBaseStats PokemonInfo::BaseStats(int pokenum)
{
    QString stats = __get_line(m_Directory + "poke_stats.txt", pokenum);
    QTextStream statsstream(&stats, QIODevice::ReadOnly);

    int hp, att, def, spd, satt, sdef;

    statsstream >> hp >> att >> def >> spd >> satt >> sdef;

    return PokeBaseStats(hp, att, def, spd, satt, sdef);
}

void PokemonInfo::loadNames()
{
    QFile pokenames(m_Directory + "pokemons_en.txt");

    //TODO: exception
    pokenames.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream namestream(&pokenames);

    while(!namestream.atEnd())
    {
	m_Names << namestream.readLine();
    }
}

QList<int> PokemonInfo::getMoves(const QString &filename, int pokenum)
{
    QList<int> return_value;

    /* getting the line we want */
    QString interesting_line = __get_line(m_Directory + filename, pokenum);

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

void MoveInfo::loadNames()
{
    QFile movenames(m_Directory + "moves_en.txt");

    //TODO: exception
    movenames.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream namestream(&movenames);

    while (!namestream.atEnd())
    {
	m_Names << namestream.readLine();
    }
}

QString MoveInfo::Name(int movenum)
{
    return m_Names[movenum];
}

int MoveInfo::Power(int movenum)
{
    return __get_line(m_Directory+"move_power.txt", movenum).toInt();
}

int MoveInfo::Type(int movenum)
{
    return __get_line(m_Directory+"move_type.txt", movenum).toInt();
}

int MoveInfo::PP(int movenum)
{
    return __get_line(m_Directory+"move_pp.txt", movenum).toInt();
}

QString MoveInfo::AccS(int movenum)
{
    return __get_line(m_Directory+"move_accuracy.txt", movenum);
}

QString MoveInfo::PowerS(int movenum)
{
    return __get_line(m_Directory+"move_power.txt", movenum);
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
    QFile itemnames(m_Directory + "items_en.txt");

    //TODO: exception
    itemnames.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream namestream(&itemnames);

    while (!namestream.atEnd())
    {
	m_Names << namestream.readLine();
    }

    itemnames.close();
    itemnames.setFileName(m_Directory + "berries_en.txt");

    itemnames.open(QIODevice::ReadOnly | QIODevice::Text);

    namestream.setDevice(&itemnames);

    while (!namestream.atEnd())
    {
	m_Names << namestream.readLine();
    }
}

int ItemInfo::NumberOfItems()
{
    return m_Names.size();
}

QString ItemInfo::Name(int itemnum)
{
    return m_Names[itemnum];
}

QStringList ItemInfo::Names()
{
    return m_Names;
}

void TypeInfo::loadNames()
{
    QFile typenames(m_Directory + "types_en.txt");

    //TODO: exception
    typenames.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream namestream(&typenames);

    while (!namestream.atEnd())
    {
	m_Names << namestream.readLine();
    }
}

void TypeInfo::loadColors()
{
    QFile typecolors(m_Directory + "type_colors.txt");

    //TODO: exception
    typecolors.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream namestream(&typecolors);

    while (!namestream.atEnd())
    {
	m_Colors << QColor(namestream.readLine());
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
