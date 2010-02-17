#ifndef POKEMONSTRUCTS_H
#define POKEMONSTRUCTS_H

#include <QtGui>
#include <QDataStream>
#include "../Utilities/functions.h"

namespace Pokemon
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
}


namespace Move
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

    enum Target
    {
	None = -1,
	User,
	ChosenTarget,
	RandomTarget,
	Opponents,
	All,
	AllButSelf
    };
}

namespace Type
{
    enum Effectiveness
    {
	Ineffective = 0,
	NotEffective = 1,
	Effective = 2,
	SuperEffective = 4
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
}

enum Stat
{
    Hp = 0,
    Attack,
    Defense,
    Speed,
    SpAttack,
    SpDefense = 5
};

class PokeBaseStats
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

    quint8 baseStat(int stat) const;
    void setBaseStat(int stat, quint8 base);
};

/* Data that every pokémon of the same specy share. */
class PokeGeneral
{
    PROPERTY(quint16, num);
protected:
    PokeBaseStats m_stats;
    QSet<int> m_moves;
    QList<int> m_abilities;
    int m_types[2];
    int m_genderAvail;

    void loadBaseStats();
    void loadMoves();
    void loadTypes();
    void loadAbilities();
    void loadGenderAvail();
public:
    PokeGeneral();

    void setBaseStats(const PokeBaseStats &stats);
    const PokeBaseStats & baseStats() const;

    const QList<int>& abilities() const;
    int genderAvail() const;

    const QSet<int>& moves() const;

    /* loads using num() */
    void load();
};

/* Data that is unique to a pokémon */
class PokePersonal
{
    PROPERTY(QString, nickname);
    PROPERTY(quint16, num);
    PROPERTY(quint16, item);
    PROPERTY(quint16, ability);
    PROPERTY(quint8, nature);
    PROPERTY(quint8, gender);
    PROPERTY(bool, shiny);
    PROPERTY(quint8, happiness);
    PROPERTY(quint8, level);
protected:
    int m_moves[4];

    quint8 m_DVs[6];
    quint8 m_EVs[6];

    /* checks if the sum of the EVs isn't too high and reduces EVs in all stats but *stat* in order to keep that true */
    void controlEVs(int stat);
public:
    PokePersonal();

    /* -1 if the nature is hindering, 0 if neutral and 1 if it boosts that stat */
    int natureBoost(int stat) const;
    int move(int moveSlot) const;
    /* resets everything to default values */
    void reset();

    void setMove(int moveNum, int moveSlot, bool check=true);
    int addMove(int moveNum);

    bool hasMove(int moveNum);

    quint8 DV(int stat) const;
    quint8 hpDV() const;
    quint8 attackDV() const;
    quint8 defenseDV() const;
    quint8 speedDV() const;
    quint8 spAttackDV() const;
    quint8 spDefenseDV() const;

    void setDV(int stat, quint8 DV);
    void setHpDV(quint8);
    void setAttackDV(quint8);
    void setDefenseDV(quint8);
    void setSpeedDV(quint8);
    void setSpAttackDV(quint8);
    void setSpDefenseDV(quint8);

    quint8 EV(int stat) const;
    quint8 hpEV() const;
    quint8 attackEV() const;
    quint8 defenseEV() const;
    quint8 speedEV() const;
    quint8 spAttackEV() const;
    quint8 spDefenseEV() const;
    int EVSum() const;

    void setEV(int stat, quint8 EV);
    void setHpEV(quint8);
    void setAttackEV(quint8);
    void setDefenseEV(quint8);
    void setSpeedEV(quint8);
    void setSpAttackEV(quint8);
    void setSpDefenseEV(quint8);
};

#ifdef CLIENT_SIDE

/* Contains / loads the graphics of a pokemon */
class PokeGraphics
{
protected:
    /* This is the current implementation, but an implemenation where more than one
       image is stored can do to */
    QPixmap m_picture;
    QIcon m_icon;
    int m_num;
    int m_storedgender;
    bool m_storedshininess;
    bool m_uptodate;

    void setUpToDate(bool uptodate);
    bool upToDate() const;
public:
    PokeGraphics();
    QPixmap picture(); /* just gives the already loaded picture */
    QPixmap picture(int gender, bool shiny); /* loads a new picture if necessary, anyway gives the right picture */
    QIcon icon();
    QIcon icon(int index);

    void setNum(int num);
    int num() const;
    void load(int gender, bool shiny);
    void loadIcon(int index);
};

class PokeTeam : virtual public PokeGeneral, virtual public PokePersonal, virtual public PokeGraphics
{
protected:
    /* Calculates actual stats based on the pokemon's characteristics */
    int calc_stat(quint8 basestat, int level, quint8 ev, quint8 dv) const;
    int calc_stat_F(int stat) const;
public:
    PokeTeam();

    quint16 num() const;
    void setNum(quint16 num);

    int stat(int statno) const;
    int hp() const;
    int attack() const;
    int defense() const;
    int speed() const;
    int spAttack() const;
    int spDefense() const;

    /* load various data from the pokenum */
    void load();
    /* display automatically the right picture */
    QPixmap picture();
    QIcon icon();
};

class Team
{
protected:
    PokeTeam m_pokes[6];

public:
    Team();

    const PokeTeam & poke(int index) const;
    PokeTeam & poke(int index);
};

class TrainerTeam
{
protected:
    Team m_team;
    QString m_trainerNick;
    QString m_trainerInfo;
    QString m_trainerWin;
    QString m_trainerLose;

public:
    TrainerTeam();

    const Team & team() const;
    Team & team();

    void setTrainerInfo(const QString &newinfo);
    void setTrainerWin(const QString &newwin);
    void setTrainerLose(const QString &newlose);
    void setTrainerNick(const QString &newnick);

    QString trainerInfo() const;
    QString trainerWin() const;
    QString trainerLose() const;
    QString trainerNick() const;

    bool loadFromFile(const QString &path);
    bool saveToFile(const QString &path) const;
};


/* Dialog for loading/saving team */
/* The third argument, if non null, gives the path the user chose */
bool saveTTeamDialog(const TrainerTeam &team, const QString &defaultPath = QObject::tr("Team/trainer.tp"), QString *chosenPath=0);
bool loadTTeamDialog(TrainerTeam &team, const QString &defaultPath = QObject::tr("Team/"), QString *chosenPath=0);


QDataStream & operator << (QDataStream & out,const Team & team);
QDataStream & operator << (QDataStream & out,const TrainerTeam & trainerTeam);

QDataStream & operator >> (QDataStream & in,Team & team);
QDataStream & operator >> (QDataStream & in,PokeTeam & Pokemon);
QDataStream & operator >> (QDataStream & in,TrainerTeam & trainerTeam);
#endif

QDataStream & operator << (QDataStream & out,const PokePersonal & Pokemon);
QDataStream & operator >> (QDataStream & in,PokePersonal & Pokemon);
#endif // POKEMONSTRUCTS_H
