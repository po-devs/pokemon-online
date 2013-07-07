#ifndef ENUMS_H
#define ENUMS_H

// Minimum Generation supported.
static const int GEN_MIN = 1;
static const int GEN_MIN_ITEMS = 2;
static const int GEN_MIN_ABILITIES = 3;
// Number of gens

#include <numeric>

namespace Gen
{
enum {
    RedBlue=1, Yellow=1+(1<<8), Stadium=1+(2<<8), StadiumWithTradebacks=(1+(3<<8)),
    GoldSilver=2, Crystal=2+(1<<8), Stadium2=2+(2<<8),
    RubySapphiry=3, Colosseum=3+(1<<8),RFLG=3+(2<<8),Emerald=3+(3<<8), XD=3+(4<<8),
    DiamondPearl=4, Platinum=4+(1<<8), HGSS=4+(2<<8),
    BlackWhite=5, BlackWhite2=5+(1<<8)
};

}


namespace Version
{
    enum {
        Platinum = 14,
        SoulSilver = 15,
        HeartGold = 16
    };

    enum {
        NumberOfGens = 5
    };

    static const int avatarSize[] = {
        56,
        60,
        64,
        80,
        96
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

    enum Status {
        Fine = 0,
        Paralysed = 1,
        Asleep = 2,
        Frozen = 3,
        Burnt = 4,
        Poisoned = 5,
        Confused = 6,
        Attracted = 7,
        Wrapped = 8,
        Nightmared = 9,
        Tormented = 12,
        Disabled = 13,
        Drowsy = 14,
        HealBlocked = 15,
        Sleuthed = 17,
        Seeded = 18,
        Embargoed = 19,
        Requiemed = 20,
        Rooted = 21,
        Koed = 31
    };

    enum StatusKind
    {
        NoKind = 0,
        SimpleKind = 1,
        TurnKind = 2,
        AttractKind = 3,
        WrapKind = 4
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
        Victini,
        Snivy,
        Servine,
        Serperior,
        Tepig,
        Pignite,
        Emboar,
        Oshawott,
        Dewott,
        Sammurott,
        Patrat,
        Watchog,
        Lillipup,
        Herdier,
        Stoutland,
        Purrloin,
        Liepard,
        Pansage,
        Simisage,
        Pansear,
        Simisear,
        Panpour,
        Simipour,
        Munna,
        Musharna,
        Pidove,
        Tranquill,
        Unfezant,
        Blitzle,
        Zebstrika,
        Roggenrola,
        Boldore,
        Gigalith,
        Woobat,
        Swoobat,
        Drilbur,
        Excadrill,
        Audino,
        Timburr,
        Gurdurr,
        Conkeldurr,
        Tympole,
        Palpitoad,
        Seismitoad,
        Throh,
        Sawk,
        Sewaddle,
        Swadloon,
        Leavanny,
        Venipede,
        Whirlipede,
        Scolipede,
        Cottonee,
        Whimsicott,
        Petilil,
        Liligant,
        Basculin,
        Sandile,
        Krokorok,
        Krookodile,
        Darumaka,
        Darmanitan,
        Maractus,
        Dwebble,
        Crustle,
        Scraggy,
        Scrafty,
        Sigilyph,
        Yamask,
        Cofagrigus,
        Tirtouga,
        Carracosta,
        Archen,
        Archeops,
        Trubbish,
        Garbodor,
        Zorua,
        Zoroark,
        Minccino,
        Cinccino,
        Gothita,
        Gothorita,
        Gothitelle,
        Solosis,
        Duosion,
        Reuniclus,
        Ducklett,
        Swanna,
        Vanillite,
        Vanillish,
        Vanilluxe,
        Deerling,
        Sawsbuck,
        Emolga,
        Karrablast,
        Escavalier,
        Foongus,
        Amoonguss,
        Frillish,
        Jellicent,
        Alomomola,
        Joltik,
        Galvantula,
        Ferroseed,
        Ferrotorn,
        Klink,
        Klang,
        Klinkklang,
        Tynamo,
        Eelektrik,
        Eelektross,
        Elgyem,
        Beheeyem,
        Litwick,
        Lampent,
        Chandelure,
        Axew,
        Fraxure,
        Haxorus,
        Cubchoo,
        Beartic,
        Cryogonal,
        Shelmet,
        Accelgor,
        Stunfisk,
        Mienfoo,
        Mienshao,
        Druddigon,
        Golett,
        Golurk,
        Pawniard,
        Bisharp,
        Bouffalant,
        Rufflet,
        Braviary,
        Vullaby,
        Mandibuzz,
        Heatmor,
        Durant,
        Deino,
        Zweilous,
        Hydreigon,
        Larvesta,
        Volcarona,
        Cobalion,
        Terrakion,
        Virizion,
        Tornadus,
        Thundurus,
        Reshiram,
        Zekrom,
        Landorus,
        Kyurem,
        Keldeo,
        Meloetta,
        Genesect,
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
        Shaymin_S = Shaymin + (1 << 16),
        Meloetta_P = Meloetta + (1 << 16),
        Thundurus_T = Thundurus + (1 << 16),
        Landorus_T = Landorus + (1 << 16),
        Tornadus_T = Tornadus + (1 << 16),
        Darmanitan_Z = Darmanitan + (1 << 16),
        SpikyPichu = Pichu + (1 << 16),
        Kyurem_W = Kyurem + (1 << 16),
        Kyurem_B = Kyurem + (2 << 16),
        Keldeo_R = Keldeo + (1 << 16),
        Genesect_D = Genesect + (1 << 16),
        Genesect_S = Genesect + (2 << 16),
        Genesect_B = Genesect + (3 << 16),
        Genesect_C = Genesect + (4 << 16)
    };
}

namespace Move
{
    enum Category
    {
        Other,
        Physical,
        Special
    };

    enum Flags
    {
        ContactFlag = 1, // Is the move a contact move
        ChargeFlag = 2, // Is the move a charging move? not used by PO yet
        RechargeFlag = 4, // Is the move a recharging move? not used by PO yet
        ProtectableFlag = 8, //Can the move be protected against
        MagicCoatableFlag = 16, //Can the move be magic coated
        SnatchableFlag = 32, //Can the move be snatched
        MemorableFlag = 64, //Can the move be mirror moves
        PunchFlag = 128, //Is the move boosted with Iron Fist
        SoundFlag = 256, //Is the move blocked with SoundProof
        FlyingFlag = 512, //Is the move an invulnerable move (shadow force/...)? not used by PO yet
        UnthawingFlag = 1024, // Does the user of this move unthaw when frozen?
        PulsingFlag = 2048, // Can this move reach targets far across in triples?
        HealingFlag = 4096, //Can this move be blocked with Heal Block
        MischievousFlag = 8192// Can this move bypass substitute?
    };

    enum Target
    {
        ChosenTarget = 0,
        PartnerOrUser = 1,
        Partner = 2,
        MeFirstTarget = 3,
        AllButSelf = 4,
        Opponents = 5,
        TeamParty = 6,
        User = 7,
        All = 8,
        RandomTarget = 9,
        Field = 10,
        OpposingTeam = 11,
        TeamSide = 12,
        IndeterminateTarget = 13
    };

    enum Classification
    {
        StandardMove = 0,
        StatusInducingMove = 1,
        StatChangingMove = 2,
        HealingMove = 3,
        OffensiveStatusInducingMove = 4,
        StatAndStatusMove = 5,
        OffensiveStatChangingMove = 6,
        OffensiveSelfStatChangingMove = 7,
        AbsorbingMove = 8,
        OHKOMove = 9,
        FieldMove = 10,
        TeamZoneMove = 11,
        RoaringMove = 12,
        SpecialMove = 13
    };

    enum Name
    {
        NoMove,
        Pound,
        KarateChop,
        DoubleSlap,
        CometPunch,
        MegaPunch,
        PayDay,
        FirePunch,
        IcePunch,
        ThunderPunch,
        Scratch,
        ViceGrip,
        Guillotine,
        RazorWind,
        SwordsDance,
        Cut,
        Gust,
        WingAttack,
        Whirlwind,
        Fly,
        Bind,
        Slam,
        VineWhip,
        Stomp,
        DoubleKick,
        MegaKick,
        JumpKick,
        RollingKick,
        Sand_Attack,
        Headbutt,
        HornAttack,
        FuryAttack,
        HornDrill,
        Tackle,
        BodySlam,
        Wrap,
        TakeDown,
        Thrash,
        Double_Edge,
        TailWhip,
        PoisonSting,
        Twineedle,
        PinMissile,
        Leer,
        Bite,
        Growl,
        Roar,
        Sing,
        Supersonic,
        SonicBoom,
        Disable,
        Acid,
        Ember,
        Flamethrower,
        Mist,
        WaterGun,
        HydroPump,
        Surf,
        IceBeam,
        Blizzard,
        Psybeam,
        BubbleBeam,
        AuroraBeam,
        HyperBeam,
        Peck,
        DrillPeck,
        Submission,
        LowKick,
        Counter,
        SeismicToss,
        Strength,
        Absorb,
        MegaDrain,
        LeechSeed,
        Growth,
        RazorLeaf,
        SolarBeam,
        PoisonPowder,
        StunSpore,
        SleepPowder,
        PetalDance,
        StringShot,
        DragonRage,
        FireSpin,
        ThunderShock,
        Thunderbolt,
        ThunderWave,
        Thunder,
        RockThrow,
        Earthquake,
        Fissure,
        Dig,
        Toxic,
        Confusion,
        Psychic,
        Hypnosis,
        Meditate,
        Agility,
        QuickAttack,
        Rage,
        Teleport,
        NightShade,
        Mimic,
        Screech,
        DoubleTeam,
        Recover,
        Harden,
        Minimize,
        SmokeScreen,
        ConfuseRay,
        Withdraw,
        DefenseCurl,
        Barrier,
        LightScreen,
        Haze,
        Reflect,
        FocusEnergy,
        Bide,
        Metronome,
        MirrorMove,
        Selfdestruct,
        EggBomb,
        Lick,
        Smog,
        Sludge,
        BoneClub,
        FireBlast,
        Waterfall,
        Clamp,
        Swift,
        SkullBash,
        SpikeCannon,
        Constrict,
        Amnesia,
        Kinesis,
        Softboiled,
        HiJumpKick,
        Glare,
        DreamEater,
        PoisonGas,
        Barrage,
        LeechLife,
        LovelyKiss,
        SkyAttack,
        Transform,
        Bubble,
        DizzyPunch,
        Spore,
        Flash,
        Psywave,
        Splash,
        AcidArmor,
        Crabhammer,
        Explosion,
        FurySwipes,
        Bonemerang,
        Rest,
        RockSlide,
        HyperFang,
        Sharpen,
        Conversion,
        TriAttack,
        SuperFang,
        Slash,
        Substitute,
        Struggle,
        Sketch,
        TripleKick,
        Thief,
        SpiderWeb,
        MindReader,
        Nightmare,
        FlameWheel,
        Snore,
        Curse,
        Flail,
        Conversion2,
        Aeroblast,
        CottonSpore,
        Reversal,
        Spite,
        PowderSnow,
        Protect,
        MachPunch,
        ScaryFace,
        FaintAttack,
        SweetKiss,
        BellyDrum,
        SludgeBomb,
        Mud_Slap,
        Octazooka,
        Spikes,
        ZapCannon,
        Foresight,
        DestinyBond,
        PerishSong,
        IcyWind,
        Detect,
        BoneRush,
        Lock_On,
        Outrage,
        Sandstorm,
        GigaDrain,
        Endure,
        Charm,
        Rollout,
        FalseSwipe,
        Swagger,
        MilkDrink,
        Spark,
        FuryCutter,
        SteelWing,
        MeanLook,
        Attract,
        SleepTalk,
        HealBell,
        Return,
        Present,
        Frustration,
        Safeguard,
        PainSplit,
        SacredFire,
        Magnitude,
        DynamicPunch,
        Megahorn,
        DragonBreath,
        BatonPass,
        Encore,
        Pursuit,
        RapidSpin,
        SweetScent,
        IronTail,
        MetalClaw,
        VitalThrow,
        MorningSun,
        Synthesis,
        Moonlight,
        HiddenPower,
        CrossChop,
        Twister,
        RainDance,
        SunnyDay,
        Crunch,
        MirrorCoat,
        PsychUp,
        ExtremeSpeed,
        AncientPower,
        ShadowBall,
        FutureSight,
        RockSmash,
        Whirlpool,
        BeatUp,
        FakeOut,
        Uproar,
        Stockpile,
        SpitUp,
        Swallow,
        HeatWave,
        Hail,
        Torment,
        Flatter,
        Will_O_Wisp,
        Memento,
        Facade,
        FocusPunch,
        SmellingSalt,
        FollowMe,
        NaturePower,
        Charge,
        Taunt,
        HelpingHand,
        Trick,
        RolePlay,
        Wish,
        Assist,
        Ingrain,
        Superpower,
        MagicCoat,
        Recycle,
        Revenge,
        BrickBreak,
        Yawn,
        KnockOff,
        Endeavor,
        Eruption,
        SkillSwap,
        Imprison,
        Refresh,
        Grudge,
        Snatch,
        SecretPower,
        Dive,
        ArmThrust,
        Camouflage,
        TailGlow,
        LusterPurge,
        MistBall,
        FeatherDance,
        TeeterDance,
        BlazeKick,
        MudSport,
        IceBall,
        NeedleArm,
        SlackOff,
        HyperVoice,
        PoisonFang,
        CrushClaw,
        BlastBurn,
        HydroCannon,
        MeteorMash,
        Astonish,
        WeatherBall,
        Aromatherapy,
        FakeTears,
        AirCutter,
        Overheat,
        OdorSleuth,
        RockTomb,
        SilverWind,
        MetalSound,
        GrassWhistle,
        Tickle,
        CosmicPower,
        WaterSpout,
        SignalBeam,
        ShadowPunch,
        Extrasensory,
        SkyUppercut,
        SandTomb,
        SheerCold,
        MuddyWater,
        BulletSeed,
        AerialAce,
        IcicleSpear,
        IronDefense,
        Block,
        Howl,
        DragonClaw,
        FrenzyPlant,
        BulkUp,
        Bounce,
        MudShot,
        PoisonTail,
        Covet,
        VoltTackle,
        MagicalLeaf,
        WaterSport,
        CalmMind,
        LeafBlade,
        DragonDance,
        RockBlast,
        ShockWave,
        WaterPulse,
        DoomDesire,
        PsychoBoost,
        Roost,
        Gravity,
        MiracleEye,
        Wake_UpSlap,
        HammerArm,
        GyroBall,
        HealingWish,
        Brine,
        NaturalGift,
        Feint,
        Pluck,
        Tailwind,
        Acupressure,
        MetalBurst,
        U_turn,
        CloseCombat,
        Payback,
        Assurance,
        Embargo,
        Fling,
        PsychoShift,
        TrumpCard,
        HealBlock,
        WringOut,
        PowerTrick,
        GastroAcid,
        LuckyChant,
        MeFirst,
        Copycat,
        PowerSwap,
        GuardSwap,
        Punishment,
        LastResort,
        WorrySeed,
        SuckerPunch,
        ToxicSpikes,
        HeartSwap,
        AquaRing,
        MagnetRise,
        FlareBlitz,
        ForcePalm,
        AuraSphere,
        RockPolish,
        PoisonJab,
        DarkPulse,
        NightSlash,
        AquaTail,
        SeedBomb,
        AirSlash,
        X_Scissor,
        BugBuzz,
        DragonPulse,
        DragonRush,
        PowerGem,
        DrainPunch,
        VacuumWave,
        FocusBlast,
        EnergyBall,
        BraveBird,
        EarthPower,
        Switcheroo,
        GigaImpact,
        NastyPlot,
        BulletPunch,
        Avalanche,
        IceShard,
        ShadowClaw,
        ThunderFang,
        IceFang,
        FireFang,
        ShadowSneak,
        MudBomb,
        PsychoCut,
        ZenHeadbutt,
        MirrorShot,
        FlashCannon,
        RockClimb,
        Defog,
        TrickRoom,
        DracoMeteor,
        Discharge,
        LavaPlume,
        LeafStorm,
        PowerWhip,
        RockWrecker,
        CrossPoison,
        GunkShot,
        IronHead,
        MagnetBomb,
        StoneEdge,
        Captivate,
        StealthRock,
        GrassKnot,
        Chatter,
        Judgment,
        BugBite,
        ChargeBeam,
        WoodHammer,
        AquaJet,
        AttackOrder,
        DefendOrder,
        HealOrder,
        HeadSmash,
        DoubleHit,
        RoarofTime,
        SpacialRend,
        LunarDance,
        CrushGrip,
        MagmaStorm,
        DarkVoid,
        SeedFlare,
        OminousWind,
        ShadowForce,
        HoneClaws,
        WideGuard,
        GuardSplit,
        PowerSplit,
        WonderRoom,
        Psyshock,
        Venoshock,
        Autotomize,
        RagePowder,
        Telekinesis,
        MagicRoom,
        SmackDown,
        StormThrow,
        FlameBurst,
        SludgeWave,
        QuiverDance,
        HeavySlam,
        Synchronoise,
        ElectroBall,
        Soak,
        FlameCharge,
        Coil,
        LowSweep,
        AcidSpray,
        FoulPlay,
        SimpleBeam,
        Entrainment,
        AfterYou,
        Round,
        EchoedVoice,
        ChipAway,
        ClearSmog,
        StoredPower,
        QuickGuard,
        AllySwitch,
        Scald,
        ShellSmash,
        HealPulse,
        Hex,
        SkyDrop,
        ShiftGear,
        CircleThrow,
        Incinerate,
        Quash,
        Acrobatics,
        ReflectType,
        Retaliate,
        FinalGambit,
        Bestow,
        Inferno,
        WaterPledge,
        FirePledge,
        GrassPledge,
        VoltSwitch,
        StruggleBug,
        Bulldoze,
        FrostBreath,
        DragonTail,
        WorkUp,
        Electroweb,
        WildCharge,
        DrillRun,
        DualChop,
        HeartStamp,
        HornLeech,
        SacredSword,
        RazorShell,
        HeatCrash,
        LeafTornado,
        Steamroller,
        CottonGuard,
        NightDaze,
        Psystrike,
        TailSlap,
        Hurricane,
        HeadCharge,
        GearGrind,
        SoaringShot,
        TechnoBlast,
        RelicSong,
        SecretSword,
        Glaciate,
        BoltStrike,
        BlueFire,
        FieryDance,
        FreezeShock,
        IceBurn,
        Snarl,
        IcicleCrash,
        VCreate,
        FusionFlare,
        FusionBolt
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
        Stench,
        Drizzle,
        SpeedBoost,
        BattleArmor,
        Sturdy,
        Damp,
        Limber,
        SandVeil,
        Static,
        VoltAbsorb,
        WaterAbsorb,
        Oblivious,
        CloudNine,
        Compoundeyes,
        Insomnia,
        ColorChange,
        Immunity,
        FlashFire,
        ShieldDust,
        OwnTempo,
        SuctionCups,
        Intimidate,
        ShadowTag,
        RoughSkin,
        WonderGuard,
        Levitate,
        EffectSpore,
        Synchronize,
        ClearBody,
        NaturalCure,
        Lightningrod,
        SereneGrace,
        SwiftSwim,
        Chlorophyll,
        Illuminate,
        Trace,
        HugePower,
        PoisonPoint,
        InnerFocus,
        MagmaArmor,
        WaterVeil,
        MagnetPull,
        Soundproof,
        RainDish,
        SandStream,
        Pressure,
        ThickFat,
        EarlyBird,
        FlameBody,
        RunAway,
        KeenEye,
        HyperCutter,
        Pickup,
        Truant,
        Hustle,
        CuteCharm,
        Plus,
        Minus,
        Forecast,
        StickyHold,
        ShedSkin,
        Guts,
        MarvelScale,
        LiquidOoze,
        Overgrow,
        Blaze,
        Torrent,
        Swarm,
        RockHead,
        Drought,
        ArenaTrap,
        VitalSpirit,
        WhiteSmoke,
        PurePower,
        ShellArmor,
        AirLock,
        TangledFeet,
        MotorDrive,
        Rivalry,
        Steadfast,
        SnowCloak,
        Gluttony,
        AngerPoint,
        Unburden,
        Heatproof,
        Simple,
        DrySkin,
        Download,
        IronFist,
        PoisonHeal,
        Adaptability,
        SkillLink,
        Hydration,
        SolarPower,
        QuickFeet,
        Normalize,
        Sniper,
        MagicGuard,
        NoGuard,
        Stall,
        Technician,
        LeafGuard,
        Klutz,
        MoldBreaker,
        SuperLuck,
        Aftermath,
        Anticipation,
        Forewarn,
        Unaware,
        TintedLens,
        Filter,
        SlowStart,
        Scrappy,
        StormDrain,
        IceBody,
        SolidRock,
        SnowWarning,
        HoneyGather,
        Frisk,
        Reckless,
        Multitype,
        FlowerGift,
        BadDreams,
        Pickpocket,
        SheerForce,
        Contrary,
        Unnerve,
        Defiant,
        Defeatist,
        CursedBody,
        Healer,
        FriendGuard,
        WeakArmor,
        HeavyMetal,
        LightMetal,
        MultiScale,
        ToxicBoost,
        FlareBoost,
        Harvest,
        Telepathy,
        Moody,
        Overcoat,
        PoisonTouch,
        Regenerator,
        BigPecks,
        SandRush,
        MiracleSkin,
        Analytic,
        Illusion,
        Imposter,
        Infiltrator,
        Mummy,
        Moxie,
        Justified,
        Rattled,
        MagicBounce,
        SapSipper,
        Prankster,
        SandForce,
        IronBarbs,
        ZenMode,
        VictoryStar,
        TurboBlaze,
        TeraVolt
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
        GriseousOrb,
        AirMail,
        BloomMail,
        BrickMail,
        BubbleMail,
        FlameMail,
        GrassMail,
        HarborMail,
        HeartMail,
        MosaicMail,
        SnowMail,
        SpaceMail,
        SteelMail,
        TunnelMail,
        DouseDrive,
        ShockDrive,
        BurnDrive,
        ChillDrive,
        SweetHeart,
        PrismScale,
        Eviolite,
        FloatStone,
        RockyHelmet,
        AirBalloon,
        RedCard,
        RingTarget,
        BindingBand,
        AbsorbBulb,
        CellBattery,
        EscapeButton,
        FireGem,
        WaterGem,
        ElectricGem,
        GrassGem,
        IceGem,
        FightGem,
        PoisonGem,
        EarthGem,
        FlightGem,
        PsychicGem,
        BugGem,
        RockGem,
        GhostGem,
        DragonGem,
        DarkGem,
        SteelGem,
        NormalGem,
        StrengthWing,
        PowerWing,
        ResistanceWing,
        WisdomWing,
        MindWing,
        InstantWing,
        BeautifulWing,
        DreamBall,
        FragrantMushroom,
        HugeGoldOrb,
        RoundPearl,
        CometPiece,
        AncientBronzeCoin,
        AncientSilverCoin,
        AncientGoldCoin,
        AncientVase,
        AncientBracelet,
        AncientStatue,
        AncientCrown,
        HiunIceCream,
        DireHit2,
        XSpeed2,
        XSpecial2,
        XSpDef2,
        XDefend2,
        XAttack2,
        XAccuracy2,
        XSpeed3,
        XSpecial3,
        XSpDef3,
        XDefend3,
        XAttack3,
        XAccuracy3,
        XSpeed6,
        XSpecial6,
        XSpDef6,
        XDefend6,
        XAttack6,
        XAccuracy6,
        SkillCall,
        ItemDrop,
        ItemCall,
        FlatCall,
        DireHit3,
        BerserkGene
    };

    enum Target
    {
        Attack = 0, /* Targets an attack (Ether, Max Ether) */
        FieldPokemon = 1, /* Targets a pokemon on the field (X Attack, ...) */
        TeamPokemon = 2, /* Targets a pokemon in your team (potion, berries, ...) */
        Team = 3, /* Sacred Ash! */
        Opponent = 4, /* Pokeballs */
        NoTarget = 13
    };
}

enum Stat
{
    Hp = 0,
    Attack = 1,
    Defense = 2,
    SpAttack = 3,
    SpDefense = 4,
    Speed = 5,
    Accuracy = 6,
    Evasion = 7,
    AllStats = 8
};

/* Could be enum class weather in C++0x */
namespace Weather {
    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };
}

#endif // ENUMS_H
