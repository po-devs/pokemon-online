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

struct PokemonMoves
{
    //QSet<int> moves;
    /* All moves except egg & special */
    QSet<int> regularMoves[2];
    QSet<int> TMMoves;
    QSet<int> preEvoMoves[2];
    QSet<int> levelMoves[2];
    QSet<int> eggMoves[2];
    QSet<int> specialMoves[2];
    QSet<int> tutorMoves[2];
};

class PokemonInfo
{
private:
    static QTSList<QString> m_Names;
    static QTSList<QString> m_Weights;
    static QString m_Directory;
    static QTSList<int> m_Type1;
    static QTSList<int> m_Type2;
    static QTSList<int> m_Genders;
    static QTSList<int> m_Ability1;
    static QTSList<int> m_Ability2;
    static QTSList<PokeBaseStats> m_BaseStats;
    static QTSList<int> m_LevelBalance;
    /* That is NOT multi-threaded! */
    static QHash<int,QList<int> > m_AlternateFormes;
    /* First and last aesthetic forme */
    static QHash<int, QPair<int, int> > m_AestheticFormes;
    static QHash<int, bool> m_AestheticFormesHidden;
    static QHash<int, QString> m_AestheticFormesDescs;
    static QList<PokemonMoves> m_Moves;
    static int m_trueNumberOfPokes;

    static void loadNames();
    static void loadFormes();
    static void loadBaseStats();
    static void loadMoves();
    static QSet<int> getMoves(const QString &filename, int Pokenum);
    static QString path(const QString &filename);
    static int calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static int TrueCount(); // pokes without counting forms
    static int NumberOfPokemons();
    static QString Name(int pokenum);
    static int Number(const QString &pokename);
    static int LevelBalance(int pokenum);
    static QString WeightS(int pokenum);
    static float Weight(int pokenum);
    static int Gender(int pokenum);
    static int BaseGender(int pokenum);
    static QByteArray Cry(int pokenum);
    static int Type1(int pokenum);
    static int Type2(int pokenum);
    static QPixmap Picture(int pokenum, int forme = 0, int gender = Pokemon::Male, bool shiney = false, bool backimage = false);
    static QPixmap Sub(bool back = false);
    static QPixmap Icon(int index);
    static QSet<int> Moves(int pokenum);
    static QSet<int> EggMoves(int pokenum, int gen = 4);
    static QSet<int> LevelMoves(int pokenum, int gen = 4);
    static QSet<int> TutorMoves(int pokenum, int gen = 4);
    static QSet<int> TMMoves(int pokenum);
    static QSet<int> PreEvoMoves(int pokenum, int gen = 4);
    static QSet<int> SpecialMoves(int pokenum, int gen = 4);
    static QSet<int> RegularMoves(int pokenum, int gen = 4);
    /* Aesthetic formes are formes that are just a small variation of
       a poke and not a new poke. (Shaymin-S is a new poke compared to Shaymin imo).

       Some are chosable, like Shellos or Unown formes, some not, like Castform formes,
       as Castform only changes in battle.
       */
    static bool HasAestheticFormes(int pokenum);
    static int NumberOfAFormes(int pokenum);
    static bool AFormesShown(int pokenum);
    static int AestheticFormeId(int pokenum);
    static QString AestheticDesc(int pokenum, int forme);
    /* Standard formes: Rotom, Giratina, Deoxys, .. */
    static bool IsForme(int pokenum);
    static int OriginalForm(int pokenum);
    static bool HasFormes(int pokenum);
    static QList<int> Formes(int pokenum);
    static PokeBaseStats BaseStats(int pokenum);
    static bool Exist(int pokenum);
    static QList<int> Abilities(int pokenum);
    static int Stat(int poke, int stat, int level, quint8 dv, quint8 ev);
    static int FullStat(int poke, int nature, int stat, int level, quint8 dv, quint8 ev);
    static QString Desc(int poke, int cartridge);
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
    static QTSList<QString> m_Descriptions;
    static QTSList<QString> m_Details;
    static QHash<QString, int> m_LowerCaseMoves;

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
    static void loadDescriptions();
    static void loadDetails();
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
    static QString DetailedDescription(int movenum);
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
    static QTSList<int> m_BerryPowers;
    static QTSList<int> m_BerryTypes;

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
    static int PlateType(int itemnum);
    static QTSList<QString> SortedNames();
    static QList<Effect> Effects(int item);
    static QString Message(int item, int part);
    static int Number(const QString &itemname);
    /* returns the number corresponding to the name, but with the sortedNames as a ref */
    static int SortedNumber(const QString &itemname);
    static QString Description(int itemnum);
    static int Power(int itemnum);
    static int BerryPower(int itemnum);
    static int BerryType(int itemnum);
#ifdef CLIENT_SIDE
    static QPixmap Icon(int itemnum);
#endif
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
#ifdef CLIENT_SIDE
    static QList<QPixmap> m_Pics;
#endif

    static void loadNames();
    static void loadColors();
    static void loadEff();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int typenum);
    static int Number(const QString &type);
    static QColor Color(int typenum);
    static int Eff(int type_attack, int type_defend); /* Returns how effective it is: 4 = super, 2 = normal, 1 = not much, 0 = ineffective */
    static int NumberOfTypes();
    static int TypeForWeather(int weather);
#ifdef CLIENT_SIDE
    static QPixmap Picture(int type);
#endif
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
    static int Number(const QString &pokename);
    /* Finds nature of two stats, first parameter is the stat raised, second it the stat lowered*/
    static int NatureOf(int statUp, int statDown);
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
    static int Number(const QString &ab);
    static QString Message(int ab, int part);
    static int NumberOfAbilities();
    static QString Desc(int abnum);
    static QString EffectDesc(int abnum);
};

class GenderInfo
{
private:
    static QTSList<QString> m_Names;
    static QString m_Directory;
    static QTSList<QPixmap> m_Pictures;
    static QTSList<QPixmap> m_BattlePictures;

    static void loadNames();
    static void loadPixmaps();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    /* Self-explainable functions */
    static QString Name(int gender);
    static int NumberOfGenders();
    static QPixmap Picture(int gender, bool battle = false);
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
    static QHash<int, QPixmap> m_battleIcons;

    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="./");

    static QString Stat(int stat);
    static QString Status(int status);
    static QString ShortStatus(int status);
    static QColor StatusColor(int status);
    static QPixmap Icon(int status);
    static QPixmap BattleIcon(int status);
};

#endif // POKEMONINFO_H
