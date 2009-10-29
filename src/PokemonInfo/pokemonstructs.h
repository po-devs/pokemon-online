#ifndef POKEMONSTRUCTS_H
#define POKEMONSTRUCTS_H

#include <QtGui>
#include <QDataStream>

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
protected:
    PokeBaseStats m_stats;
    QList<int> m_moves;
    QList<int> m_abilities;
    int m_types[2];
    int m_num;
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

    int num() const;
    void setNum(int new_num);

    const QList<int>& moves() const;

    /* loads using num() */
    void load();
};

/* Data that is unique to a pokémon */
class PokePersonal
{
protected:
    QString m_nickname;
    int m_num;
    int m_item;
    int m_ability;
    int m_nature;
    int m_gender;
    bool m_shininess;
    quint8 m_happiness;
    int m_level;

    int m_moves[4];

    quint8 m_DVs[6];
    quint8 m_EVs[6];

    /* checks if the sum of the EVs isn't too high and reduces EVs in all stats but *stat* in order to keep that true */
    void controlEVs(int stat);
public:
    PokePersonal();

    /* -1 if the nature is hindering, 0 if neutral and 1 if it boosts that stat */
    int natureBoost(int stat) const;

    QString nickname() const;
    int num() const;
    int item() const;
    int ability() const;
    int nature() const;
    int gender() const;
    bool shiny() const;
    quint8 happiness() const;
    int level() const;
    int move(int moveSlot) const;
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

    int num() const;
    void setNum(int num);

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

    void loadFromFile(const QString &path);
    void saveToFile(const QString &path);
};

/* Dialog for loading/saving team */
/* The third argument, if non null, gives the path the user chose */
bool saveTTeamDialog(const TrainerTeam &team, const QString &defaultPath = QObject::tr("Team/trainer.tp"), QString *chosenPath=0);
bool loadTTeamDialog(TrainerTeam &team, const QString &defaultPath = QObject::tr("Team/"), QString *chosenPath=0);

QDataStream & operator << (QDataStream & out,const Team & team);
QDataStream & operator << (QDataStream & out,const PokeTeam & Pokemon);
QDataStream & operator << (QDataStream & out,const TrainerTeam & trainerTeam);

QDataStream & operator >>(QDataStream & in,Team & team);
QDataStream & operator >>(QDataStream & in,PokeTeam & Pokemon);
QDataStream & operator >>(QDataStream & in,TrainerTeam & trainerTeam);

#endif // POKEMONSTRUCTS_H
