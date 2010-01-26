#ifndef POKEMONINFO_H
#define POKEMONINFO_H

#include "pokemonstructs.h"
#include <QtCore>

class PokeBaseStats;
class QPixmap;

#ifdef MULTI_THREADED_ACCESS
#include "../Utilities/thread_safe_containers.h"
#else
#define QTSList QList
#endif

/* A class that should be used as a singleton and provide every ressource needed on pokemons */
class PokemonInfo
{
private:
    static QTSList<QString> m_Names;
    static QTSList<float> m_Weights;
    static QString m_Directory;
    static QTSList<int> m_Type1;
    static QTSList<int> m_Type2;
    static QTSList<int> m_Genders;
    static QTSList<int> m_Ability1;
    static QTSList<int> m_Ability2;
    static QTSList<PokeBaseStats> m_BaseStats;

    static void loadNames();
    static void loadBaseStats();
    static QList<int> getMoves(const QString &filename, int Pokenum);
    static QString path(const QString &filename);
    static int calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static int NumberOfPokemons();
    static QString Name(int pokenum);
    static int Number(const QString &pokename);
    static float Weight(int pokenum);
    static int Gender(int pokenum);
    static int Type1(int pokenum);
    static int Type2(int pokenum);
    static QPixmap Picture(int pokenum, int gender = Pokemon::Male, bool shiney = false, bool backimage = false);
    static QIcon Icon(int index);
    static QList<int> Moves(int pokenum);
    static QList<int> EggMoves(int pokenum);
    static QList<int> LevelMoves(int pokenum);
    static QList<int> TutorMoves(int pokenum);
    static QList<int> TMMoves(int pokenum);
    static QList<int> SpecialMoves(int pokenum);
    static PokeBaseStats BaseStats(int pokenum);
    static bool Exist(int pokenum);
    static QList<int> Abilities(int pokenum);
    static int Stat(int stat, quint8 base_stat, int level, quint8 dv, quint8 ev);
    static int FullStat(int nature, int stat, quint8 base_stat, int level, quint8 dv, quint8 ev);
};

class MoveInfo
{
private:
    static QTSList<QString> m_Names;
    static QTSList<QString> m_PowerS;
    static QTSList<QString> m_AccS;
    static QTSList<QString> m_Effects;
    static QTSList<QString> m_SpecialEffects;
    static QTSList<QStringList> m_MoveMessages;
    static QTSList<char> m_Type;
    static QTSList<char> m_PP;
    static QTSList<char> m_Category;
    static QTSList<char> m_Critical;
    static QTSList<char> m_EffectRate;
    static QTSList<bool> m_Physical;
    static QTSList<bool> m_KingRock;
    static QTSList<char> m_Speeds;
    static QTSList<int> m_Flinch;
    static QTSList<int> m_Recoil;
    static QTSList<int> m_Targets;
    static QTSList<QPair<char, char> > m_Repeat;
    static QString m_Directory;

    static void loadNames();
    static void loadPPs();
    static void loadTypes();
    static void loadCategorys();
    static void loadPowers();
    static void loadAccs();
    static void loadEffects();
    static void loadCritics();
    static void loadEffectRates();
    static void loadPhysics();
    static void loadKingRocks();
    static void loadRepeats();
    static void loadSpeeds();
    static void loadTargets();
    static void loadFlinchs();
    static void loadRecoil();
    static void loadSpecialEffects();
    static void loadMoveMessages();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int movenum);
    static int Type(int movenum);
    static int Category(int movenum);
    static int Number(const QString &movename);
    static int NumberOfMoves();
    static int FlinchRate(int movenum);
    static int Recoil(int movenum);
    static QString Description(int movenum);
    static int Power(int movenum);
    /* gives the power of a move in the form of a string */
    static QString PowerS(int movenum);
    static int PP(int movenum);
    static int Acc(int movenum);
    /* gives the accuracy of a move in the form of a string */
    static QString AccS(int movenum);
    /* the status mod of a move*/
    static QString Effect(int movenum);
    static QString SpecialEffect(int movenum);
    static int CriticalRaise(int movenum);
    static int RepeatMin(int movenum);
    static int RepeatMax(int movenum);
    static int SpeedPriority(int movenum);
    static bool PhysicalContact(int movenum);
    static bool KingRock(int movenum);
    static bool Exist(int movenum);
    static bool isOHKO(int movenum);
    static int EffectRate(int movenum);
    static int Target(int movenum);
    static QString MoveMessage(int moveeffect, int part);
};

class ItemInfo
{
public:
    struct Effect {
	int num;
	QString args;
	Effect(int i, const QString &q="") : num(i), args(q){}
    };
private:
    static QTSList<QString> m_BerryNames;
    static QTSList<QString> m_RegItemNames;
    static QHash<QString, int> m_BerryNamesH;
    static QHash<QString, int> m_ItemNamesH;
    static QTSList<QString> m_SortedNames;
    static QString m_Directory;
    static QTSList<QList<Effect> > m_RegEffects;
    static QTSList<QList<Effect> > m_BerryEffects;
    static QTSList<QStringList> m_RegMessages;
    static QTSList<QStringList> m_BerryMessages;
    static QTSList<int> m_Powers;

    static void loadNames();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static int NumberOfItems();
    static QString Name(int itemnum);
    static bool Exist(int itemnum);
    static bool isBerry(int itemnum);
    static bool isPlate(int itemnum);
#ifndef MULTI_THREADED_ACCESS
    static QStringList SortedNames();
#endif
    static QList<Effect> Effects(int item);
    static QString Message(int item, int part);
    static int Number(const QString &itemname);
    /* returns the number corresponding to the name, but with the sortedNames as a ref */
    static int SortedNumber(const QString &itemname);
    static QString Description(int itemnum);
    static int Power(int itemnum);
};

class TypeInfo
{
private:
    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    static QTSList<QString> m_Names;
    static QString m_Directory;
    static QTSList<QColor> m_Colors;
    static QTSList<int> m_TypeVsType;

    static void loadNames();
    static void loadColors();
    static void loadEff();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int typenum);
    static QColor Color(int typenum);
    static int Eff(int type_attack, int type_defend); /* Returns how effective it is: 4 = super, 2 = normal, 1 = not much, 0 = ineffective */
    static int NumberOfTypes();
    static int TypeForWeather(int weather);
};

class NatureInfo
{
private:
    static QTSList<QString> m_Names;
    static QString m_Directory;
    static void loadNames();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int naturenum);
    static int NumberOfNatures();

    /* -1 if the nature is hindering, 0 if neutral and 1 if it boosts that stat */
    static int Boost(int nature, int stat);
};

class CategoryInfo
{
private:
    static QTSList<QString> m_Names;
    static QString m_Directory;
    static QTSList<QColor> m_Colors;

    static void loadNames();
    static void loadColors();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int catnum);
    static QColor Color(int catnum);
    static int NumberOfCategories();
};

class AbilityInfo
{
public:
    struct Effect {
        int num;
        int arg;
        Effect(int i, int q=0) : num(i), arg(q){}
    };
private:
    static QTSList<QString> m_Names;
    static QString m_Directory;
    static QTSList<Effect> m_Effects;
    static QTSList<QStringList> m_Messages;

    static void loadNames();
    static void loadEffects();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int abnum);
    static Effect Effects(int abnum);
    static QString Message(int ab, int part);
    static int NumberOfAbilities();
};

class GenderInfo
{
private:
    static QTSList<QString> m_Names;
    static QString m_Directory;
    static QTSList<QPixmap> m_Pictures;

    static void loadNames();
    static void loadPixmaps();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int gender);
    static int NumberOfGenders();
    static QPixmap Picture(int gender);
    static int Default(int genderAvail);
    static bool Possible(int gender, int genderAvail);
};


class HiddenPowerInfo
{
private:
    static QString m_Directory;

    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* The type of the hidden power depending on the dvs */
    static int Type(quint8 hpdv, quint8 attdv, quint8 defdv, quint8 spddv, quint8 sattdv, quint8 sdefdv);
    /* The power of the hidden power depending on the dvs */
    static int Power(quint8 hpdv, quint8 attdv, quint8 defdv, quint8 spddv, quint8 sattdv, quint8 sdefdv);
    /* the different set of dvs (which are chosen within 30-31) that give an hidden power of that type */
    static QList<QStringList> PossibilitiesForType(int type);
};

class StatInfo
{
private:
    static QString m_Directory;
    static QTSList<QString> m_stats;
    static QTSList<QString> m_status;
    static QHash<int, QPixmap> m_statusIcons;

    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    static QString Stat(int stat);
    static QString Status(int status);
    static QPixmap Icon(int status);
};

#endif // POKEMONINFO_H
