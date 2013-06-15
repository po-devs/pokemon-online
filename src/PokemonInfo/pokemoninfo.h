#ifndef POKEMONINFO_H
#define POKEMONINFO_H

#include "pokemon.h"
#include "geninfo.h"
#include "pokemonstructs.h"
#include <QtCore>
#include <QMovie>

class PokeBaseStats;
class QPixmap;

namespace FillMode {
    enum FillModeType {
        NoMod,
        Server,
        Client
    };
}

namespace PokemonInfoConfig {
    /* If set to yes, when loading moves for the last subgen of the generation,
      rather than loading prepared files it will load every subgen and calculate the
      correct movepool for the pokemon.

      Default is false.
    */
    void setLastSubgenToWhole(bool yes);

    void setFillMode(FillMode::FillModeType mode);
    void changeTranslation(const QString& ts = QString());
    void changeMod(const QString &mod);

    const QString& dataRepo();
    void setDataRepo(const QString &s);

    QStringList allFiles(const QString &filename, bool trans=false);
    QString currentMod();
    QString currentModPath();
    FillMode::FillModeType getFillMode();

    QStringList availableMods();
}

/* A class that should be used as a singleton and provide every ressource needed on pokemons */

struct PokemonMoves
{
    //QSet<int> moves;
    /* All moves except egg & special */
    QSet<int> regularMoves;
    QSet<int> TMMoves;
    QSet<int> preEvoMoves;
    QSet<int> levelMoves;
    QSet<int> eggMoves;
    QSet<int> specialMoves;
    QSet<int> tutorMoves;
    QSet<int> genMoves;
    QSet<int> dreamWorldMoves;
};


class PokemonInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/pokes/");

    /* Self-explainable functions */
    static int TrueCount(); // pokes without counting forms
    static int NumberOfPokemons(); // base + all forms.
    static QString Name(const Pokemon::uniqueId &pokeid);
    static Pokemon::uniqueId Number(const QString &pokename);
    static int LevelBalance(const Pokemon::uniqueId &pokeid);
    static QString WeightS(const Pokemon::uniqueId &pokeid);
    static QString Classification(const Pokemon::uniqueId &pokeid);
    /* 4 = 50% male, 7 = 87.5% male, 6 = 75% male. Range is 0-8 */
    static int GenderRate(const Pokemon::uniqueId &pokeid);
    static int Weight(const Pokemon::uniqueId &pokeid);
    static int Gender(const Pokemon::uniqueId &pokeid);
    static int BaseGender(const Pokemon::uniqueId &pokeid);
    static QByteArray Cry(const Pokemon::uniqueId &pokeid, bool mod=true);
    static int Type1(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static int Type2(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QPixmap Picture(const Pokemon::uniqueId &pokeid, Pokemon::gen gen = GenInfo::GenMax(), int gender = Pokemon::Male, bool shiney = false, bool backimage = false, bool mod=true);
    static QPixmap Picture(const QString &url);
    static QMovie  *AnimatedSprite(const Pokemon::uniqueId &pokeId, int gender, bool shiny, bool back);
    static bool HasAnimatedSprites();
    static bool HasAnimatedSpritesEnabled();
    static QPixmap Sub(Pokemon::gen gen=5, bool back = false);
    static QPixmap Icon(const Pokemon::uniqueId &pokeid, bool mod = true);
    static bool HasMoveInGen(const Pokemon::uniqueId &pokeid, int move, Pokemon::gen gen);
    static QSet<int> Moves(const Pokemon::uniqueId &pokeid, Pokemon::gen);
    static QSet<int> EggMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> LevelMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> TutorMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> TMMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> PreEvoMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> SpecialMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> RegularMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QSet<int> dreamWorldMoves(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
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
    // Will NOT return base form.
    static QList<Pokemon::uniqueId> Formes(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QList<Pokemon::uniqueId> VisibleFormes(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static int MinLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static int MinEggLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static int AbsoluteMinLevel(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static QList<int> Evos(int pokenum);
    static QList<int> DirectEvos(int pokenum);
    static bool HasEvolutions(int pokenum);

    // Will always return base form (subnum 0).
    static Pokemon::uniqueId OriginalEvo(const Pokemon::uniqueId &pokeid);
    //Returns 0 if no preevo
    static int PreEvo(int pokenum);
    static bool HasPreEvo(int pokenum);
    static bool IsInEvoChain(const Pokemon::uniqueId &pokeid);
    static PokeBaseStats BaseStats(const Pokemon::uniqueId &pokeid);
    static int SpecialStat(const Pokemon::uniqueId &pokeid);
    static bool Released(const Pokemon::uniqueId &pokeid, Pokemon::gen gen); /* Does not check for exists first, do it yourself */
    static bool Exists(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static bool Exists(const Pokemon::uniqueId &pokeid);
    static AbilityGroup Abilities(const Pokemon::uniqueId &pokeid, Pokemon::gen gen);
    static int Ability(const Pokemon::uniqueId &pokeid, int slot, Pokemon::gen gen);
    static int Stat(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int stat, int level, quint8 dv, quint8 ev);
    static int FullStat(const Pokemon::uniqueId &pokeid, Pokemon::gen gen, int nature, int stat, int level, quint8 dv, quint8 ev);
    static int BoostedStat(int stat, int boost);
    static QString Desc(const Pokemon::uniqueId &pokeid, int cartridge);
    static QString Height(const Pokemon::uniqueId &pokeid);
    // Will NOT return Missingno.
    static Pokemon::uniqueId getRandomPokemon(Pokemon::gen gen=GenInfo::GenMax());

    static void RunMovesSanityCheck(int gen);

    static void retranslate();

    struct Gen {
        Pokemon::gen gen;
        QString dir;

        void load(const QString &path, const Pokemon::gen &gen, Gen *parent=NULL);
        void loadMoves(Gen *parent);
        void loadReleased(Gen *parent);
        void loadMinLevels(Gen *parent);

        bool isReleased(const Pokemon::uniqueId &);

        QString path(const QString &filename);

        QHash<Pokemon::uniqueId, PokemonMoves> m_Moves;
        QSet<Pokemon::uniqueId> m_Released;

        QHash<Pokemon::uniqueId, int> m_MinLevels;
        QHash<Pokemon::uniqueId, int> m_MinEggLevels;
    };

    static Gen & gen(Pokemon::gen gen);
private:
    static QHash<Pokemon::gen, Gen> gens;

    static QVector<QHash<Pokemon::uniqueId, int> > m_Type1;
    static QVector<QHash<Pokemon::uniqueId, int> > m_Type2;
    static QVector<QHash<Pokemon::uniqueId, int> > m_Abilities [3];

    // m_Names is a base.
    // It is assumed that anything that is not there do not exist at all.
    // Is a map because we need it to be sorted.
    static QMap<Pokemon::uniqueId, QString> m_Names;
    static QHash<Pokemon::uniqueId, QString> m_Weights;
    static QHash<int, QHash<int, QString> > m_Desc;
    static QHash<int, QString> m_Classification;
    static QHash<int, int> m_GenderRates;
    static QHash<Pokemon::uniqueId, QString> m_Height;
    static QString m_Directory;

    static QHash<Pokemon::uniqueId, int> m_Genders;
    static QHash<Pokemon::uniqueId, PokeBaseStats> m_BaseStats;
    static QHash<Pokemon::uniqueId, int> m_SpecialStats;
    static QHash<Pokemon::uniqueId, int> m_LevelBalance;

    static QHash<int, QList<int> > m_Evolutions;
    static QHash<int, int> m_OriginalEvos;
    static QHash<int, int> m_PreEvos;
    static QHash<int, QList<int> > m_DirectEvos;
    /* Tells if there is a real difference from the original forme.
       If not, tiers will consider the pokemon as its original forme, to avoid
       listing too many pokemons */
    static QSet<Pokemon::uniqueId> m_AestheticFormes;
    // A number of forms a pokemon has. 0 for most cases.
    // Keep it as QHash.
    // quint16 as only pokenum matters.
    static QHash<int, quint16> m_MaxForme;
    // Holds 1-letter options.
    // Sample use: if(m_Options.value(pokeid).contains('H')) whatever();
    // Values for pokemons.txt:
    // 1 - always 1 HP.
    // H - hidden form(e).
    static QHash<Pokemon::uniqueId, QString> m_Options;

    static int m_trueNumberOfPokes;

    static void loadNames();
    static void loadGen(Pokemon::gen g);
    static void loadEvos();
    static void loadBaseStats();
    static void loadClassifications();
    static void loadGenderRates();
    static void loadHeights();
    static void loadDescriptions();
    // Call this after loading all data.
    static void makeDataConsistent();
    static QSet<int> getMoves(const QString &filename, int Pokenum);
    static QString path(const QString &filename, const Pokemon::gen &g = 0);
    static int calc_stat(int gen, quint8 basestat, int level, quint8 dv, quint8 ev);
};

class MoveInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/moves/");
    static void retranslate();

    /* Self-explainable functions */
    static QString Name(int movenum);
    static int Type(int movenum, Pokemon::gen gen);
    static int Category(int movenum, Pokemon::gen gen);
    static int Classification(int movenum, Pokemon::gen gen);
    static int Number(const QString &movename);
    static int NumberOfMoves();
    static int NumberOfMoves(Pokemon::gen gen);
    static int FlinchRate(int movenum, Pokemon::gen gen);
    static int Recoil(int movenum, Pokemon::gen gen);
    static QString Description(int movenum, Pokemon::gen gen);
    static QString DetailedDescription(int movenum);
    static int Power(int movenum, Pokemon::gen gen);
    /* gives the power of a move in the form of a string */
    static QString PowerS(int movenum, Pokemon::gen gen);
    static int PP(int movenum, Pokemon::gen gen);
    static int Acc(int movenum, Pokemon::gen gen);
    /* gives the accuracy of a move in the form of a string */
    static QString AccS(int movenum, Pokemon::gen gen);
    static int CriticalRaise(int movenum, Pokemon::gen gen);
    static int RepeatMin(int movenum, Pokemon::gen gen);
    static int RepeatMax(int movenum, Pokemon::gen gen);
    static int SpeedPriority(int movenum, Pokemon::gen gen);
    static int Flags(int movenum, Pokemon::gen gen);
    static bool Exists(int movenum, Pokemon::gen gen);
    static bool isOHKO(int movenum, Pokemon::gen gen);
    static bool isHM(int movenum, Pokemon::gen gen);
    static bool FlinchByKingRock(int movenum, Pokemon::gen gen);
    static int EffectRate(int movenum, Pokemon::gen gen);
    static quint32 StatAffected(int movenum, Pokemon::gen gen);
    static quint32 BoostOfStat(int movenum, Pokemon::gen gen);
    static quint32 RateOfStat(int movenum, Pokemon::gen gen);
    static int Target(int movenum, Pokemon::gen gen);
    static int Healing(int movenum, Pokemon::gen gen);
    static int MinTurns(int movenum, Pokemon::gen gen);
    static int MaxTurns(int movenum, Pokemon::gen gen);
    static int Status(int movenum, Pokemon::gen gen);
    static int StatusKind(int movenum, Pokemon::gen gen);
    static int ConvertFromOldMove(int oldmovenum);
    static QString MoveMessage(int moveeffect, int part);
    /* the status mod of a move*/
    //static QString Effect(int movenum, int gen);
    static QString SpecialEffect(int movenum, Pokemon::gen gen);
    static QSet<int> Moves(Pokemon::gen gen);
private:
    static QHash<int, QString> m_Names;
    static QHash<QString, int> m_LowerCaseMoves;
    static QHash<int, QStringList> m_MoveMessages;
    static QHash<int, QString> m_Details;
    static QHash<int,int> m_OldMoves;
    static QHash<int,bool> m_KingRock;
    static QVector<QSet<int> > m_GenMoves;

    struct Gen {
        Gen() {
            parent = NULL;
        }

        Gen *parent;

        void load(const QString &path, Pokemon::gen gen);
        void retranslate();

        QString path(const QString &fileName);

        Pokemon::gen gen;
        QString dir;

        QHash<int, char> accuracy;
        QHash<int, char> category;
        QHash<int, char> causedEffect;
        QHash<int, char> critRate;
        QHash<int, char> damageClass;
        QHash<int, QString> effect;
        QHash<int, QString> specialEffect;
        QHash<int, char> effectChance;
        QHash<int, int> flags;
        QHash<int, char> flinchChance;
        QHash<int, signed char> healing;
        QHash<int, char> maxTurns;
        QHash<int, char> minTurns;
        QHash<int, char> minMaxHits;
        QHash<int,long> stataffected;
        QHash<int,long> statboost;
        QHash<int,long> statrate;
        QHash<int, unsigned char> power;
        QHash<int, char> pp;
        QHash<int, signed char> priority;
        QHash<int, char> range;
        QHash<int, signed char> recoil;
        QHash<int, char> status;
        QHash<int, char> type;
        QSet<int> HMs;
    };

    static QString m_Directory;
    static QHash<Pokemon::gen, Gen> gens;

    static void loadNames();
    static void loadMoveMessages();
    static void loadDetails();

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
    static void retranslate();

    /* Self-explainable functions */
    static int NumberOfItems();
    static QString Name(int itemnum);
    static bool Exists(int itemnum);
    static bool Exists(int itemnum, Pokemon::gen gen);
    static bool isBerry(int itemnum);
    static bool isPlate(int itemnum);
    static bool isDrive(int itemnum);
    static bool isJewel(int itemnum);
    static bool isMail(int itemnum);
    static bool isUseful(int itemnum);
    static int PlateType(int itemnum);
    static int PlateForType(int type);
    static int DriveType(int itemnum);
    static int DriveForme(int itemnum);
    static int DriveForForme(int forme);
    static bool IsBattleItem(int itemnum, Pokemon::gen gen);
    static int Target(int itemnum, Pokemon::gen gen);
    static QList<QString> SortedNames(Pokemon::gen gen);
    static QList<QString> SortedUsefulNames(Pokemon::gen gen);
    static QList<Effect> Effects(int item, Pokemon::gen gen);
    static QString Message(int item, int part);
    static int Number(const QString &itemname);
    static QString Description(int itemnum);
    static int Power(int itemnum);
    static int BerryPower(int itemnum);
    static int BerryType(int itemnum);
    static QPixmap Icon(int itemnum);
    static QPixmap HeldItem();
private:
    static QHash<int,QString> m_BerryNames;
    static QHash<int,QString> m_RegItemNames;
    static QHash<QString, int> m_BerryNamesH;
    static QHash<QString, int> m_ItemNamesH;
    static QVector<QList<QString> > m_SortedNames;
    static QVector<QList<QString> > m_SortedUsefulNames;
    static QString m_Directory;
    static QVector<QHash<int, QList<Effect> > > m_RegEffects;
    static QHash<int, QList<Effect> > m_BerryEffects;
    static QHash<int, QStringList> m_RegMessages;
    static QHash<int, QStringList> m_BerryMessages;
    static QHash<int,int> m_Powers;
    static QHash<int,int> m_BerryPowers;
    static QHash<int,int> m_BerryTypes;
    static QHash<int, bool> m_UsefulItems;
    static QVector<QSet<int> > m_GenItems;

    static void loadNames();
    static void loadEffects();
    static void loadFlingData();
    static void loadGenData();
    static void loadMessages();
    static QString path(const QString &filename);
};

class TypeInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/types/");
    static void retranslate();

    /* Self-explainable functions */
    static QString Name(int typenum);
    static int Number(const QString &type);
    static int Eff(int type_attack, int type_defend, Pokemon::gen gen = GenInfo::GenMax()); /* Returns how effective it is: 4 = super, 2 = normal, 1 = not much, 0 = ineffective */
    static int NumberOfTypes();
    static int TypeForWeather(int weather);
    static int Category(int type);
    static QString weatherName(int weather);
private:
    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    static QHash<int, QString> m_Names;
    static QString m_Directory;
    static QHash<int, QVector<int> > m_TypeVsType;
    static QHash<int, QVector<int> > m_TypeVsTypeGen1;
    static QHash<int, int> m_Categories;

    static void loadNames();
    static void loadCategories();
    static void loadEff();
    static QString path(const QString &filename);
};

class NatureInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/natures/");
    static void retranslate();

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
    static QHash<int, QString> m_Names;
    static QString m_Directory;
    static void loadNames();
    static QString path(const QString &filename);
};

class CategoryInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/categories/");
    static void retranslate();

    /* Self-explainable functions */
    static QString Name(int catnum);
    static int NumberOfCategories();
private:
    static QHash<int, QString> m_Names;
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
        Effect(int i=0, int q=0) : num(i), arg(q){}
    };
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/abilities/");
    static void retranslate();

    /* Self-explainable functions */
    static QString Name(int abnum);
    static Effect Effects(int abnum, Pokemon::gen gen);
    static int Number(const QString &ab);
    static QString Message(int ab, int part);
    static int NumberOfAbilities(Pokemon::gen gen);
    static QString Desc(int abnum);
    static QString EffectDesc(int abnum);
    static bool Exists(int ability, Pokemon::gen gen);
    static int ConvertFromOldAbility(int oldability);
private:
    static QHash<int, QString> m_Names;
    static QString m_Directory;
    static QVector<QHash<int,Effect> > m_Effects;
    static QHash<int,QStringList> m_Messages;
    static QHash<int,int> m_OldAbilities;
    static QHash<int,QString> m_Desc;
    static QHash<int,QString> m_BattleDesc;

    static void loadNames();
    static void loadMessages();
    static void loadEffects();
    static QString path(const QString &filename);
};

class GenderInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/genders/");
    static void retranslate();

    /* Self-explainable functions */
    static QString Name(int gender);
    static int NumberOfGenders();
    static int Default(int genderAvail);
    static bool Possible(int gender, int genderAvail);
private:
    static QHash<int, QString> m_Names;
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
    static int Type(Pokemon::gen gen, quint8 hpdv, quint8 attdv, quint8 defdv, quint8 sattdv, quint8 sdefdv, quint8 spddv);
    /* The power of the hidden power depending on the dvs */
    static int Power(Pokemon::gen gen, quint8 hpdv, quint8 attdv, quint8 defdv, quint8 sattdv, quint8 sdefdv, quint8 spddv);
    /* the different set of dvs (which are chosen within 30-31) that give an hidden power of that type */
    static QList<QStringList> PossibilitiesForType(int type);

    static QPair<quint8,quint8> AttDefDVsForGen2(int type);
private:
    static QString m_Directory;

    static QString path(const QString &filename);
};

class StatInfo
{
public:
    /* directory where all the data is */
    static void init(const QString &dir="db/stats/");
    static void retranslate();

    static QString Stat(int stat, const Pokemon::gen &gen);
    static QString Status(int status);
    static QString ShortStatus(int status);
private:
    static QString m_Directory;
    static QHash<int, QString> m_stats;
    static QHash<int, QString> m_status;

    static QString path(const QString &filename);
};

#endif // POKEMONINFO_H
