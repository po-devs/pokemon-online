#ifndef POKEMONINFO_H
#define POKEMONINFO_H

#include "pokemonstructs.h"
#include <QtCore>

class PokeBaseStats;
class QPixmap;

class PokemonInfoConfig {
public:
    enum Config {
        Gui = 0,
        NoGui
    };

    static Config config();

    static void setConfig(Config cf);
private:
    static Config _config;
};

/* A class that should be used as a singleton and provide every ressource needed on pokemons */

struct PokemonMoves
{
    //QSet<int> moves;
    /* All moves except egg & special */
    QSet<int> regularMoves[2];
    QSet<int> TMMoves[2];
    QSet<int> preEvoMoves[2];
    QSet<int> levelMoves[2];
    QSet<int> eggMoves[2];
    QSet<int> specialMoves[2];
    QSet<int> tutorMoves[2];
};


class PokemonInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/pokes/");

    /* Self-explainable functions */
    static int TrueCount(int gen=4); // pokes without counting forms
    static int NumberOfPokemons(); // base + all forms.
    static int NumberOfVisiblePokes(); // base + visible forms.
    static QString Name(const Pokemon::uniqueId &pokeid);
    static Pokemon::uniqueId Number(const QString &pokename);
    static int LevelBalance(const Pokemon::uniqueId &pokeid);
    static QString WeightS(const Pokemon::uniqueId &pokeid);
    static QString Classification(const Pokemon::uniqueId &pokeid);
    static float Weight(const Pokemon::uniqueId &pokeid);
    static int Gender(const Pokemon::uniqueId &pokeid);
    static int BaseGender(const Pokemon::uniqueId &pokeid);
    static QByteArray Cry(const Pokemon::uniqueId &pokeid);
    static int Type1(const Pokemon::uniqueId &pokeid);
    static int Type2(const Pokemon::uniqueId &pokeid);
    static QPixmap Picture(const Pokemon::uniqueId &pokeid, int gender = Pokemon::Male, bool shiney = false, bool backimage = false);
    static QPixmap Sub(bool back = false);
    static QPixmap Icon(const Pokemon::uniqueId &pokeid);
    static QSet<int> Moves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> EggMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> LevelMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> TutorMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> TMMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> PreEvoMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> SpecialMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    static QSet<int> RegularMoves(const Pokemon::uniqueId &pokeid, int gen = 4);
    // Base form do NOT count.
    static quint16 NumberOfAFormes(const Pokemon::uniqueId &pokeid);
    static bool AFormesShown(const Pokemon::uniqueId &pokeid);
    /* Standard formes: Rotom, Giratina, Deoxys, .. */
    static bool IsForme(const Pokemon::uniqueId &pokeid);
    static Pokemon::uniqueId OriginalForme(const Pokemon::uniqueId &pokeid);
    static bool HasFormes(const Pokemon::uniqueId &pokeid);
    // Will NOT return base form. Should it?
    static QList<Pokemon::uniqueId> Formes(const Pokemon::uniqueId &pokeid);
    static QList<int> Evos(int pokenum);
    // Will always return base form (subnum 0).
    static Pokemon::uniqueId OriginalEvo(const Pokemon::uniqueId &pokeid);
    static bool IsInEvoChain(const Pokemon::uniqueId &pokeid);
    static PokeBaseStats BaseStats(const Pokemon::uniqueId &pokeid);
    static bool Exists(const Pokemon::uniqueId &pokeid, int gen=4);
    static AbilityGroup Abilities(const Pokemon::uniqueId &pokeid, int gen=4);
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
    static QString m_Directory;
    static QHash<Pokemon::uniqueId, int> m_Type1;
    static QHash<Pokemon::uniqueId, int> m_Type2;
    static QHash<Pokemon::uniqueId, int> m_Genders;
    static QHash<Pokemon::uniqueId, int> m_Ability1[2];
    static QHash<Pokemon::uniqueId, int> m_Ability2[2];
    static QHash<Pokemon::uniqueId, PokeBaseStats> m_BaseStats;
    static QHash<Pokemon::uniqueId, int> m_LevelBalance;
    /* That is NOT multi-threaded! */
    static QHash<quint16, QList<quint16> > m_Evolutions;
    static QHash<quint16, quint16> m_OriginalEvos;
    // A number of forms a pokemon has. 0 for most cases.
    // Keep it as QHash.
    // quint16 as only pokenum matters.
    static QHash<quint16, quint16> m_MaxForme;
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
    // Call this after loading all data.
    static void makeDataConsistent();
    static QSet<int> getMoves(const QString &filename, int Pokenum);
    static QString path(const QString &filename);
    static int calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev);
};

class MoveInfo
{
private:
    static QList<QString> m_Names;
    static QList<QString> m_PowerS;
    static QList<QString> m_AccS;
    static QList<QString> m_Effects;
    static QList<QString> m_SpecialEffects;
    static QList<QStringList> m_MoveMessages;
    static QList<char> m_Type;
    static QList<char> m_PP;
    static QList<char> m_Category;
    static QList<char> m_Critical;
    static QList<char> m_EffectRate;
    static QList<bool> m_Physical;
    static QList<bool> m_KingRock;
    static QList<char> m_Speeds;
    static QList<int> m_Flinch;
    static QList<int> m_Recoil;
    static QList<int> m_Targets;
    static QList<QPair<char, char> > m_Repeat;
    static QList<QString> m_Descriptions;
    static QList<QString> m_Details;
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
    static void init(const QString &dir="db/moves/");

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
    static bool Exists(int movenum);
    static bool isOHKO(int movenum);
    static int EffectRate(int movenum);
    static int Target(int movenum);
    static QString MoveMessage(int moveeffect, int part);
    static QStringList MoveList();
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
    static bool Exists(int itemnum, int gen=4);
    static bool isBerry(int itemnum);
    static bool isPlate(int itemnum);
    static bool isMail(int itemnum);
    static bool isUseful(int itemnum);
    static int PlateType(int itemnum);
    static QList<QString> SortedNames(int gen);
    static QList<QString> SortedUsefulNames(int gen);
    static QList<Effect> Effects(int item);
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
    static QList<QString> m_SortedNames[2];
    static QList<QString> m_SortedUsefulNames[2];
    static QString m_Directory;
    static QList<QList<Effect> > m_RegEffects;
    static QList<QList<Effect> > m_BerryEffects;
    static QList<QStringList> m_RegMessages;
    static QList<QStringList> m_BerryMessages;
    static QList<int> m_Powers;
    static QList<int> m_BerryPowers;
    static QList<int> m_BerryTypes;
    static QList<int> m_UsefulItems;
    static QSet<int> m_3rdGenItems;

    static void loadNames();
    static QString path(const QString &filename);
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

    static QList<QString> m_Names;
    static QString m_Directory;
    static QList<QColor> m_Colors;
    static QList<int> m_TypeVsType;
    static QList<QPixmap> m_Pics;

    static void loadNames();
    static void loadColors();
    static void loadEff();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/types/");

    /* Self-explainable functions */
    static QString Name(int typenum);
    static int Number(const QString &type);
    static QColor Color(int typenum);
    static int Eff(int type_attack, int type_defend); /* Returns how effective it is: 4 = super, 2 = normal, 1 = not much, 0 = ineffective */
    static int NumberOfTypes();
    static int TypeForWeather(int weather);
    static QPixmap Picture(int type);

    static void modifyTypeChart(int type_attack, int type_defend, int value);
};

class NatureInfo
{
private:
    static QList<QString> m_Names;
    static QString m_Directory;
    static void loadNames();
    static QString path(const QString &filename);
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
};

class CategoryInfo
{
private:
    static QList<QString> m_Names;
    static QString m_Directory;
    static QList<QColor> m_Colors;

    static void loadNames();
    static void loadColors();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/categories/");

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
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/abilities/");

    /* Self-explainable functions */
    static QString Name(int abnum);
    static Effect Effects(int abnum);
    static int Number(const QString &ab);
    static QString Message(int ab, int part);
    static int NumberOfAbilities();
    static QString Desc(int abnum);
    static QString EffectDesc(int abnum);
    static bool Exists(int ability, int gen);
private:
    static QList<QString> m_Names;
    static QString m_Directory;
    static QList<Effect> m_Effects;
    static QList<QStringList> m_Messages;
    static QSet<int> m_3rdGenAbilities;

    static void loadNames();
    static void loadEffects();
    static QString path(const QString &filename);
};

class GenderInfo
{
private:
    static QList<QString> m_Names;
    static QString m_Directory;
    static QList<QPixmap> m_Pictures;
    static QList<QPixmap> m_BattlePictures;

    static void loadNames();
    static void loadPixmaps();
    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/genders/");

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
    static void init(const QString &dir="db/types/");

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
    static QList<QString> m_stats;
    static QList<QString> m_status;
    static QHash<int, QPixmap> m_statusIcons;
    static QHash<int, QPixmap> m_battleIcons;

    static QString path(const QString &filename);
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/stats/");

    static QString Stat(int stat);
    static QString Status(int status);
    static QString ShortStatus(int status);
    static QColor StatusColor(int status);
    static QPixmap Icon(int status);
    static QPixmap BattleIcon(int status);
};

#endif // POKEMONINFO_H
