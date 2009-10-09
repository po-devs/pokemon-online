#include "pokemoninfo.h"
#include <zzip/lib.h>

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

PokemonInfo::PokemonInfo(const QString &dir)
	    : m_Directory(dir)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    loadNames();
}

int PokemonInfo::NumberOfPokemons() const
{
    return m_NumOfPokemons;
}

QString PokemonInfo::Name(int pokenum) const
{
    return m_Names[pokenum];
}

int PokemonInfo::Number(const QString &pokename) const
{
    return (qFind(m_Names, m_Names+NumberOfPokemons()+1, pokename)-m_Names) % (NumberOfPokemons()+1);
}

Pokemon::GenderAvail PokemonInfo::Gender(int pokenum) const
{
    return Pokemon::GenderAvail(__get_line(m_Directory + "poke_gender.txt", pokenum).toInt());
}

QPixmap PokemonInfo::Picture(int pokenum, Pokemon::Gender gender, bool shiney) const
{
    QString path = QString("%1poke_img/%2/DP%3%4.png").arg(m_Directory).arg(pokenum).arg((gender==Pokemon::Female)?"f":"m", shiney?"s":"");
    ZZIP_FILE *file = zzip_open(path.toAscii(), 0);

    if (file == NULL)
    {
	/* We change the gender/shininess to try to find a valid file */
	if (shiney)
	{
	    path = QString("%1poke_img/%2/DP%3.png").arg(m_Directory).arg(pokenum).arg((gender==Pokemon::Female)?"f":"m");
	    file = zzip_open(path.toAscii(), 0);
	}
	if (file == NULL)
	{
	    path = QString("%1poke_img/%2/DP%3.png").arg(m_Directory).arg(pokenum).arg((gender==Pokemon::Female)?"m":"f");
	    file = zzip_open(path.toAscii(), 0);
	}
	if (file == NULL)
	    return QPixmap();
    }

    /* Using C style manipulations to get the file */
    zzip_seek(file, 0, SEEK_END);
    zzip_off_t filesize = zzip_tell(file);
    zzip_seek(file, 0, SEEK_SET);

    QByteArray buffer;
    buffer.reserve(filesize);

    zzip_ssize_t read_size = zzip_read(file, buffer.data(), filesize);

    if (read_size < 0 || read_size < filesize)
	return QPixmap();

    QPixmap ret;
    ret.loadFromData(reinterpret_cast<uchar*>(buffer.data()), filesize, "png");

    return ret;
}

QLinkedList<int> PokemonInfo::Moves(int pokenum) const
{
    return EggMoves(pokenum) << LevelMoves(pokenum) << TutorMoves(pokenum) << TMMoves(pokenum) << SpecialMoves(pokenum);
}

QLinkedList<int> PokemonInfo::EggMoves(int pokenum) const
{
    return getMoves("pokes_DP_eggmoves.txt", pokenum);
}

QLinkedList<int> PokemonInfo::LevelMoves(int pokenum) const
{
    return getMoves("pokes_DP_lvmoves.txt", pokenum);
}

QLinkedList<int> PokemonInfo::TutorMoves(int pokenum) const
{
    return getMoves("pokes_DP_MTmoves.txt", pokenum);
}

QLinkedList<int> PokemonInfo::TMMoves(int pokenum) const
{
    return getMoves("pokes_DP_TMmoves.txt", pokenum);
}

QLinkedList<int> PokemonInfo::SpecialMoves(int pokenum) const
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



/** Private **/
void PokemonInfo::loadNames()
{
    QFile pokenames(m_Directory + "pokemons_en.txt");

    //TODO: exception
    pokenames.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream namestream(&pokenames);

    for (int i = 0; i <= NumberOfPokemons(); i++)
    {
	m_Names[i] = namestream.readLine();
    }
}

QLinkedList<int> PokemonInfo::getMoves(const QString &filename, int pokenum) const
{
    QLinkedList<int> return_value;

    /* getting the line we want */
    QString interesting_line = __get_line(m_Directory + filename, pokenum);

    /* extracting the moves */
    for (int i = 0; i + 3 <= interesting_line.length(); i++)
    {
	return_value << interesting_line.mid(i,3).toUInt();
    }

    return return_value;
}
