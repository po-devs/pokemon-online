#ifndef POKEMONINFO_H
#define POKEMONINFO_H

#include "pokemonstructs.h"
#include <QtCore>

class PokeBaseStats;
class QPixmap;

/* A class that should be used as a singleton and provide every ressource needed on pokemons */

struct PokemonMoves
{
    //QSet<int> moves;
    /* All moves except egg & special */
    QSet<int> regularMoves[3];
    QSet<int> TMMoves[3];
    QSet<int> preEvoMoves[3];
    QSet<int> levelMoves[3];
    QSet<int> eggMoves[3];
    QSet<int> specialMoves[3];
    QSet<int> tutorMoves[3];
    QSet<int> genMoves[3];
};


class PokemonInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/pokes/");

    /* Self-explainable functions */
    static int TrueCount(int gen=5); // pokes without counting forms
    static int NumberOfPokemons(); // base + all forms.
    static int NumberOfVisiblePokes(); // base + visible forms.
    static QString Name(const Pokemon::uniqueId &pokeid);
    static Pokemon::uniqueId Number(const QString &pokename);
    static int LevelBalance(const Pokemon::uniqueId &pokeid);
    static QString WeightS(const Pokemon::uniqueId &pokeid);
    static QString Classification(const Pokemon::uniqueId &pokeid);
    static int Weight(const Pokemon::uniqueId &pokeid);
    static int Gender(const Pokemon::uniqueId &pokeid);
    static int BaseGender(const Pokemon::uniqueId &pokeid);
    static QByteArray Cry(const Pokemon::uniqueId &pokeid);
    static int Type1(const Pokemon::uniqueId &pokeid, int gen = 5);
    static int Type2(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QPixmap Picture(const Pokemon::uniqueId &pokeid, int gen = 5, int gender = Pokemon::Male, bool shiney = false, bool backimage = false);
    static QPixmap Sub(int gen=5, bool back = false);
    static QPixmap Icon(const Pokemon::uniqueId &pokeid);
    static QSet<int> Moves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> EggMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> LevelMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> TutorMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> TMMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> PreEvoMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> SpecialMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QSet<int> RegularMoves(const Pokemon::uniqueId &pokeid, int gen = 5);
    static QList<Pokemon::uniqueId> AllIds();
    // Base form do NOT count.
    static quint16 NumberOfAFormes(const Pokemon::uniqueId &pokeid);
    static bool AFormesShown(const Pokemon::uniqueId &pokeid);
    /* Standard formes: Rotom, Giratina, Deoxys, .. */
    static bool IsForme(const Pokemon::uniqueId &pokeid);
    static bool IsAesthetic(Pokemon::uniqueId id);
    static Pokemon::uniqueId NonAestheticForme(Pokemon::uniqueId id);
    static Pokemon::uniqueId OriginalForme(const Pokemon::uniqueId &pokeid);
    static bool HasFormes(const Pokemon::uniqueId &pokeid);
    // Will NOT return base form. Should it?
    static QList<Pokemon::uniqueId> Formes(const Pokemon::uniqueId &pokeid);
    static QList<int> Evos(int pokenum);

    // Will always return base form (subnum 0).
    static Pokemon::uniqueId OriginalEvo(const Pokemon::uniqueId &pokeid);
    static int PreEvo(int pokenum);
    static bool IsInEvoChain(const Pokemon::uniqueId &pokeid);
    static PokeBaseStats BaseStats(const Pokemon::uniqueId &pokeid);
    static bool Exists(const Pokemon::uniqueId &pokeid, int gen=5);
    static AbilityGroup Abilities(const Pokemon::uniqueId &pokeid, int gen=5);
    static int Stat(const Pokemon::uniqueId &pokeid, int stat, int level, quint8 dv, quint8 ev);
    static int FullStat(const Pokemon::uniqueId &pokeid, int nature, int stat, int level, quint8 dv, quint8 ev);
    static QString Desc(const Pokemon::uniqueId &pokeid, int cartridge);
    static QString Height(const Pokemon::uniqueId &pokeid);
    // Will NOT return Missingno.
    static Pokemon::uniqueId getRandomPokemon();
private:
    // m_Names is a base.
    // It is assumed that anything that is not there do not exist at all.
    // Is a map because we need it to be sorted.
    static QMap<Pokemon::uniqueId, QString> m_Names;
    static QHash<Pokemon::uniqueId, QString> m_Weights;
    static QHash<int, QHash<quint16, QString> > m_Desc;
    static QHash<int, QString> m_Classification;
    static QHash<Pokemon::uniqueId, QString> m_Height;
    static QString m_Directory;
    static QHash<Pokemon::uniqueId, int> m_Type1[3];
    static QHash<Pokemon::uniqueId, int> m_Type2[3];
    static QHash<Pokemon::uniqueId, int> m_Genders;
    static QHash<Pokemon::uniqueId, int> m_Abilities[3][3];
    static QHash<Pokemon::uniqueId, PokeBaseStats> m_BaseStats;
    static QHash<Pokemon::uniqueId, int> m_LevelBalance;

    static QHash<int, QList<int> > m_Evolutions;
    static QHash<int, int> m_OriginalEvos;
    static QHash<int, int> m_PreEvos;
    /* Tells if there is a real difference from the original forme.
       If not, tiers will consider the pokemon as its original forme, to avoid
       listing too many pokemons */
    static QSet<Pokemon::uniqueId> m_AestheticFormes;
    // A number of forms a pokemon has. 0 for most cases.
    // Keep it as QHash.
    // quint16 as only pokenum matters.
    static QHash<int, quint16> m_MaxForme;
    static QHash<Pokemon::uniqueId, PokemonMoves> m_Moves;
    // Holds 1-letter options.
    // Sample use: if(m_Options.value(pokeid).contains('H')) whatever();
    // Values for pokemons.txt:
    // 1 - always 1 HP.
    // H - hidden form(e).
    static QHash<Pokemon::uniqueId, QString> m_Options;

    static int m_trueNumberOfPokes;
    // To get random pokemon faster.
    static QList<Pokemon::uniqueId> m_VisiblePokesPlainList;

    static void loadNames();
    static void loadEvos();
    static void loadBaseStats();
    static void loadMoves();
    static void loadClassifications();
    static void loadHeights();
    static void loadDescriptions();
    // Call this after loading all data.
    static void makeDataConsistent();
    static QSet<int> getMoves(const QString &filename, int Pokenum);
    static QString path(const QString &filename);
    static int calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev);
};

class MoveInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/moves/");

    /* Self-explainable functions */
    static QString Name(int movenum);
    static int Type(int movenum, int gen);
    static int Category(int movenum, int gen);
    static int Classification(int movenum, int gen);
    static int Number(const QString &movename);
    static int NumberOfMoves();
    static int FlinchRate(int movenum, int gen);
    static int Recoil(int movenum, int gen);
    static QString Description(int movenum, int gen);
    static QString DetailedDescription(int movenum);
    static int Power(int movenum, int gen);
    /* gives the power of a move in the form of a string */
    static QString PowerS(int movenum, int gen);
    static int PP(int movenum, int gen);
    static int Acc(int movenum, int gen);
    /* gives the accuracy of a move in the form of a string */
    static QString AccS(int movenum, int gen);
    static int CriticalRaise(int movenum, int gen);
    static int RepeatMin(int movenum, int gen);
    static int RepeatMax(int movenum, int gen);
    static int SpeedPriority(int movenum, int gen);
    static int Flags(int movenum, int gen);
    static bool Exists(int movenum, int gen);
    static bool isOHKO(int movenum, int gen);
    static int EffectRate(int movenum, int gen);
    static quint32 StatAffected(int movenum, int gen);
    static quint32 BoostOfStat(int movenum, int gen);
    static quint32 RateOfStat(int movenum, int gen);
    static int Target(int movenum, int gen);
    static int Healing(int movenum, int gen);
    static int MinTurns(int movenum, int gen);
    static int MaxTurns(int movenum, int gen);
    static int Status(int movenum, int gen);
    static int StatusKind(int movenum, int gen);
    static QString MoveMessage(int moveeffect, int part);
    static QStringList MoveList();
    /* the status mod of a move*/
    //static QString Effect(int movenum, int gen);
    static QString SpecialEffect(int movenum);
private:
    static QList<QString> m_Names;
    static QHash<QString, int> m_LowerCaseMoves;
    static QList<QStringList> m_MoveMessages;
    static QList<QString> m_Details;
    static QList<QString> m_SpecialEffects;

    struct Gen {
        void load(const QString &path, int gen);
        QString path(const QString &fileName);

        int gen;
        QString dir;

        QVector<char> accuracy;
        QVector<char> category;
        QVector<char> causedEffect;
        QVector<char> critRate;
        QVector<char> damageClass;
        QStringList effect;
        QVector<char> effectChance;
        QVector<int> flags;
        QVector<char> flinchChance;
        QVector<char> healing;
        QVector<char> maxTurns;
        QVector<char> minTurns;
        QVector<char> minMaxHits;
        QVector<int> none0;
        QVector<int> none1;
        QVector<int> none2;
        QVector<unsigned char> power;
        QVector<char> pp;
        QVector<char> priority;
        QVector<char> range;
        QVector<char> recoil;
        QVector<char> status;
        QVector<char> type;
    };

    static QString m_Directory;
    static Gen gens[Version::NumberOfGens];
    static const Gen & gen(int gen) {
        return gens[gen-1];
    }

    static void loadNames();
    static void loadMoveMessages();
    static void loadDetails();
    static void loadSpecialEffects();

    static QString path(const QString &filename);
};

class ItemInfo
{
public:
    struct Effect {
	int num;
	QString args;
	Effect(int i, const QString &q="") : num(i), args(q){}
    };

    /* directory where all the data is */
    static void init(const QString &dir="db/items/");

    /* Self-explainable functions */
    static int NumberOfItems();
    static QString Name(int itemnum);
    static bool Exists(int itemnum, int gen=5);
    static bool isBerry(int itemnum);
    static bool isPlate(int itemnum);
    static bool isMail(int itemnum);
    static bool isUseful(int itemnum);
    static int PlateType(int itemnum);
    static QList<QString> SortedNames(int gen);
    static QList<QString> SortedUsefulNames(int gen);
    static QList<Effect> Effects(int item, int gen);
    static QString Message(int item, int part);
    static int Number(const QString &itemname);
    static QString Description(int itemnum);
    static int Power(int itemnum);
    static int BerryPower(int itemnum);
    static int BerryType(int itemnum);
    static QPixmap Icon(int itemnum);
private:
    static QList<QString> m_BerryNames;
    static QList<QString> m_RegItemNames;
    static QHash<QString, int> m_BerryNamesH;
    static QHash<QString, int> m_ItemNamesH;
    static QList<QString> m_SortedNames[3];
    static QList<QString> m_SortedUsefulNames[3];
    static QString m_Directory;
    static QList<QList<Effect> > m_RegEffects[3];
    static QList<QList<Effect> > m_BerryEffects;
    static QList<QStringList> m_RegMessages;
    static QList<QStringList> m_BerryMessages;
    static QList<int> m_Powers;
    static QList<int> m_BerryPowers;
    static QList<int> m_BerryTypes;
    static QList<int> m_UsefulItems;
    static QSet<int> m_GenItems[3];

    static void loadNames();
    static QString path(const QString &filename);
};

class TypeInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/types/");

    /* Self-explainable functions */
    static QString Name(int typenum);
    static int Number(const QString &type);
    static int Eff(int type_attack, int type_defend); /* Returns how effective it is: 4 = super, 2 = normal, 1 = not much, 0 = ineffective */
    static int NumberOfTypes();
    static int TypeForWeather(int weather);
    static int Category(int type);
    static void modifyTypeChart(int type_attack, int type_defend, int value);
private:
    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    static QList<QString> m_Names;
    static QString m_Directory;
    static QList<int> m_TypeVsType;
    static QList<int> m_Categories;

    static void loadNames();
    static void loadEff();
    static QString path(const QString &filename);
};

class NatureInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/nature/");

    /* Self-explainable functions */
    static QString Name(int naturenum);
    static int NumberOfNatures();
    static int Number(const QString &pokename);
    /* Finds nature of two stats, first parameter is the stat raised, second it the stat lowered*/
    static int NatureOf(int statUp, int statDown);
    /* -1 if the nature is hindering, 0 if neutral and 1 if it boosts that stat */
    static int Boost(int nature, int stat);
    static int StatBoosted(int nature);
    static int StatHindered(int nature);
    static int ConvertStat(int stat);
    static int ConvertToStat(int stat);
private:
    static QList<QString> m_Names;
    static QString m_Directory;
    static void loadNames();
    static QString path(const QString &filename);
};

class CategoryInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/categories/");

    /* Self-explainable functions */
    static QString Name(int catnum);
    static int NumberOfCategories();
private:
    static QList<QString> m_Names;
    static QString m_Directory;

    static void loadNames();
    static QString path(const QString &filename);
};

class AbilityInfo
{
public:
    struct Effect {
        int num;
        int arg;
        Effect(int i, int q=0) : num(i), arg(q){}
    };
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/abilities/");

    /* Self-explainable functions */
    static QString Name(int abnum);
    static Effect Effects(int abnum, int gen);
    static int Number(const QString &ab);
    static QString Message(int ab, int part);
    static int NumberOfAbilities();
    static QString Desc(int abnum);
    static QString EffectDesc(int abnum);
    static bool Exists(int ability, int gen);
private:
    static QList<QString> m_Names;
    static QString m_Directory;
    static QList<Effect> m_Effects[3];
    static QList<QStringList> m_Messages;

    static void loadNames();
    static void loadEffects();
    static QString path(const QString &filename);
};

class GenderInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/genders/");

    /* Self-explainable functions */
    static QString Name(int gender);
    static int NumberOfGenders();
    static int Default(int genderAvail);
    static bool Possible(int gender, int genderAvail);
private:
    static QList<QString> m_Names;
    static QString m_Directory;

    static void loadNames();
    static QString path(const QString &filename);
};

class HiddenPowerInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/types/");

    /* The type of the hidden power depending on the dvs */
    static int Type(quint8 hpdv, quint8 attdv, quint8 defdv, quint8 sattdv, quint8 sdefdv, quint8 spddv);
    /* The power of the hidden power depending on the dvs */
    static int Power(quint8 hpdv, quint8 attdv, quint8 defdv, quint8 sattdv, quint8 sdefdv, quint8 spddv);
    /* the different set of dvs (which are chosen within 30-31) that give an hidden power of that type */
    static QList<QStringList> PossibilitiesForType(int type);
private:
    static QString m_Directory;

    static QString path(const QString &filename);
};

class StatInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/stats/");

    static QString Stat(int stat);
    static QString Status(int status);
    static QString ShortStatus(int status);
private:
    static QString m_Directory;
    static QList<QString> m_stats;
    static QList<QString> m_status;

    static QString path(const QString &filename);
};

#endif // POKEMONINFO_H
