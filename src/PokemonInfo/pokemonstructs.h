#ifndef POKEMONSTRUCTS_H
#define POKEMONSTRUCTS_H

#include "pokemon.h"
#include "geninfo.h"
#include <QString>
#include <QSet>
#include <QIcon>
#include <QPixmap>
#include "../Utilities/functions.h"
#include "enums.h"

class QDomElement;
class QDomDocument;
class DataStream;

struct AbilityGroup {
    quint8 _ab[3];

    AbilityGroup() {
        _ab[0] = 0;
        _ab[1] = 0;
        _ab[2] = 0;
    }

    int ab(int num) const {
        return _ab[num];
    }

    bool contains(int num) const {
        return _ab[0] == num || _ab[1] == num || _ab[2] == num;
    }
};

class PokeBaseStats
{
private:
    quint8 m_BaseStats[6];
public:
    PokeBaseStats(quint8 base_hp=80, quint8 base_att=80, quint8 base_def = 80, quint8 base_spAtt = 80, quint8 base_spDef = 80, quint8 base_spd = 80);

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

/* Data that every pokemon of the same specy share. */
class PokeGeneral
{
    PROPERTY(Pokemon::uniqueId, num);
    PROPERTY(Pokemon::gen, gen);
public:
    PokeGeneral();
    virtual ~PokeGeneral(){}

    const AbilityGroup &abilities() const;
    int genderAvail() const;
    int type1() const;
    int type2() const;

    const QSet<int>& moves() const;

    /* loads using num() */
    void load();
protected:
    QSet<int> m_moves;
    AbilityGroup m_abilities;
    int m_types[2];
    int m_genderAvail;

    void loadMoves();
    void loadTypes();
    void loadAbilities();
    void loadGenderAvail();
};

/* Data that is unique to a pokémon */
class PokePersonal
{
    PROPERTY(QString, nickname);
    PROPERTY(Pokemon::uniqueId, num);
    PROPERTY(quint16, item);
    PROPERTY(quint16, ability);
    PROPERTY(quint8, nature);
    PROPERTY(quint8, gender);
    PROPERTY(bool, shiny);
    PROPERTY(quint8, happiness);
    PROPERTY(quint8, level);
    PROPERTY(Pokemon::gen, gen);
protected:
    quint16 m_moves[4];

    quint8 m_DVs[6];
    quint8 m_EVs[6];

    /* checks if the sum of the EVs isn't too high and reduces EVs in all stats but *stat* in order to keep that true */
    void controlEVs(int stat);
public:
    PokePersonal();

    /* -1 if the nature is hindering, 0 if neutral and 1 if it boosts that stat */
    int natureBoost(int stat) const;
    quint16 move(int moveSlot) const;
    /* resets everything to default values */
    void reset();
    /* Removes / Reset things if they are wrong */
    void runCheck();

    void setMove(int moveNum, int moveSlot, bool check=false) throw (QString);
    int addMove(int moveNum, bool check = false) throw (QString);
    void removeMove(int movenum);

    bool hasMove(int moveNum);

    quint8 DV(int stat) const;
    void setDV(int stat, quint8 DV);
    void controlHPDV();
    void controlShininess();
    void controlGender();

    quint8 EV(int stat) const;
    int EVSum() const;

    void setEV(int stat, quint8 EV);

    enum Flags {
        hasGen, hasNickname, hasPokeball, hasHappiness, hasPPups, hasIVs,
        isShiny=0
    };
};

/* Contains / loads the graphics of a pokemon */
class PokeGraphics
{
public:
    PokeGraphics();
    QPixmap picture(); /* just gives the already loaded picture */
    QPixmap picture(int gender, bool shiny); /* loads a new picture if necessary, anyway gives the right picture */
    QIcon icon();
    QIcon icon(const Pokemon::uniqueId &pokeid);


    void setNum(Pokemon::uniqueId num);
    void setGen(Pokemon::gen gen);
    Pokemon::uniqueId num() const;
    Pokemon::gen gen() const;

    void load(int gender, bool shiny);
    void loadIcon(const Pokemon::uniqueId &pokeid);
protected:
    /* This is the current implementation, but an implemenation where more than one
       image is stored can do to */
    QPixmap m_picture;
    QIcon m_icon;
    Pokemon::uniqueId m_num;
    int m_storedgender;
    Pokemon::gen m_gen;

    bool m_storedshininess;
    bool m_uptodate;

    void setUpToDate(bool uptodate);
    bool upToDate() const;
};

class PokeTeam : virtual public PokeGeneral, virtual public PokePersonal, virtual public PokeGraphics
{
public:
    PokeTeam();

    Pokemon::uniqueId num() const;
    void setNum(Pokemon::uniqueId num);
    void setGen(Pokemon::gen gen);
    Pokemon::gen gen() const;
    void runCheck();

    int stat(int statno) const;

    /* load various data from the pokenum */
    void load();
    /* loads but without changing what's valid already */
    void loadQuietly();
    /* display automatically the right picture */
    QPixmap picture();
    QIcon icon();

    void loadFromXml(const QDomElement &el, int version);
    QDomElement & toXml(QDomElement &dest) const;

    void sanityCheck();
};

class Team
{
    PROPERTY(QString, defaultTier);
protected:
    PokeTeam m_pokes[6];
    Pokemon::gen m_gen;

public:
    Team();
    Pokemon::gen gen() const {return m_gen;}
    void setGen(Pokemon::gen gen);

    const PokeTeam & poke(int index) const {return m_pokes[index];}
    PokeTeam & poke(int index) {return m_pokes[index];}

    bool loadFromFile(const QString &path);
    void toXml(QDomDocument &doc) const;
    QString toXml() const;
    bool saveToFile(const QString &path) const;
    bool importFromTxt(const QString &path);
    QString exportToTxt() const;

    QString name() const;
    QString folder() const;
    QString path() const {return m_path;}
    void setName(const QString &name);
    void setFolder(const QString &folder);

    /* Runs some check to validate a team better, and edits invalid values */
    void sanityCheck();
private:
    mutable QString m_path;
};

/* Dialog for loading/saving team */
void saveTTeamDialog(const Team &team, QObject *receiver=NULL, const char *slot=NULL);
void loadTTeamDialog(Team &team, QObject *receiver=NULL, const char *slot=NULL);

DataStream & operator << (DataStream & out,const Team & team);

DataStream & operator << (DataStream & out,const PokePersonal & Pokemon);
DataStream & operator >> (DataStream & in,PokePersonal & Pokemon);

#endif // POKEMONSTRUCTS_H
