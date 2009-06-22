#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include <fstream>
#include <limits>
#include <string>
#include <sstream>
#include "Team_Struct.h"
#include "MF_applet.hh"
#include "utilities.hh"

//Hérite de l'applet
class Poke_Zone : public MF_BApplet
{
    public:
        Poke_Zone(Font &police, const char *nickname, const char *item, const char *move1, const char *move2, const char *move3, const char *move4, Surface &avatar = Surface(), Sint16 x = 0, Sint16 y = 0);
        ~Poke_Zone(){
            ;
        }

        //pour redessiner tout
        void reset(Font &police, const char *nickname, const char *item, const char *move1, const char *move2, const char *move3, const char *move4, Surface &avatar);
};

class TeamBuilder;

//Menu avancé
class Team_Av :  public MF_BDirections, public MF_Applet, public MF_MoveAbleBoss, public MF_Prio
{
    public:
        MF_ListeDeroulante *hp_dv, *att_dv, *def_dv, *satt_dv, *sdef_dv, *speed_dv;
        MF_ListeDeroulante *hidden_power;
        MF_BarManager *HP_det;
        MF_TButton *reset, *ok;
        MF_BRadio *ab_choice, *gen_choice;
        MF_CheckBox *shinyness;
        TeamBuilder *team;
        vector<string> hp_data;
        Uint16 abilities[2];
        string ability_names[2];
        bool choice;

        Team_Av(TeamBuilder *team);

        bool recoitMessage(const char *message, MF_Base *fenetre);
        void load_abilities();
        void load_gender();
        void resize(Uint16 w, Uint16 h);

        void affiche(Surface &ecran);
        bool gereEvenement (const SDL_Event &event){return MF_MoveAbleBoss::gereEvenement(event);}
        void display_stats();
        void display_base(Uint16 base, Sint8 boost, Sint16 posy);
        void display_HP();
        void display_Att();
        void display_Def();
        void display_Speed();
        void display_SpAtt();
        void display_SpDef();
        void update_hidden_power();
        void set_hidden_power(int value);
        void disp_hp_power();
        void load_hidden_power();
        void remet_DVs();
        void set_DVs(int num);
};

//la classe teambuilder hérite de MF_Boss, qui permet de gérer des fenêtres
class TeamBuilder : public MF_BDirections, public MF_Applet
{
    friend class Team_Av;
    protected:
        //tous les champs de texte
        Poke_Zone *poke_z[6]; //affichage
        MF_BApplet *trainer_z;
        MF_TextBox *nickname;
        MF_ListeDeroulante *nature, *item;
        MF_TButton *advanced, *save, *load, *exit;
        MF_TextList *move[4];
        MF_BarManager *PokemonField, *movepool;
        MF_TrayBar *hp_bar, *att_bar, *def_bar, *satt_bar, *sdef_bar, *speed_bar, *ev_bar;
        //Dresseur
        MF_TextBox *trNick, *trInfo, *trWin, *trLose;

#define POKE_COUNT 494
#define MOVE_COUNT 468
#define ITEMS_COUNT 213
#define BERRY_COUNT 64

        std::string poke_names[POKE_COUNT];
        std::string item_names[ITEMS_COUNT+BERRY_COUNT];
        std::string moves[MOVE_COUNT];
        std::string move_acc[MOVE_COUNT];
        std::string move_pow[MOVE_COUNT];
        TYPE move_type[MOVE_COUNT];
        CATEGORY move_cat[MOVE_COUNT];
        std::string move_pp[MOVE_COUNT];
        //when we load a poke, we need to display its possible moves
        std::string current_move_db[5];
        //Move_Way_Of_Learning
        static const char * move_learning[5];
        static const char * type_name[18];
        static const char * category_name[3];
        static const char * nature_name[25];
        //police utilisée
        Font verdana;
        //equipe (toutes les infos)
        Team &equipe;
        TeamInfo equipe2;
        Uint8 current_zone;
        //Un tableau contenant 001, 002, 003 ...
        char tableau_nums[POKE_COUNT][4];
        //Un tableau contenant les surfaces des types
        Surface move_jpg[18];
        Surface cat_jpg[3];
        //Trainer, Pokemon
        Switch Mode;

    public:
        TeamBuilder(Team &equipe); //création des champs de texte
        ~TeamBuilder(); //libération de tout

        //charge les noms de pokes dans poke_names^^
        void load_pokenames();
        //et les attaques
        void load_moves();
        //et les items
        void load_items();
        //les attaques pour un certain poke, dans un tableau de string[5]
        void load_pokemoves(int pokenum);
        //les genres
        void load_gender(int zone);

        void affiche(Surface &ecran);
        virtual bool recoitMessage(const char *message, MF_Base *fenetre);
        virtual void resize(Uint16 w, Uint16 h)
        {
            MF_Applet::resize(w, h);
        }
        //lorsque l'on veut refaire un poke, on reset tous ses champs
        bool reset_poke(Uint8 zone, unsigned int pokenum);
        void reset_zone(Uint8 zone);
        void actualise_zone(Uint8 zone);
        void actualise_moves(Uint8 zone);
        void change_poke_image(Uint8 zone);
        void reset_move()
        {
            move[0]->reset();
            move[1]->reset();
            move[2]->reset();
            move[3]->reset();
            movepool->clear_options();
        }
        void rewrite_nick(uint8_t zone);
        void insert_in(MF_TextBox *box, unsigned length, string &dest);
        void insert_nickname() {insert_in(nickname, 12, equipe.pokes[current_zone].nick);}
        void insert_trNick() {insert_in(trNick, 15, equipe.Trainer_Name);}
        void insert_trInfo() {insert_in(trInfo, 200, equipe.Trainer_Info);}
        void insert_trWin() {insert_in(trWin, 300, equipe.Trainer_Win);}
        void insert_trLose() {insert_in(trLose, 300, equipe.Trainer_Lose);}
        void insert_trainer() {insert_trNick();insert_trInfo();insert_trLose();insert_trWin();}
        void insert_current() {if (current_zone < 6) insert_nickname(); else insert_trainer();}
        Uint16 calculate_base(Uint8 base_stat, Uint8 level, Uint8 dv, Uint8 ev);
        Sint8 get_boost(Uint8 stat) { return -(equipe.pokes[current_zone].nature%5 == stat) + (equipe.pokes[current_zone].nature/5 == stat);}
        Uint16 calc_HP(Uint8 zone) { return calculate_base(equipe2.pokes[zone].base_HP, equipe.pokes[zone].level, equipe.pokes[zone].hp_dv, equipe.pokes[zone].hp_ev) + equipe.pokes[zone].level + 5; }
        Uint16 calc_Att(Uint8 zone) { return (calculate_base(equipe2.pokes[zone].base_att, equipe.pokes[zone].level, equipe.pokes[zone].att_dv, equipe.pokes[zone].att_ev))*(10+get_boost(0))/10; }
        Uint16 calc_Def(Uint8 zone) { return (calculate_base(equipe2.pokes[zone].base_def, equipe.pokes[zone].level, equipe.pokes[zone].def_dv, equipe.pokes[zone].def_ev))*(10+get_boost(1))/10; }
        Uint16 calc_Speed(Uint8 zone) { return (calculate_base(equipe2.pokes[zone].base_spd, equipe.pokes[zone].level, equipe.pokes[zone].speed_dv, equipe.pokes[zone].speed_ev))*(10+get_boost(2))/10; }
        Uint16 calc_SpAtt(Uint8 zone) { return (calculate_base(equipe2.pokes[zone].base_satt, equipe.pokes[zone].level, equipe.pokes[zone].satt_dv, equipe.pokes[zone].satt_ev))*(10+get_boost(3))/10; }
        Uint16 calc_SpDef(Uint8 zone) { return (calculate_base(equipe2.pokes[zone].base_sdef, equipe.pokes[zone].level, equipe.pokes[zone].sdef_dv, equipe.pokes[zone].sdef_ev))*(10+get_boost(4))/10; }

        void display_stats();
        void display_base(Uint16 base, Uint8 ev, Sint8 boost, Sint16 posy);
        void display_HP(){display_base(calc_HP(current_zone), equipe.pokes[current_zone].hp_ev, 0, 165);}
        void display_Att(){display_base(calc_Att(current_zone), equipe.pokes[current_zone].att_ev, get_boost(0), 195);}
        void display_Def(){display_base(calc_Def(current_zone), equipe.pokes[current_zone].def_ev, get_boost(1), 225);}
        void display_Speed(){display_base(calc_Speed(current_zone), equipe.pokes[current_zone].speed_ev, get_boost(2), 255);}
        void display_SpAtt(){display_base(calc_SpAtt(current_zone), equipe.pokes[current_zone].satt_ev, get_boost(3), 285);}
        void display_SpDef(){display_base(calc_SpDef(current_zone), equipe.pokes[current_zone].sdef_ev, get_boost(4), 315);}
        void display_Evs();
        Uint16 sum_evs(){ return hp_bar->pos_icon+att_bar->pos_icon+def_bar->pos_icon+satt_bar->pos_icon+sdef_bar->pos_icon+speed_bar->pos_icon;}
        void SwitchTo(Switch wanted);
        void charge_image(int zone);
        void display_gender();

        void save_team() {insert_current(); equipe.save("team/team.pcb");}
        void load_team();
        void set_team();
};

#endif
