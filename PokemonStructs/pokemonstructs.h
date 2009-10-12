#ifndef POKEMONSTRUCTS_H
#define POKEMONSTRUCTS_H

#include <QtCore>
#include <QtGui>
#include "PokemonStructs_global.h"

struct Pokemon
{
    enum Gender
    {
	Neutral,
	Male,
	Female
    };

    /* For simplicity issues we keep the same order as in Gender. You can assume it'll stay
   that way for next versions.

   That allows you to do PokemonInfo::Picture(pokenum, (Gender)GenderAvail(pokenum)) */

    enum GenderAvail
    {
	NeutralAvail,
	MaleAvail,
	FemaleAvail,
	MaleAndFemaleAvail
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

    enum Nature
    {
	Hardy = 0,
	Lonely,
	Brave,
	Adamant,
	Naughty,
	Bold,
	Docile,
	Relaxed,
	Impish,
	Lax,
	Timid,
	Hasty,
	Serious,
	Jolly,
	Naive,
	Modest,
	Mild,
	Quiet,
	Bashful,
	Rash,
	Calm,
	Gentle,
	Sassy,
	Careful,
	Quirky = 24
    };
};

class POKEMONSTRUCTSSHARED_EXPORT PokeBaseStats
{
private:
    quint8 m_BaseStats[6];
public:
    PokeBaseStats(quint8 base_hp=80, quint8 base_att=80, quint8 base_def = 80, quint8 base_spd = 80, quint8 base_spAtt = 80, quint8 base_spDef = 80);

    quint8 baseHp() const;
    quint8 baseAttack() const;
    quint8 baseDefense() const;
    quint8 baseSpeed() const;
    quint8 baseSpAttack() const;
    quint8 baseSpDefense() const;

    void setBaseHp(quint8);
    void setBaseAttack(quint8);
    void setBaseDefense(quint8);
    void setBaseSpeed(quint8);
    void setBaseSpAttack(quint8);
    void setBaseSpDefense(quint8);
};

/* Data that every pokémon of the same specy share. */
class POKEMONSTRUCTSSHARED_EXPORT PokeGeneral
{
protected:
    PokeBaseStats m_stats;
    QList<int> m_moves;
    QList<int> m_abilities;
    int m_types[2];
    int m_num;

    void loadBaseStats();
    void loadMoves();
    void loadTypes();
    void loadAbilities();
public:
    PokeGeneral();

    void setBaseStats(const PokeBaseStats &stats);
    const PokeBaseStats & baseStats() const;

    int num() const;
    void setNum(int new_num);

    const QList<int>& moves() const;

    /* loads using num() */
    void load();
};

/* Data that is unique to a pokémon */
class POKEMONSTRUCTSSHARED_EXPORT PokePersonal
{
protected:
    QString m_nickname;
    int m_num;
    int m_item;
    int m_ability;
    int m_nature;
    int m_gender;
    bool m_shininess;
    int m_happiness;
    int m_level;

    int m_moves[4];

    quint8 m_DVs[6];
    quint8 m_EVs[6];
public:
    PokePersonal();

    QString nickname() const;
    int num() const;
    int item() const;
    int ability() const;
    int nature() const;
    int gender() const;
    int shininess() const;
    int happiness() const;
    int level() const;
    /* resets everything to default values */
    void reset();

    void setNickname(const QString &);
    void setNum(int num);
    void setItem(int item);
    void setAbility(int ability);
    void setNature(int nature);
    void setGender(int gender);
    void setShininess(bool shiny);
    void setHappiness(quint8 happiness);
    void setLevel(int level);
    void setMove(int moveNum, int moveSlot);

    quint8 hpDV() const;
    quint8 attackDV() const;
    quint8 defenseDV() const;
    quint8 speedDV() const;
    quint8 spAttackDV() const;
    quint8 spDefenseDV() const;

    void setHpDV(quint8);
    void setAttackDV(quint8);
    void setDefenseDV(quint8);
    void setSpeedDV(quint8);
    void setSpAttackDV(quint8);
    void setSpDefenseDV(quint8);

    quint8 hpEV() const;
    quint8 attackEV() const;
    quint8 defenseEV() const;
    quint8 speedEV() const;
    quint8 spAttackEV() const;
    quint8 spDefenseEV() const;

    void setHpEV(quint8);
    void setAttackEV(quint8);
    void setDefenseEV(quint8);
    void setSpeedEV(quint8);
    void setSpAttackEV(quint8);
    void setSpDefenseEV(quint8);
};

/* Contains / loads the graphics of a pokemon */
class POKEMONSTRUCTSSHARED_EXPORT PokeGraphics
{
protected:
    /* This is the current implementation, but an implemenation where more than one
       image is stored can do to */
    QPixmap m_picture;
    int m_num;
    int m_storedgender;
    bool m_storedshininess;
    bool m_uptodate;

    void setUpToDate(bool uptodate);
    bool upToDate();
public:
    PokeGraphics();
    QPixmap picture(); /* just gives the already loaded picture */
    QPixmap picture(int gender, bool shiny); /* loads a new picture if necessary, anyway gives the right picture */

    void setNum(int num);
    int num() const;
    void load(int gender, bool shiny);
};

class POKEMONSTRUCTSSHARED_EXPORT PokeTeam : virtual public PokeGeneral, virtual public PokePersonal, virtual public PokeGraphics
{
public:
    PokeTeam();

    int num() const;
    void setNum(int num);

    /* load various data from the pokenum */
    void load();
};

class POKEMONSTRUCTSSHARED_EXPORT Team
{
protected:
    PokeTeam m_pokes[6];

public:
    Team();

    const PokeTeam & poke(int index) const;
    PokeTeam & poke(int index);
};

#endif // POKEMONSTRUCTS_H
