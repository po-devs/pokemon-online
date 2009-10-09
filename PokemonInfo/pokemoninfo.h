#ifndef POKEMONINFO_H
#define POKEMONINFO_H

#include "PokemonInfo_global.h"
#include "../pokemonstructs.h"
#include <QtGui>

/* A class that should be used as a singleton and provide every ressource needed on pokemons */
class POKEMONINFOSHARED_EXPORT PokemonInfo
{
private:
    static const int m_NumOfPokemons = 493;

    QString m_Names[m_NumOfPokemons+1];
    QString m_Directory;

    void loadNames();
    QLinkedList<int> getMoves(const QString &filename, int Pokenum) const;
public:

    /* directory where all the data is */
    PokemonInfo(const QString &dir="./");

    /* Self-explainable functions */
    int NumberOfPokemons() const;
    QString Name(int pokenum) const;
    int Number(const QString &pokename) const;
    Pokemon::GenderAvail Gender(int pokenum) const;
    QPixmap Picture(int pokenum, Pokemon::Gender gender = Pokemon::Male, bool shiney = false) const;
    QLinkedList<int> Moves(int pokenum) const;
    QLinkedList<int> EggMoves(int pokenum) const;
    QLinkedList<int> LevelMoves(int pokenum) const;
    QLinkedList<int> TutorMoves(int pokenum) const;
    QLinkedList<int> TMMoves(int pokenum) const;
    QLinkedList<int> SpecialMoves(int pokenum) const;
    PokeBaseStats BaseStats(int pokenum);
    QList<int> Abilities(int pokenum);
};

struct POKEMONINFOSHARED_EXPORT Move
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

class POKEMONINFOSHARED_EXPORT MoveInfo
{
private:
    static const int m_NumOfMoves = 467;

    QString m_Names[m_NumOfMoves+1];
    QString m_Directory;
public:
    /* directory where all the data is */
    MoveInfo(const QString &dir="./");

    /* Self-explainable functions */
    QString Name(int movenum);
    QString Number(const QString &movename);
    QString Description(int movenum);
    int Power(int movenum);
};

#endif // POKEMONINFO_H
