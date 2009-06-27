#ifndef TEAM_STRUCT_H_INCLUDED
#define TEAM_STRUCT_H_INCLUDED

#include <SDL/SDL.h>
#include <string>
#include "utilities.hh"
#include "intervideo.hpp"

using namespace interface;

enum Switch
{
    Trainer = 0,
    Pokemons = 1
};
enum TYPE
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
enum CATEGORY
{
    Physical = 0,
    Special = 1,
    Other = 2
};
enum NATURE
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

//contient toutes les infos sur l'équipe
struct Team
{
    //les infos des pokes
    struct Pokes
    {
        Pokes();
        void reset();
        Uint16 num; // 9 bits
        Uint8 nature; //5 bits
        bool gender;
        bool shiney;
        Uint16 moves[4]; //9 bits chacun
        std::string nick;
        Uint16 item; //9 bits
        Uint8 level; //7 bits
        Uint8 hp_ev;
        Uint8 att_ev;
        Uint8 def_ev;
        Uint8 satt_ev;
        Uint8 sdef_ev;
        Uint8 speed_ev;
        Uint8 hp_dv;//5 bits
        Uint8 att_dv;//5 bits
        Uint8 def_dv;//5 bits
        Uint8 satt_dv;//5 bits
        Uint8 sdef_dv;//5 bits
        Uint8 speed_dv;//5 bits
        bool ability;
    };

    Pokes pokes[6];
    //celles du trainer
    std::string Trainer_Name;
    std::string Trainer_Info;
    std::string Trainer_Win;
    std::string Trainer_Lose;

    Team();
    smart_ptr< fast_array<char> > convertToBits() const;
    void convertFromBits(const char* bits, Uint32 size);
    void load(const char *file_name);
    void save(const char *file_name);
};

void serialize(MegaSerializer &bits, const Team &t);
void serialize(MegaSerializer &bits, const Team::Pokes &pok);
void deserialize(MegaDeserializer &bits, Team &t);
void deserialize(MegaDeserializer &bits, Team::Pokes &pok);

struct TeamInfo
{
    //les infos des pokes
    struct Pokes
    {
        Pokes();
        ~Pokes();
        void reset();
		int load_image(int num, bool shinyness, bool gender, int gender_t);
		void load_stats(int num);
        Uint8 base_HP;
        Uint8 base_att;
        Uint8 base_def;
        Uint8 base_satt;
        Uint8 base_sdef;
        Uint8 base_spd;
        Uint8 gender_t; //male: 0x01, female: 0x02
        Surface avatar;
		Surface mini_avatar;
    };

    Pokes pokes[6];
};

#endif // TEAM_STRUCT_H_INCLUDED
