#ifndef POKEMONSTRUCTS_H
#define POKEMONSTRUCTS_H

#include <QtGui>
#include <QDataStream>
#include "../Utilities/functions.h"

class QDomElement;

namespace Version
{
    enum {
        Platinum = 14,
        SoulSilver = 15,
        HeartGold = 16
    };
}

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

    enum Name
    {
        NoPoke,
        Bulbasaur,
        Ivysaur,
        Venusaur,
        Charmander,
        Charmeleon,
        Charizard,
        Squirtle,
        Wartortle,
        Blastoise,
        Caterpie,
        Metapod,
        Butterfree,
        Weedle,
        Kakuna,
        Beedrill,
        Pidgey,
        Pidgeotto,
        Pidgeot,
        Rattata,
        Raticate,
        Spearow,
        Fearow,
        Ekans,
        Arbok,
        Pikachu,
        Raichu,
        Sandshrew,
        Sandslash,
        Nidoran_F,
        Nidorina,
        Nidoqueen,
        Nidoran_M,
        Nidorino,
        Nidoking,
        Clefairy,
        Clefable,
        Vulpix,
        Ninetales,
        Jigglypuff,
        Wigglytuff,
        Zubat,
        Golbat,
        Oddish,
        Gloom,
        Vileplume,
        Paras,
        Parasect,
        Venonat,
        Venomoth,
        Diglett,
        Dugtrio,
        Meowth,
        Persian,
        Psyduck,
        Golduck,
        Mankey,
        Primeape,
        Growlithe,
        Arcanine,
        Poliwag,
        Poliwhirl,
        Poliwrath,
        Abra,
        Kadabra,
        Alakazam,
        Machop,
        Machoke,
        Machamp,
        Bellsprout,
        Weepinbell,
        Victreebel,
        Tentacool,
        Tentacruel,
        Geodude,
        Graveler,
        Golem,
        Ponyta,
        Rapidash,
        Slowpoke,
        Slowbro,
        Magnemite,
        Magneton,
        Farfetchd,
        Doduo,
        Dodrio,
        Seel,
        Dewgong,
        Grimer,
        Muk,
        Shellder,
        Cloyster,
        Gastly,
        Haunter,
        Gengar,
        Onix,
        Drowzee,
        Hypno,
        Krabby,
        Kingler,
        Voltorb,
        Electrode,
        Exeggcute,
        Exeggutor,
        Cubone,
        Marowak,
        Hitmonlee,
        Hitmonchan,
        Lickitung,
        Koffing,
        Weezing,
        Rhyhorn,
        Rhydon,
        Chansey,
        Tangela,
        Kangaskhan,
        Horsea,
        Seadra,
        Goldeen,
        Seaking,
        Staryu,
        Starmie,
        MrMime,
        Scyther,
        Jynx,
        Electabuzz,
        Magmar,
        Pinsir,
        Tauros,
        Magikarp,
        Gyarados,
        Lapras,
        Ditto,
        Eevee,
        Vaporeon,
        Jolteon,
        Flareon,
        Porygon,
        Omanyte,
        Omastar,
        Kabuto,
        Kabutops,
        Aerodactyl,
        Snorlax,
        Articuno,
        Zapdos,
        Moltres,
        Dratini,
        Dragonair,
        Dragonite,
        Mewtwo,
        Mew,
        Chikorita,
        Bayleef,
        Meganium,
        Cyndaquil,
        Quilava,
        Typhlosion,
        Totodile,
        Croconaw,
        Feraligatr,
        Sentret,
        Furret,
        Hoothoot,
        Noctowl,
        Ledyba,
        Ledian,
        Spinarak,
        Ariados,
        Crobat,
        Chinchou,
        Lanturn,
        Pichu,
        Cleffa,
        Igglybuff,
        Togepi,
        Togetic,
        Natu,
        Xatu,
        Mareep,
        Flaaffy,
        Ampharos,
        Bellossom,
        Marill,
        Azumarill,
        Sudowoodo,
        Politoed,
        Hoppip,
        Skiploom,
        Jumpluff,
        Aipom,
        Sunkern,
        Sunflora,
        Yanma,
        Wooper,
        Quagsire,
        Espeon,
        Umbreon,
        Murkrow,
        Slowking,
        Misdreavus,
        Unown,
        Wobbuffet,
        Girafarig,
        Pineco,
        Forretress,
        Dunsparce,
        Gligar,
        Steelix,
        Snubbull,
        Granbull,
        Qwilfish,
        Scizor,
        Shuckle,
        Heracross,
        Sneasel,
        Teddiursa,
        Ursaring,
        Slugma,
        Magcargo,
        Swinub,
        Piloswine,
        Corsola,
        Remoraid,
        Octillery,
        Delibird,
        Mantine,
        Skarmory,
        Houndour,
        Houndoom,
        Kingdra,
        Phanpy,
        Donphan,
        Porygon2,
        Stantler,
        Smeargle,
        Tyrogue,
        Hitmontop,
        Smoochum,
        Elekid,
        Magby,
        Miltank,
        Blissey,
        Raikou,
        Entei,
        Suicune,
        Larvitar,
        Pupitar,
        Tyranitar,
        Lugia,
        Ho_Oh,
        Celebi,
        Treecko,
        Grovyle,
        Sceptile,
        Torchic,
        Combusken,
        Blaziken,
        Mudkip,
        Marshtomp,
        Swampert,
        Poochyena,
        Mightyena,
        Zigzagoon,
        Linoone,
        Wurmple,
        Silcoon,
        Beautifly,
        Cascoon,
        Dustox,
        Lotad,
        Lombre,
        Ludicolo,
        Seedot,
        Nuzleaf,
        Shiftry,
        Taillow,
        Swellow,
        Wingull,
        Pelipper,
        Ralts,
        Kirlia,
        Gardevoir,
        Surskit,
        Masquerain,
        Shroomish,
        Breloom,
        Slakoth,
        Vigoroth,
        Slaking,
        Nincada,
        Ninjask,
        Shedinja,
        Whismur,
        Loudred,
        Exploud,
        Makuhita,
        Hariyama,
        Azurill,
        Nosepass,
        Skitty,
        Delcatty,
        Sableye,
        Mawile,
        Aron,
        Lairon,
        Aggron,
        Meditite,
        Medicham,
        Electrike,
        Manectric,
        Plusle,
        Minun,
        Volbeat,
        Illumise,
        Roselia,
        Gulpin,
        Swalot,
        Carvanha,
        Sharpedo,
        Wailmer,
        Wailord,
        Numel,
        Camerupt,
        Torkoal,
        Spoink,
        Grumpig,
        Spinda,
        Trapinch,
        Vibrava,
        Flygon,
        Cacnea,
        Cacturne,
        Swablu,
        Altaria,
        Zangoose,
        Seviper,
        Lunatone,
        Solrock,
        Barboach,
        Whiscash,
        Corphish,
        Crawdaunt,
        Baltoy,
        Claydol,
        Lileep,
        Cradily,
        Anorith,
        Armaldo,
        Feebas,
        Milotic,
        Castform,
        Kecleon,
        Shuppet,
        Banette,
        Duskull,
        Dusclops,
        Tropius,
        Chimecho,
        Absol,
        Wynaut,
        Snorunt,
        Glalie,
        Spheal,
        Sealeo,
        Walrein,
        Clamperl,
        Huntail,
        Gorebyss,
        Relicanth,
        Luvdisc,
        Bagon,
        Shelgon,
        Salamence,
        Beldum,
        Metang,
        Metagross,
        Regirock,
        Regice,
        Registeel,
        Latias,
        Latios,
        Kyogre,
        Groudon,
        Rayquaza,
        Jirachi,
        Deoxys,
        Turtwig,
        Grotle,
        Torterra,
        Chimchar,
        Monferno,
        Infernape,
        Piplup,
        Prinplup,
        Empoleon,
        Starly,
        Staravia,
        Staraptor,
        Bidoof,
        Bibarel,
        Kricketot,
        Kricketune,
        Shinx,
        Luxio,
        Luxray,
        Budew,
        Roserade,
        Cranidos,
        Rampardos,
        Shieldon,
        Bastiodon,
        Burmy,
        Wormadam,
        Mothim,
        Combee,
        Vespiquen,
        Pachirisu,
        Buizel,
        Floatzel,
        Cherubi,
        Cherrim,
        Shellos,
        Gastrodon,
        Ambipom,
        Drifloon,
        Drifblim,
        Buneary,
        Lopunny,
        Mismagius,
        Honchkrow,
        Glameow,
        Purugly,
        Chingling,
        Stunky,
        Skuntank,
        Bronzor,
        Bronzong,
        Bonsly,
        MimeJr,
        Happiny,
        Chatot,
        Spiritomb,
        Gible,
        Gabite,
        Garchomp,
        Munchlax,
        Riolu,
        Lucario,
        Hippopotas,
        Hippowdon,
        Skorupi,
        Drapion,
        Croagunk,
        Toxicroak,
        Carnivine,
        Finneon,
        Lumineon,
        Mantyke,
        Snover,
        Abomasnow,
        Weavile,
        Magnezone,
        Lickilicky,
        Rhyperior,
        Tangrowth,
        Electivire,
        Magmortar,
        Togekiss,
        Yanmega,
        Leafeon,
        Glaceon,
        Gliscor,
        Mamoswine,
        Porygon_Z,
        Gallade,
        Probopass,
        Dusknoir,
        Froslass,
        Rotom,
        Uxie,
        Mesprit,
        Azelf,
        Dialga,
        Palkia,
        Heatran,
        Regigigas,
        Giratina,
        Cresselia,
        Phione,
        Manaphy,
        Darkrai,
        Shaymin,
        Arceus,
        // Base forms end here.
        Rotom_C = Rotom + (1 << 16),
        Rotom_H = Rotom + (2 << 16),
        Rotom_F = Rotom + (3 << 16),
        Rotom_W = Rotom + (4 << 16),
        Rotom_S = Rotom + (5 << 16),
        Deoxys_A = Deoxys + (1 << 16),
        Deoxys_D = Deoxys + (2 << 16),
        Deoxys_S = Deoxys + (3 << 16),
        Wormadam_G = Wormadam + (1 << 16),
        Wormadam_S = Wormadam + (2 << 16),
        Giratina_O = Giratina + (1 << 16),
        Shaymin_S = Shaymin + (1 << 16)
    };
    class uniqueId
    {
    public:
        quint16 pokenum;
        quint16 subnum;
        uniqueId() : pokenum(0), subnum(0) {}
        uniqueId(int num, int subnum) : pokenum(num), subnum(subnum) {}
        uniqueId(const uniqueId &id) { pokenum = id.pokenum; subnum = id.subnum; }
        uniqueId(quint32 pokeRef) {
              subnum = pokeRef >> 16;
              pokenum = pokeRef & 0xFFFF;
        }
        bool operator == (const uniqueId &other) const {
            return (pokenum == other.pokenum) && (subnum == other.subnum);
        }
        bool operator != (const uniqueId &other) const {
            return (pokenum != other.pokenum) || (subnum != other.subnum);
        }
        bool operator < (const uniqueId &other) const {
            return (pokenum < other.pokenum) || ((pokenum == other.pokenum) && (subnum < other.subnum));
        }
        bool operator > (const uniqueId &other) const {
            return (pokenum > other.pokenum) || ((pokenum == other.pokenum) && (subnum > other.subnum));
        }
        QString toString() const;
        QString toLine(const QString &data) const;
        quint32 toPokeRef() const;
        // Separates pokenum:subnum:1-letter-options data from
        // the other part of a string.
        // 'data' will be modified to hold extracted data.
        // 'remaining' will be modified to hold remaining part.
        // Will return true if everything is fine. And false otherwise.
        static bool extract(const QString &raw, uniqueId &id, QString &info, QString *options = NULL);
        // Extracts short data in a "pokenum data_text" form.
        static bool extract_short(const QString &from, quint16 &pokenum, QString &remaining);
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

    enum Target
    {
	None = -1,
	User,
	ChosenTarget,
	RandomTarget,
	Opponents,
	All,
        AllButSelf,
        PartnerOrUser
    };

    enum Name
    {
        NoMove,
        Absorb,
        Acid,
        AcidArmor,
        Acupressure,
        AerialAce,
        Aeroblast,
        Agility,
        AirCutter,
        AirSlash,
        Amnesia,
        AncientPower,
        AquaJet,
        AquaRing,
        AquaTail,
        ArmThrust,
        Aromatherapy,
        Assist,
        Assurance,
        Astonish,
        AttackOrder,
        Attract,
        AuraSphere,
        AuroraBeam,
        Avalanche,
        Barrage,
        Barrier,
        BatonPass,
        BeatUp,
        BellyDrum,
        Bide,
        Bind,
        Bite,
        BlastBurn,
        BlazeKick,
        Blizzard,
        Block,
        BodySlam,
        BoneClub,
        BoneRush,
        Bonemerang,
        Bounce,
        BraveBird,
        BrickBreak,
        Brine,
        Bubble,
        BubbleBeam,
        BugBite,
        BugBuzz,
        BulkUp,
        BulletPunch,
        BulletSeed,
        CalmMind,
        Camouflage,
        Captivate,
        Charge,
        ChargeBeam,
        Charm,
        Chatter,
        Clamp,
        CloseCombat,
        CometPunch,
        ConfuseRay,
        Confusion,
        Constrict,
        Conversion,
        Conversion2,
        Copycat,
        CosmicPower,
        CottonSpore,
        Counter,
        Covet,
        Crabhammer,
        CrossChop,
        CrossPoison,
        Crunch,
        CrushClaw,
        CrushGrip,
        Curse,
        Cut,
        DarkPulse,
        DarkVoid,
        DefenseCurl,
        DefendOrder,
        Defog,
        DestinyBond,
        Detect,
        Dig,
        Disable,
        Discharge,
        Dive,
        DizzyPunch,
        DoomDesire,
        DoubleHit,
        DoubleKick,
        DoubleTeam,
        Double_Edge,
        DoubleSlap,
        DracoMeteor,
        DragonClaw,
        DragonDance,
        DragonPulse,
        DragonRage,
        DragonRush,
        DragonBreath,
        DrainPunch,
        DreamEater,
        DrillPeck,
        DynamicPunch,
        EarthPower,
        Earthquake,
        EggBomb,
        Embargo,
        Ember,
        Encore,
        Endeavor,
        Endure,
        EnergyBall,
        Eruption,
        Explosion,
        Extrasensory,
        ExtremeSpeed,
        Facade,
        FaintAttack,
        FakeOut,
        FakeTears,
        FalseSwipe,
        FeatherDance,
        Feint,
        FireBlast,
        FireFang,
        FirePunch,
        FireSpin,
        Fissure,
        Flail,
        FlameWheel,
        Flamethrower,
        FlareBlitz,
        Flash,
        FlashCannon,
        Flatter,
        Fling,
        Fly,
        FocusBlast,
        FocusEnergy,
        FocusPunch,
        FollowMe,
        ForcePalm,
        Foresight,
        FrenzyPlant,
        Frustration,
        FuryAttack,
        FuryCutter,
        FurySwipes,
        FutureSight,
        GastroAcid,
        GigaDrain,
        GigaImpact,
        Glare,
        GrassKnot,
        GrassWhistle,
        Gravity,
        Growl,
        Growth,
        Grudge,
        GuardSwap,
        Guillotine,
        GunkShot,
        Gust,
        GyroBall,
        Hail,
        HammerArm,
        Harden,
        Haze,
        HeadSmash,
        Headbutt,
        HealBell,
        HealBlock,
        HealOrder,
        HealingWish,
        HeartSwap,
        HeatWave,
        HelpingHand,
        HiJumpKick,
        HiddenPower,
        HornAttack,
        HornDrill,
        Howl,
        HydroCannon,
        HydroPump,
        HyperBeam,
        HyperFang,
        HyperVoice,
        Hypnosis,
        IceBall,
        IceBeam,
        IceFang,
        IcePunch,
        IceShard,
        IcicleSpear,
        IcyWind,
        Imprison,
        Ingrain,
        IronDefense,
        IronHead,
        IronTail,
        Judgment,
        JumpKick,
        KarateChop,
        Kinesis,
        KnockOff,
        LastResort,
        LavaPlume,
        LeafBlade,
        LeafStorm,
        LeechLife,
        LeechSeed,
        Leer,
        Lick,
        LightScreen,
        Lock_On,
        LovelyKiss,
        LowKick,
        LuckyChant,
        LunarDance,
        LusterPurge,
        MachPunch,
        MagicCoat,
        MagicalLeaf,
        MagmaStorm,
        MagnetBomb,
        MagnetRise,
        Magnitude,
        MeFirst,
        MeanLook,
        Meditate,
        MegaDrain,
        MegaKick,
        MegaPunch,
        Megahorn,
        Memento,
        MetalBurst,
        MetalClaw,
        MetalSound,
        MeteorMash,
        Metronome,
        MilkDrink,
        Mimic,
        MindReader,
        Minimize,
        MiracleEye,
        MirrorCoat,
        MirrorMove,
        MirrorShot,
        Mist,
        MistBall,
        Moonlight,
        MorningSun,
        MudBomb,
        MudShot,
        MudSport,
        Mud_Slap,
        MuddyWater,
        NastyPlot,
        NaturalGift,
        NaturePower,
        NeedleArm,
        NightShade,
        NightSlash,
        Nightmare,
        Octazooka,
        OdorSleuth,
        OminousWind,
        Outrage,
        Overheat,
        PainSplit,
        PayDay,
        Payback,
        Peck,
        PerishSong,
        PetalDance,
        PinMissile,
        Pluck,
        PoisonFang,
        PoisonGas,
        PoisonJab,
        PoisonSting,
        PoisonTail,
        PoisonPowder,
        Pound,
        PowderSnow,
        PowerGem,
        PowerSwap,
        PowerTrick,
        PowerWhip,
        Present,
        Protect,
        Psybeam,
        PsychUp,
        Psychic,
        PsychoBoost,
        PsychoCut,
        PsychoShift,
        Psywave,
        Punishment,
        Pursuit,
        QuickAttack,
        Rage,
        RainDance,
        RapidSpin,
        RazorLeaf,
        RazorWind,
        Recover,
        Recycle,
        Reflect,
        Refresh,
        Rest,
        Return,
        Revenge,
        Reversal,
        Roar,
        RoarOfTime,
        RockBlast,
        RockClimb,
        RockPolish,
        RockSlide,
        RockSmash,
        RockThrow,
        RockTomb,
        RockWrecker,
        RolePlay,
        RollingKick,
        Rollout,
        Roost,
        SacredFire,
        Safeguard,
        SandTomb,
        Sand_Attack,
        Sandstorm,
        ScaryFace,
        Scratch,
        Screech,
        SecretPower,
        SeedBomb,
        SeedFlare,
        SeismicToss,
        Selfdestruct,
        ShadowBall,
        ShadowClaw,
        ShadowForce,
        ShadowPunch,
        ShadowSneak,
        Sharpen,
        SheerCold,
        ShockWave,
        SignalBeam,
        SilverWind,
        Sing,
        Sketch,
        SkillSwap,
        SkullBash,
        SkyAttack,
        SkyUppercut,
        SlackOff,
        Slam,
        Slash,
        SleepPowder,
        SleepTalk,
        Sludge,
        SludgeBomb,
        SmellingSalt,
        Smog,
        SmokeScreen,
        Snatch,
        Snore,
        Softboiled,
        SolarBeam,
        SonicBoom,
        SpacialRend,
        Spark,
        SpiderWeb,
        SpikeCannon,
        Spikes,
        SpitUp,
        Spite,
        Splash,
        Spore,
        StealthRock,
        SteelWing,
        Stockpile,
        Stomp,
        StoneEdge,
        Strength,
        StringShot,
        Struggle,
        StunSpore,
        Submission,
        Substitute,
        SuckerPunch,
        SunnyDay,
        SuperFang,
        Superpower,
        Supersonic,
        Surf,
        Swagger,
        Swallow,
        SweetKiss,
        SweetScent,
        Swift,
        Switcheroo,
        SwordsDance,
        Synthesis,
        Tackle,
        TailGlow,
        TailWhip,
        Tailwind,
        TakeDown,
        Taunt,
        TeeterDance,
        Teleport,
        Thief,
        Thrash,
        Thunder,
        ThunderFang,
        ThunderWave,
        Thunderbolt,
        ThunderPunch,
        ThunderShock,
        Tickle,
        Torment,
        Toxic,
        ToxicSpikes,
        Transform,
        TriAttack,
        Trick,
        TrickRoom,
        TripleKick,
        TrumpCard,
        Twineedle,
        Twister,
        U_turn,
        Uproar,
        VacuumWave,
        ViceGrip,
        VineWhip,
        VitalThrow,
        VoltTackle,
        Wake_UpSlap,
        WaterGun,
        WaterPulse,
        WaterSport,
        WaterSpout,
        Waterfall,
        WeatherBall,
        Whirlpool,
        Whirlwind,
        Will_O_Wisp,
        WingAttack,
        Wish,
        Withdraw,
        WoodHammer,
        WorrySeed,
        Wrap,
        WringOut,
        X_Scissor,
        Yawn,
        ZapCannon,
        ZenHeadbutt
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

namespace Ability
{
    enum Name
    {
        NoAbility,
        Adaptability,
        Aftermath,
        AirLock,
        AngerPoint,
        Anticipation,
        ArenaTrap,
        BadDreams,
        BattleArmor,
        Blaze,
        Chlorophyll,
        ClearBody,
        CloudNine,
        ColorChange,
        Compoundeyes,
        CuteCharm,
        Damp,
        Download,
        Drizzle,
        Drought,
        DrySkin,
        EarlyBird,
        EffectSpore,
        Filter,
        FlameBody,
        FlashFire,
        FlowerGift,
        Forecast,
        Forewarn,
        Frisk,
        Gluttony,
        Guts,
        Heatproof,
        HoneyGather,
        HugePower,
        Hustle,
        Hydration,
        HyperCutter,
        IceBody,
        Illuminate,
        Immunity,
        InnerFocus,
        Insomnia,
        Intimidate,
        IronFist,
        KeenEye,
        Klutz,
        LeafGuard,
        Levitate,
        Lightningrod,
        Limber,
        LiquidOoze,
        MagicGuard,
        MagmaArmor,
        MagnetPull,
        MarvelScale,
        Minus,
        MoldBreaker,
        MotorDrive,
        Multitype,
        NaturalCure,
        NoGuard,
        Normalize,
        Oblivious,
        Overgrow,
        OwnTempo,
        Pickup,
        Plus,
        PoisonHeal,
        PoisonPoint,
        Pressure,
        PurePower,
        QuickFeet,
        RainDish,
        Reckless,
        Rivalry,
        RockHead,
        RoughSkin,
        RunAway,
        SandStream,
        SandVeil,
        Scrappy,
        SereneGrace,
        ShadowTag,
        ShedSkin,
        ShellArmor,
        ShieldDust,
        Simple,
        SkillLink,
        SlowStart,
        Sniper,
        SnowCloak,
        SnowWarning,
        SolarPower,
        SolidRock,
        Soundproof,
        SpeedBoost,
        Stall,
        Static,
        Steadfast,
        Stench,
        StickyHold,
        StormDrain,
        Sturdy,
        SuctionCups,
        SuperLuck,
        Swarm,
        SwiftSwim,
        Synchronize,
        TangledFeet,
        Technician,
        ThickFat,
        TintedLens,
        Torrent,
        Trace,
        Truant,
        Unaware,
        Unburden,
        VitalSpirit,
        VoltAbsorb,
        WaterAbsorb,
        WaterVeil,
        WhiteSmoke,
        WonderGuard
    };
}

namespace Item
{
    enum Name
    {
        NoItem,
        BigRoot,
        BlueScarf,
        BrightPowder,
        ChoiceBand,
        ChoiceScarf,
        ChoiceSpecs,
        DestinyKnot,
        ExpertBelt,
        FocusBand,
        FocusSash,
        FullIncense,
        GreenScarf,
        LaggingTail,
        LaxIncense,
        Leftovers,
        LuckIncense,
        MentalHerb,
        MetalPowder,
        MuscleBand,
        OddIncense,
        PinkScarf,
        PowerHerb,
        PureIncense,
        QuickPowder,
        ReaperCloth,
        RedScarf,
        RockIncense,
        RoseIncense,
        SeaIncense,
        ShedShell,
        SilkScarf,
        SilverPowder,
        SmoothRock,
        SoftSand,
        SootheBell,
        WaveIncense,
        WhiteHerb,
        WideLens,
        WiseGlasses,
        YellowScarf,
        ZoomLens,
        AmuletCoin,
        Antidote,
        Awakening,
        BerryJuice,
        BigPearl,
        BigMushroom,
        BlackBelt,
        BlackFlute,
        BlackSludge,
        BlackGlasses,
        BlueFlute,
        BlueShard,
        BurnHeal,
        Calcium,
        Carbos,
        Charcoal,
        CleanseTag,
        DampMulch,
        DeepSeaScale,
        DireHit,
        DragonScale,
        Elixir,
        EnergyRoot,
        EnergyPowder,
        EscapeRope,
        Ether,
        Everstone,
        ExpShare,
        FireStone,
        FlameOrb,
        FluffyTail,
        FreshWater,
        FullHeal,
        FullRestore,
        GooeyMulch,
        GreenShard,
        GrowthMulch,
        GuardSpec,
        HealPowder,
        HeartScale,
        Honey,
        HPUp,
        HyperPotion,
        IceHeal,
        Iron,
        KingsRock,
        LavaCookie,
        LeafStone,
        Lemonade,
        LifeOrb,
        LightBall,
        LightClay,
        LuckyEgg,
        Magnet,
        MaxElixir,
        MaxEther,
        MaxPotion,
        MaxRepel,
        MaxRevive,
        MetalCoat,
        Metronome,
        MiracleSeed,
        MooMooMilk,
        MoonStone,
        MysticWater,
        NeverMeltIce,
        Nugget,
        OldGateau,
        ParlyzHeal,
        Pearl,
        PokeDoll,
        Potion,
        PPMax,
        PPUp,
        Protein,
        RareCandy,
        RazorFang,
        RedFlute,
        RedShard,
        Repel,
        RevivalHerb,
        Revive,
        SacredAsh,
        ScopeLens,
        ShellBell,
        ShoalSalt,
        ShoalShell,
        SmokeBall,
        SodaPop,
        SoulDew,
        SpellTag,
        StableMulch,
        StarPiece,
        Stardust,
        SunStone,
        SuperPotion,
        SuperRepel,
        Thunderstone,
        TinyMushroom,
        ToxicOrb,
        TwistedSpoon,
        UpGrade,
        WaterStone,
        WhiteFlute,
        XAccuracy,
        XAttack,
        XDefend,
        XSpecial,
        XSpDef,
        XSpeed,
        YellowFlute,
        YellowShard,
        Zinc,
        IcyRock,
        LuckyPunch,
        DubiousDisc,
        SharpBeak,
        AdamantOrb,
        DampRock,
        HeatRock,
        LustrousOrb,
        MachoBrace,
        Stick,
        DragonFang,
        PoisonBarb,
        PowerAnklet,
        PowerBand,
        PowerBelt,
        PowerBracer,
        PowerLens,
        PowerWeight,
        DawnStone,
        DuskStone,
        Electirizer,
        Magmarizer,
        OddKeystone,
        OvalStone,
        Protector,
        QuickClaw,
        RazorClaw,
        ShinyStone,
        StickyBarb,
        DeepSeaTooth,
        DracoPlate,
        DreadPlate,
        EarthPlate,
        FistPlate,
        FlamePlate,
        GripClaw,
        IciclePlate,
        InsectPlate,
        IronPlate,
        MeadowPlate,
        MindPlate,
        SkyPlate,
        SplashPlate,
        SpookyPlate,
        StonePlate,
        ThickClub,
        ToxicPlate,
        ZapPlate,
        ArmorFossil,
        ClawFossil,
        DomeFossil,
        HardStone,
        HelixFossil,
        OldAmber,
        RareBone,
        RootFossil,
        SkullFossil,
        IronBall,
        GriseousOrb
    };
}

enum Stat
{
    Hp = 0,
    Attack,
    Defense,
    Speed,
    SpAttack,
    SpDefense,
    Evasion,
    Accuracy
};

struct AbilityGroup {
    quint16 ab1;
    quint16 ab2;

    AbilityGroup() {
        ab1 = 0;
        ab2 = 0;
    }
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

/* Data that every pokemon of the same specy share. */
class PokeGeneral
{
    PROPERTY(Pokemon::uniqueId, num);
    PROPERTY(quint8, gen);
protected:
    PokeBaseStats m_stats;
    QSet<int> m_moves;
    AbilityGroup m_abilities;
    int m_types[2];
    int m_genderAvail;

    void loadMoves();
    void loadTypes();
    void loadAbilities();
    void loadGenderAvail();
public:
    PokeGeneral();

    const AbilityGroup &abilities() const;
    int genderAvail() const;

    const QSet<int>& moves() const;

    /* loads using num() */
    void load();
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
    PROPERTY(quint8, gen);
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
    /* Removes / Reset things if they are wrong */
    void runCheck();

    void setMove(int moveNum, int moveSlot, bool check=false) throw (QString);
    int addMove(int moveNum, bool check = false) throw (QString);

    bool hasMove(int moveNum);

    quint8 DV(int stat) const;
    void setDV(int stat, quint8 DV);

    quint8 EV(int stat) const;
    int EVSum() const;

    void setEV(int stat, quint8 EV);
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
    void setGen(int gen);
    Pokemon::uniqueId num() const;
    int gen() const;

    void load(int gender, bool shiny);
    void loadIcon(const Pokemon::uniqueId &pokeid);
protected:
    /* This is the current implementation, but an implemenation where more than one
       image is stored can do to */
    QPixmap m_picture;
    QIcon m_icon;
    Pokemon::uniqueId m_num;
    int m_storedgender;
    int m_gen;

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
    void setGen(int gen);
    int gen() const;
    void runCheck();

    int stat(int statno) const;

    /* load various data from the pokenum */
    void load();
    /* display automatically the right picture */
    QPixmap picture();
    QIcon icon();

    void loadFromXml(const QDomElement &el);
    QDomElement & toXml(QDomElement &dest) const;
};

class Team
{
protected:
    PokeTeam m_pokes[6];
    quint8 m_gen;

public:
    Team();
    quint8 gen() const {return m_gen;}
    void setGen(int gen);

    const PokeTeam & poke(int index) const {return m_pokes[index];}
    PokeTeam & poke(int index) {return m_pokes[index];}
};

class TrainerTeam
{
    PROPERTY(quint16, avatar);
    PROPERTY(QString, defaultTier);
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

    const QString & trainerInfo() const;
    const QString & trainerWin() const;
    const QString & trainerLose() const;
    const QString & trainerNick() const;

    bool loadFromFile(const QString &path);
    bool saveToFile(const QString &path) const;
    bool importFromTxt(const QString &path);
    QString exportToTxt() const;
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


QDataStream & operator << (QDataStream & out,const PokePersonal & Pokemon);
QDataStream & operator >> (QDataStream & in,PokePersonal & Pokemon);

inline uint qHash(const Pokemon::uniqueId &key)
{
    return qHash(key.toPokeRef());
}

QDataStream & operator << (QDataStream &out, const Pokemon::uniqueId &id);
QDataStream & operator >> (QDataStream &in, Pokemon::uniqueId &id);

#endif // POKEMONSTRUCTS_H
