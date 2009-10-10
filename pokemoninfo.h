#ifndef POKEMONINFO_H
#define POKEMONINFO_H

#include "pokemonstructs.h"
#include <QtGui>
/* A class that should be used as a singleton and provide every ressource needed on pokemons */
class PokemonInfo
{
private:
    static QStringList m_Names;
    static QString m_Directory;

    static void loadNames();
    static QList<int> getMoves(const QString &filename, int Pokenum);

public:

    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static int NumberOfPokemons();
    static QString Name(int pokenum);
    static int Number(const QString &pokename);
    static int Gender(int pokenum);
    static QPixmap Picture(int pokenum, int gender = Pokemon::Male, bool shiney = false);
    static QList<int> Moves(int pokenum);
    static QList<int> EggMoves(int pokenum);
    static QList<int> LevelMoves(int pokenum);
    static QList<int> TutorMoves(int pokenum);
    static QList<int> TMMoves(int pokenum);
    static QList<int> SpecialMoves(int pokenum);
    static PokeBaseStats BaseStats(int pokenum);
    static QList<int> Abilities(int pokenum);
};

struct Move
{
    enum Category
    {
	Physical,
	Special,
	Other
    };

    enum Type
    {
	Normal = 0,
	Fighting,
	Flying,
	Poison,
	Ground,
	Rock,
	Bug,
	Ghost,
	Steel,
	Fire,
	Water,
	Grass,
	Electric,
	Psychic,
	Ice,
	Dragon,
	Dark,
	Curse = 17
    };
};

class MoveInfo
{
private:
    static QStringList m_Names;
    static QString m_Directory;

    static void loadNames();
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int movenum);
    static int Type(int movenum);
    static QString Number(const QString &movename);
    static int NumberOfMoves();
    static QString Description(int movenum);
    static int Power(int movenum);
    /* gives the power of a move in the form of a string */
    static QString PowerS(int movenum);
    static int PP(int movenum);
    /* gives the power of a move in the form of a string */
    static QString AccS(int movenum);
};

class ItemInfo
{
private:
    static QStringList m_Names;
    static QString m_Directory;

    static void loadNames();
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static int NumberOfItems();
    static QString Name(int itemnum);
    static QStringList Names();
    static QString Number(const QString &itemname);
    static QString Description(int itemnum);
    static int Power(int itemnum);
};


class TypeInfo
{
private:
    static QStringList m_Names;
    static QList<QPixmap> m_Pixmaps;
    static QString m_Directory;

    static void loadNames();
    static void loadPixmaps();
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int typenum);
    static QPixmap Picture(int typenum);
    static int NumberOfTypes();
};

#endif // POKEMONINFO_H
