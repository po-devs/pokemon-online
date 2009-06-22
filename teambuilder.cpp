#include "teambuilder.h"
#include "exception.hpp"

using namespace std;

Team::Pokes::Pokes()
{
	reset();
}

int TeamInfo::Pokes::load_image(int num, bool shinyness, bool gender, int gender_t)
{
    if (num == 0)
    {
        avatar.free();
        mini_avatar.free();
        return 0;
    }
    try {
        string path = padd(3, num) + "DP";

        if (gender == 0 && gender_t != 2)
        {
            path += "m";
        } else
        {
            path += "f";
        }
        if (shinyness)
            path += 's';
        path += ".png";

        smart_ptr< fast_array<char> > rw_data = OpenFile("db/poke_img.db", path.c_str());

        avatar.load(SDL_RWFromMem(*rw_data, rw_data->size), true);

        mini_avatar = Surface(zoomSurface(avatar.s, 0.5, 0.5, 1));
        mini_avatar.colorkey(Color(0xFF, 0xFF, 0xFF));
    } catch (string &ex)
    {
        cout << ex << endl;
    }

	return 0;
}

void TeamInfo::Pokes::load_stats(int pokenum)
{
    ifstream db;

    base_HP = atoi(find_line("db/poke_HP.txt", pokenum)->c_str());
    base_att = atoi(find_line("db/poke_Att.txt", pokenum)->c_str());
    base_def = atoi(find_line("db/poke_Def.txt", pokenum)->c_str());
    base_spd = atoi(find_line("db/poke_Spd.txt", pokenum)->c_str());
    base_satt = atoi(find_line("db/poke_SAtt.txt", pokenum)->c_str());
    base_sdef = atoi(find_line("db/poke_SDef.txt", pokenum)->c_str());
}

void Team::Pokes::reset()
{
	nick[0] = '\0';
	num = 0;
    moves[0] = 0;
    moves[1] = 0;
    moves[2] = 0;
    moves[3] = 0;
    item = 0;
    hp_ev = 0;
    att_ev = 0;
    def_ev = 0;
    satt_ev = 0;
    sdef_ev = 0;
    speed_ev = 0;
    nature = 0;
    hp_dv = 0;
    att_dv = 0;
    level = 0;
    def_dv = 0;
    satt_dv = 0;
    sdef_dv = 0;
    speed_dv = 0;
    gender = 0;
    shiney = 0;
    ability = 0;
}

TeamInfo::Pokes::Pokes()
{
    base_HP = 0;
    base_att = 0;
    base_def = 0;
    base_satt = 0;
    base_sdef = 0;
    base_spd = 0;
    reset();
}

void TeamInfo::Pokes::reset()
{
	avatar.free();
	mini_avatar.free();
}

TeamInfo::Pokes::~Pokes()
{
}

Team_Av::Team_Av(TeamBuilder *team) :MF_Applet(400, 500, Color(249, 196, 70), 100, 50),team(team)
{
    /* Initialisation des champs pour les DVs */
    ok = new MF_TButton(82, 30, team->verdana, "Ok", dims.x + 104, dims.y + 450);
    reset = new MF_TButton(82, 30, team->verdana, "Reset", dims.x + 204, dims.y + 450);
    hp_dv = new MF_ListeDeroulante(60, 20, 170, dims.x + 110, dims.y + 14, team->verdana);
    att_dv = new MF_ListeDeroulante(60, 20, 170, hp_dv->dims.x, dims.y + 44, team->verdana);
    def_dv = new MF_ListeDeroulante(60, 20, 170, hp_dv->dims.x, dims.y + 74, team->verdana);
    speed_dv = new MF_ListeDeroulante(60, 20, 170, hp_dv->dims.x, dims.y + 104, team->verdana);
    satt_dv = new MF_ListeDeroulante(60, 20, 170, hp_dv->dims.x, dims.y + 134, team->verdana);
    sdef_dv = new MF_ListeDeroulante(60, 20, 170, hp_dv->dims.x, dims.y + 164, team->verdana);

    MF_ListeDeroulante * tab[] = {hp_dv, att_dv, def_dv, speed_dv, satt_dv, sdef_dv};
    Team::Pokes &poke = team->equipe.pokes[team->current_zone];
    int tab2[] = {poke.hp_dv, poke.att_dv, poke.def_dv, poke.speed_dv, poke.satt_dv, poke.sdef_dv};
    int i = 0;
    for (MF_ListeDeroulante *it = tab[0];  ; it=tab[++i])
    {
        it->add_type<MF_TextBar>(20, 16);
        it->add_set<MF_BigASet>();

        for (int j = 0; j <= 31; j++)
        {
            it->add_option(31-j, team->tableau_nums[31-j]+1);
        }

        it->move_pos_x(31-tab2[i]);
        it->move_pos_affichage(31-tab2[i]);

        if (it == sdef_dv)
            break;
    }
    /* Fin des champs DV */

    /* Description des champs DV */
    team->verdana.style(TTF_STYLE_BOLD);
    drawString(team->verdana, "Hit Points", 24, 14);
    drawString(team->verdana, "Attack", 24, 44);
    drawString(team->verdana, "Defense", 24, 74);
    drawString(team->verdana, "Speed", 24, 104);
    drawString(team->verdana, "Sp. Attack", 24, 134);
    drawString(team->verdana, "Sp. Defense", 24, 164);
    drawString(team->verdana, "Hidden\n \nPower", 36, 204);
    team->verdana.style(TTF_STYLE_NORMAL);
    /*Fin des descriptions */

    /* Puissance cachée!! */
    hidden_power = new MF_ListeDeroulante(70, 20, 186, dims.x + 122, dims.y + 204, team->verdana);
    hidden_power->add_type<MF_TextBar>(70, 16);
    hidden_power->add_set<MF_BigASet>();
    for (int i = 1; i < 17; i++)
    {
        hidden_power->add_option(17-i, team->type_name[17-i]);
    }
    HP_det = new MF_BarManager(306, 84, dims.x + 46, dims.y + 290, team->verdana);
    HP_det->add_set<MF_BigASet>();
    for (int i = 0; i < 6; i++)
        HP_det->add_type<MF_TextBar>(50, 16);
    update_hidden_power();
    /* Fin Puissance cachée */
    /* Aptitude spéciale */
    load_abilities();
    /* Fin de l'aptitude spéciale */
    /* Genre */
    load_gender();
    /* Fin genre */
    /* Shineyttude */
    shinyness = new MF_CheckBox(team->verdana, "Shiny", team->equipe.pokes[team->current_zone].shiney, dims.x +260, dims.y+130, bgColor);
    /* Fin Shineyttude */

    allouer(ok);
    allouer(reset);
    allouer(hp_dv);
    allouer(att_dv);
    allouer(def_dv);
    allouer(speed_dv);
    allouer(satt_dv);
    allouer(sdef_dv);
    allouer(hidden_power);
    allouer(HP_det);
    allouer(shinyness);
    display_stats();
}

bool Team_Av::recoitMessage(const char *message, MF_Base * fenetre)
{
    //le tab
    if (strcmp(message, "tab") == 0 || strcmp(message, "right") == 0 || strcmp(message, "left") == 0 || strcmp(message, "up") == 0 || strcmp(message, "down") == 0)
    {
        return gereDirection(message, fenetre);
    }
    if (strcmp(message, "release") == 0)
    {
        MF_TButton *button = dynamic_cast<MF_TButton *> (fenetre);
        if (button != NULL)
        {
            if (button == ok)
            {
                team->display_stats();
                pSup->detruireMF(this);
                return true;
            }
            if (button == reset)
            {
                hp_dv->reset_pos();
                att_dv->reset_pos();
                def_dv->reset_pos();
                satt_dv->reset_pos();
                sdef_dv->reset_pos();
                speed_dv->reset_pos();
                hidden_power->reset_pos();
                if (ab_choice->pDebut->role == 1) ab_choice->polepos(ab_choice->pFin);
                if (gen_choice->pDebut->role == 1) gen_choice->polepos(gen_choice->pFin);
                shinyness->set_check(false);
            }
        }
        return false;
    }
    if (strncmp(message, "clic: ", 6) == 0)
    {
        message += 6;

        if (fenetre == HP_det)
        {
            set_DVs(atoi(message));
            return true;
        }

        MF_ListeDeroulante *liste;
        if ((liste = dynamic_cast<MF_ListeDeroulante*>(fenetre)) == NULL)
            return false;

        if (fenetre != hidden_power)
        {
            remet_DVs();
            update_hidden_power();

            return true;
        }
        set_hidden_power(0);
        return true;
    }
    if (strncmp(message, "select: ", 8) == 0)
    {
        message += 8;
        if (fenetre == ab_choice)
            team->equipe.pokes[team->current_zone].ability = atoi(message);
        else if (fenetre == gen_choice){
            team->equipe.pokes[team->current_zone].gender = atoi(message);
            team->display_gender();
            team->charge_image(team->current_zone);
            team->change_poke_image(team->current_zone);
            team->actualise_zone(team->current_zone);
        }
        return true;
    }
    if (strcmp(message, "checked") == 0 || strcmp(message, "unchecked") == 0)
    {
        if (fenetre == shinyness)
        {
            team->equipe.pokes[team->current_zone].shiney = shinyness->checked;
            team->charge_image(team->current_zone);
            team->change_poke_image(team->current_zone);
            team->actualise_zone(team->current_zone);
            return true;
        }
        return false;
    }
    return false;
}

void Team_Av::remet_DVs()
{
    if (hp_dv->MF_pos->bar_id != team->equipe.pokes[team->current_zone].hp_dv) {
        team->equipe.pokes[team->current_zone].hp_dv = hp_dv->MF_pos->bar_id;
        display_HP();
    } if (att_dv->MF_pos->bar_id != team->equipe.pokes[team->current_zone].att_dv) {
        team->equipe.pokes[team->current_zone].att_dv = att_dv->MF_pos->bar_id;
        display_Att();
    } if (def_dv->MF_pos->bar_id != team->equipe.pokes[team->current_zone].def_dv) {
        team->equipe.pokes[team->current_zone].def_dv = def_dv->MF_pos->bar_id;
        display_Def();
    } if (speed_dv->MF_pos->bar_id != team->equipe.pokes[team->current_zone].speed_dv) {
        team->equipe.pokes[team->current_zone].speed_dv = speed_dv->MF_pos->bar_id;
        display_Speed();
    } if (satt_dv->MF_pos->bar_id != team->equipe.pokes[team->current_zone].satt_dv) {
        team->equipe.pokes[team->current_zone].satt_dv = satt_dv->MF_pos->bar_id;
        display_SpDef();
    } if (sdef_dv->MF_pos->bar_id != team->equipe.pokes[team->current_zone].sdef_dv) {
        team->equipe.pokes[team->current_zone].sdef_dv = sdef_dv->MF_pos->bar_id;
        display_SpAtt();
    }
}

void Team_Av::update_hidden_power()
{
    Team::Pokes &poke = team->equipe.pokes[team->current_zone];
    int chosen_power = (((poke.hp_dv%2) + (poke.att_dv%2)*2 + (poke.def_dv%2)*4 + (poke.speed_dv%2)*8 + (poke.satt_dv%2)*16 + (poke.sdef_dv%2)*32)*15)/63 + 1;

    hidden_power->move_pos_x(-chosen_power + hidden_power->MF_pos->bar_id);
    hidden_power->move_pos_affichage(-chosen_power + hidden_power->MF_pos_affichage->bar_id);

    load_hidden_power();
    disp_hp_power();
}

void Team_Av::load_hidden_power()
{
    string path = "db/";
    path += team->type_name[hidden_power->MF_pos->bar_id];
    path += ".txt";

    ifstream in(path.c_str(), ios::binary);
    hp_data.clear();

    while (getline(in, path))
    {
        hp_data.push_back(path);
        for (int i = 2; i <= 14; i+=3) hp_data[hp_data.size()-1][i] = 0;
    }

    HP_det->clear_options();
    for (unsigned i = 0; i < hp_data.size(); i++)
    {
        HP_det->add_option(i, hp_data[i].data(), hp_data[i].data()+3, hp_data[i].data()+6, hp_data[i].data()+9, hp_data[i].data()+12, hp_data[i].data()+15);
    }
    HP_det->set_updated();
}

void Team_Av::set_DVs(int num)
{
    MF_ListeDeroulante * tab[] = {hp_dv, att_dv, def_dv, speed_dv, satt_dv, sdef_dv};
    int i = 0;
    for (MF_ListeDeroulante *it = tab[0]; ; it=tab[++i])
    {
        if (strncmp(hp_data[num].data()+ 3*i, "31", 2) == 0)
        {
            if (it->MF_pos->bar_id % 2 == 0)
            {
                it->move_pos_x(-1);
                it->move_pos_affichage(-1);
                it->set_updated();
            }
        } else
        {
            if (it->MF_pos->bar_id % 2 == 1)
            {
                it->move_pos_x(1);
                it->move_pos_affichage(1);
                it->set_updated();
            }
        }

        if (it == sdef_dv)
            break;
    }
}

void Team_Av::display_stats()
{
    display_HP();
    display_Att();
    display_Def();
    display_SpAtt();
    display_SpDef();
    display_Speed();
}

void Team_Av::set_hidden_power(int value)
{
    load_hidden_power();

    set_DVs(value);
    disp_hp_power();
}

void Team_Av::display_base(Uint16 base, Sint8 boost, Sint16 posy)
{
    Color drawingColor;
    switch(boost)
    {
        case -1:
            drawingColor = Color(0, 0, 0xFF);
            break;
        case 0:
            drawingColor = Color(0, 0, 0);
            break;
        case 1:
            drawingColor = Color(0xFF, 0, 0);
        default:
            break;
    }
    Rect destrect (185, posy, 50, 16);

    surface.fill(destrect, bgColor);
    drawString(team->verdana, toString(base).c_str(), destrect.x, posy, drawingColor);
}

void Team_Av::disp_hp_power()
{
    Team::Pokes &poke = team->equipe.pokes[team->current_zone];
    Uint8 pow = (((poke.hp_dv%4>1) + (poke.att_dv%4>1)*2 + (poke.def_dv%4>1)*4 + (poke.speed_dv%4>1)*8 + (poke.satt_dv%4>1)*16 + (poke.sdef_dv%4>1)*32)*40)/63 + 30;

    Rect dst (146, 234, 25, 20);
    FillRect(dst, bgColor);

    drawString(team->verdana, toString((int)pow).c_str(), dst.x, dst.y);
}

void Team_Av::load_abilities()
{
    int num = team->equipe.pokes[team->current_zone].num;

    abilities[0] = atoi(find_line("db/poke_ability.txt", num)->c_str());
    abilities[1] = atoi(find_line("db/poke_ability2.txt", num)->c_str());
    ability_names[0] = *find_line("db/ability_en.txt", abilities[0]+1);

    if (abilities[1] != (Uint16)-1)
        ability_names[1] = *find_line("db/ability_en.txt", abilities[1]+1);
    else
        ability_names[1] = "";

    ab_choice = new MF_BRadio();
    ab_choice->bgColor = bgColor;

    ab_choice->add_option(team->verdana, ability_names[0].c_str(), 0);
    if (abilities[1] != (Uint16)-1)
    {
        ab_choice->add_option(team->verdana, ability_names[1].c_str(), 1);
        if (!team->equipe.pokes[team->current_zone].ability)
            ab_choice->polepos(ab_choice->pFin);
    }

    ab_choice->move(dims.x + 260, dims.y + 20);
    allouer(ab_choice);
}

void Team_Av::load_gender()
{
    gen_choice = new MF_BRadio();
    gen_choice->bgColor = bgColor;
    switch (team->equipe2.pokes[team->current_zone].gender_t)
    {
        case 0:
            gen_choice->add_option(team->verdana, "Neutral", 0);
            break;
        case 1:
            gen_choice->add_option(team->verdana, "Male", 0);
            break;
        case 2:
            gen_choice->add_option(team->verdana, "Female", 0);
            break;
        case 3:
            gen_choice->add_option(team->verdana, "Male", 0);
            gen_choice->add_option(team->verdana, "Female", 1);
            if (team->equipe.pokes[team->current_zone].gender == 0)
                gen_choice->polepos(gen_choice->pFin);
        default:
            break;
    }
    gen_choice->move(dims.x + 260, dims.y + 80);
    allouer(gen_choice);
}

inline void Team_Av::resize(Uint16 w, Uint16 h)
{
    MF_Applet::resize(w, h);
}

inline void Team_Av::affiche(Surface &ecran)
{
    updated = true;

    MF_Applet::affiche(ecran);
    afficheMF(ecran);
}

void Team_Av::display_HP(){display_base(team->calc_HP(team->current_zone), 0, 14);}
void Team_Av::display_Att(){display_base(team->calc_Att(team->current_zone), team->get_boost(0), 44);}
void Team_Av::display_Def(){display_base(team->calc_Def(team->current_zone), team->get_boost(1), 74);}
void Team_Av::display_Speed(){display_base(team->calc_Speed(team->current_zone), team->get_boost(2), 104);}
void Team_Av::display_SpAtt(){display_base(team->calc_SpAtt(team->current_zone), team->get_boost(3), 134);}
void Team_Av::display_SpDef(){display_base(team->calc_SpDef(team->current_zone), team->get_boost(4), 164);}

Team::Team()
{
    Trainer_Name[0] = '\0';
    Trainer_Info[0] = '\0';
    Trainer_Win[0] = '\0';
    Trainer_Lose[0] = '\0';
}

Poke_Zone::Poke_Zone(Font &police, const char *nickname, const char *item, const char *move1, const char *move2, const char *move3, const char *move4, Surface &avatar, Sint16 x, Sint16 y)
          :MF_BApplet(153, 143, Color(0xAA, 0xAA, 0xFF), x, y)
{
    reset(police, nickname, item, move1, move2, move3, move4, avatar);
}

void Poke_Zone::reset(Font &police, const char *nickname, const char *item, const char *move1, const char *move2, const char *move3, const char *move4, Surface &avatar)
{
    set_updated();
    setColor(Color(0xAA, 0xAA, 0xFF));
    FillRect(0, bgColor);
    //image poke
    Rect r (8,12, 43, 44);
    FillRect(r);
    MF_Box_DrawBorders(r, surface, false);

    if (avatar)
        BlitSurface(avatar, 0, 9 + (43-avatar.w())/2, 14 + (42-avatar.h())/2);
    //messages
    string a = " "; a+= nickname; a += "\n \n@ "; a += item;
    drawString(police, a.data(), 55, 14);
    a = move1; a += "\n"; a += move2; a += "\n"; a += move3; a += "\n"; a += move4;
    drawString(police, a.data(), 25, 70);
    //Et voilà!
}

const char* TeamBuilder::move_learning[5] = {"Level", "EggMove", "TM / HM", "MoveTutor", "Special Move"};
const char* TeamBuilder::type_name[18] = {"Normal", "Fighting", "Flying", "Poison", "Ground", "Rock", "Bug", "Ghost", "Steel", "Fire", "Water", "Grass", "Electric", "Psychic", "Ice", "Dragon", "Dark", "Curse"};
const char* TeamBuilder::category_name[3] = {"physical", "special", "other"};
const char* TeamBuilder::nature_name[25] = {"Hardy", "Lonely", "Brave", "Adamant", "Naughty", "Bold", "Docile", "Relaxed", "Impish", "Lax", "Timid", "Hasty", "Serious", "Jolly", "Naive", "Modest", "Mild", "Quiet", "Bashful", "Rash", "Calm", "Gentle", "Sassy", "Careful", "Quirky"};

TeamBuilder::TeamBuilder(Team &equipe)
            :MF_Applet(SDL_GetVideoSurface()->w /*around 650 */, SDL_GetVideoSurface()->h /*around 610 */, Color(249, 196, 70)), verdana(NULL), equipe(equipe), Mode(Pokemons)
{
    verdana.load("verdana.ttf", 11);

    //initialisation du tableau
    for (int i = 0; i <= POKE_COUNT; i++)
    {
        strncpy(tableau_nums[i], padd(3, i).c_str(), 4);
    }

    //chargement des noms des pokes et des attaques et des objets
    load_moves();
    load_pokenames();
    load_items();

    Surface s;

    allouer (poke_z[0] = new Poke_Zone(verdana, "", "", "", "", "", "", s, 14, 12));
    allouer (poke_z[1] = new Poke_Zone(verdana, "", "", "", "", "", "", s, 5+poke_z[0]->dims.x+poke_z[0]->dims.w, 12));
    allouer (poke_z[2] = new Poke_Zone(verdana, "", "", "", "", "", "", s, 5+poke_z[1]->dims.x+poke_z[1]->dims.w, 12));
    allouer (trainer_z = new MF_BApplet(153, 143, Color(0xFF,0x33,0x33), 5+poke_z[2]->dims.x+poke_z[2]->dims.w, 12));
    allouer (poke_z[3] = new Poke_Zone(verdana, "", "", "", "", "", "", s, trainer_z->dims.x, 5 + trainer_z->dims.y + trainer_z->dims.h));
    allouer (poke_z[4] = new Poke_Zone(verdana, "", "", "", "", "", "", s, poke_z[3]->dims.x, 5 + poke_z[3]->dims.y + poke_z[3]->dims.h));
    allouer (poke_z[5] = new Poke_Zone(verdana, "", "", "", "", "", "", s, poke_z[4]->dims.x, 5 + poke_z[4]->dims.y + poke_z[4]->dims.h));

    PokemonField = new MF_BarManager(141, 84, poke_z[0]->dims.x + 12, poke_z[0]->dims.y + poke_z[0]->dims.h + 25, verdana);
    nickname = new MF_TextBox(verdana, 141, 21, 1, "", PokemonField->dims.x, PokemonField->dims.h+PokemonField->dims.y+20, 2, 1, false);
    item = new MF_ListeDeroulante(141, 20, 276, PokemonField->dims.x, nickname->dims.h+nickname->dims.y+20, verdana);
    nature = new MF_ListeDeroulante(82, 20, 84, poke_z[1]->dims.x + 12, 291, verdana);
    movepool = new MF_BarManager(438, 197, item->dims.x, item->dims.y + item->dims.h + 31, verdana);
    move[0] = new MF_TextList(verdana, 99, 22, 1, "", movepool->dims.x, movepool->dims.y + movepool->dims.h + 6, 2, 1);
    move[1] = new MF_TextList(verdana, 99, 22, 1, "", move[0]->dims.x + move[0]->dims.w + 14, move[0]->dims.y, 2, 1);
    move[2] = new MF_TextList(verdana, 99, 22, 1, "", move[1]->dims.x + move[1]->dims.w + 14, move[1]->dims.y, 2, 1);
    move[3] = new MF_TextList(verdana, 99, 22, 1, "", move[2]->dims.x + move[2]->dims.w + 14, move[2]->dims.y, 2, 1);
    //dresseur
    trNick = new MF_TextBox(verdana, 130, 25, 1, "", PokemonField->dims.x, PokemonField->dims.y+7, 2, 3, false);
    trInfo = new MF_TextBox(verdana, 290, 70, 4, "", trNick->dims.x + 70, 300, 2, 0);
    trWin = new MF_TextBox(verdana, 290, 70, 7, "", trInfo->dims.x, 393, 2, 0);
    trLose = new MF_TextBox(verdana, 290, 70, 7, "", trWin->dims.x, 486, 2, 0);

    advanced = new MF_TButton(82, 30, verdana, "Advanced", nature->dims.x, nature->dims.y + nature->dims.h + 15);
    save = new MF_TButton(82, 30, verdana, "Save", 40, 565);
    load = new MF_TButton(82, 30, verdana, "Load", save->dims.x+save->dims.w+10, save->dims.y);
    exit = new MF_TButton(82, 30, verdana, "Done", load->dims.x+load->dims.w+10, save->dims.y);

    Color c (120, 245, 233);
    hp_bar = new MF_TrayBar(351, 165, 105, 256, c, 4);
    c = Color(46, 46, 46);
    att_bar = new MF_TrayBar(hp_bar->dims.x, 195, hp_bar->dims.w, 256, c, 4);
    c = Color(106, 106, 255);
    def_bar = new MF_TrayBar(hp_bar->dims.x, 225, hp_bar->dims.w, 256, c, 4);
    c = Color(31, 252, 108);
    speed_bar = new MF_TrayBar(hp_bar->dims.x, 255, hp_bar->dims.w, 256, c, 4);
    c = Color(255, 0, 0);
    satt_bar = new MF_TrayBar(hp_bar->dims.x, 285, hp_bar->dims.w, 256, c, 4);
    c = Color(120, 0, 120);
    sdef_bar = new MF_TrayBar(hp_bar->dims.x, 315, hp_bar->dims.w, 256, c, 4);
    c = Color(64, 128, 128);
    ev_bar = new MF_TrayBar(278, 343, hp_bar->dims.w + 73, 511, c, 1, 0, false);

    Color white (0xFF, 242, 0);
    trainer_z->BlitImage(16, 47, "trainer.png", true, &white);
    trainer_z->BlitImage(30, 10, "trainer_n.png", true, &white);

    poke_z[0]->setDirections(PokemonField, NULL, NULL, trainer_z, poke_z[1]);
    poke_z[1]->setDirections(PokemonField, NULL, NULL, poke_z[0], poke_z[2]);
    poke_z[2]->setDirections(PokemonField, NULL, NULL, poke_z[1], trainer_z);
    trainer_z->setDirections(trNick, poke_z[5], poke_z[3], poke_z[2], poke_z[0]);
    poke_z[3]->setDirections(PokemonField, trainer_z, poke_z[4]);
    poke_z[4]->setDirections(PokemonField, poke_z[3], poke_z[5]);
    poke_z[5]->setDirections(PokemonField, poke_z[4], trainer_z);
    PokemonField->setDirections(nickname);
    nickname->setDirections(item);
    item->setDirections(nature);
    nature->setDirections(advanced);
    advanced->setDirections(hp_bar, nature, movepool);
    hp_bar->setDirections(att_bar, advanced, att_bar);
    att_bar->setDirections(def_bar, hp_bar, def_bar);
    def_bar->setDirections(speed_bar, att_bar, speed_bar);
    speed_bar->setDirections(satt_bar, def_bar, satt_bar);
    satt_bar->setDirections(sdef_bar, speed_bar, sdef_bar);
    sdef_bar->setDirections(movepool, sdef_bar, movepool);
    movepool->setDirections(move[0]);
    move[0]->setDirections(move[1]);
    move[1]->setDirections(move[2]);
    move[2]->setDirections(move[3]);

    trNick->setDirections(trInfo);
    trInfo->setDirections(trWin);
    trWin->setDirections(trLose);
    trLose->setDirections(poke_z[6]);

    for(int i = 0; i < 18; i ++)
    {
        string path = "db/";
        path += type_name[i];
        path += ".gif";
        move_jpg[i].load(path.c_str(), Color(0));
    }

    for(int i = 0; i < 3; i ++)
    {
        string path = "db/";
        path += category_name[i];
        path += ".bmp";
        cat_jpg[i].load(path.c_str(), Color(0));
    }

    PokemonField->add_set<MF_SmallASet>();

    int ls = verdana.line_skip();
    PokemonField->add_type<MF_TextBar>(30,ls+2);
    PokemonField->add_type<MF_TextBar>(111,ls+2);
    for(int i = 1; i < POKE_COUNT; i++)
    {
        PokemonField->add_option(i-1, tableau_nums[i], poke_names[i].c_str());
    }

    movepool->add_set<MF_BigASet>();
    movepool->add_type<MF_ImageBar>(34,16);
    movepool->add_type<MF_TextBar>(105,ls+2);
    movepool->add_type<MF_TextBar>(95,ls+2);
    movepool->add_type<MF_TextBar>(50,ls+2);
    movepool->add_type<MF_TextBar>(50,ls+2);
    movepool->add_type<MF_TextBar>(50,ls+2);
    movepool->add_type<MF_ImageBar>(34,16);

    nature->add_set<MF_BigASet>();
    nature->add_type<MF_TextBar>(nature->dims.w-4-nature->aff_data->dim_fleches, nature->dims.h-4);
    for (int i = 0; i < 25; i++)
    {
        nature->add_option(i, nature_name[i]);
    }

    item->add_set<MF_BigASet>();
    item->add_type<MF_TextBar>(item->dims.w-4-item->aff_data->dim_fleches, item->dims.h-4);
    for (int i = 0; i < ITEMS_COUNT+BERRY_COUNT; i++)
    {
        item->add_option(i, item_names[i].c_str());
    }
    item->sort_options(0);

    SwitchTo(Trainer);

    current_zone = 6;
    trainer_z->clicOn = true;

    set_team();
}

TeamBuilder::~TeamBuilder()
{
    //allouer pour mieux détruire!
    if (Mode == Trainer)
    {
        allouer(PokemonField);
        allouer(nickname);
        allouer(item);
        allouer(nature);
        allouer(advanced);
        allouer(movepool);
        allouer(move[0]);
        allouer(move[1]);
        allouer(move[2]);
        allouer(move[3]);
        allouer(hp_bar);
        allouer(att_bar);
        allouer(def_bar);
        allouer(speed_bar);
        allouer(satt_bar);
        allouer(sdef_bar);
        allouer(ev_bar);
    } else
    {
        allouer(trNick);
        allouer(trInfo);
        allouer(trWin);
        allouer(trLose);
        allouer(save);
        allouer(load);
        allouer(exit);
    }
}

void TeamBuilder::load_items()
{
    ifstream in("db/items_en.txt");
    int i (0);
    for ( ; i < ITEMS_COUNT; i++)
    {
        getline(in, item_names[i]);
    }
    ifstream in2("db/berries_en.txt");
    for ( ; i < ITEMS_COUNT+BERRY_COUNT; i++)
    {
        getline(in2, item_names[i]);
    }
}

void TeamBuilder::load_team()
{
    equipe.load("team/team.pcb");

    set_team();
}

void TeamBuilder::load_gender(int i)
{
    equipe2.pokes[i].gender_t = atoi(find_line("db/poke_gender.txt", equipe.pokes[i].num)->c_str());
}

void TeamBuilder::SwitchTo(Switch wanted)
{
    if (Mode == wanted) return;
    Mode = wanted;

    Rect rect (2, 2, dims.w-4, dims.h-4);
    surface.fill(rect, Color(249, 196, 70));
    switch (wanted)
    {
        case Pokemons:
            desallouer(trNick);
            desallouer(trInfo);
            desallouer(trWin);
            desallouer(save);
            desallouer(load);
            desallouer(trLose);
            desallouer(exit);

            allouer(PokemonField);
            allouer(nickname);
            allouer(item);
            allouer(nature);
            allouer(advanced);
            allouer(movepool);
            allouer(move[0]);
            allouer(move[1]);
            allouer(move[2]);
            allouer(move[3]);
            allouer(hp_bar);
            allouer(att_bar);
            allouer(def_bar);
            allouer(speed_bar);
            allouer(satt_bar);
            allouer(sdef_bar);
            allouer(ev_bar);

            drawString(verdana, "Pokémon", PokemonField->dims.x, poke_z[0]->dims.y + poke_z[0]->dims.h + 10);
            drawString(verdana, "Nickname", PokemonField->dims.x, PokemonField->dims.h+PokemonField->dims.y+4);
            drawString(verdana, "Item", PokemonField->dims.x, nickname->dims.h+nickname->dims.y+4);
            drawString(verdana, "Nature", nature->dims.x, nature->dims.y-16);
            verdana.style(TTF_STYLE_BOLD);
            drawString(verdana, "Move", movepool->dims.x+2, movepool->dims.y-14);
            drawString(verdana, "Learning", movepool->dims.x+142, movepool->dims.y-14);
            drawString(verdana, "PP", movepool->dims.x+236, movepool->dims.y-14);
            drawString(verdana, "Pow", movepool->dims.x+284, movepool->dims.y-14);
            drawString(verdana, "Acc", movepool->dims.x+336, movepool->dims.y-14);
            drawString(verdana, "Cat", movepool->dims.x+386, movepool->dims.y-14);
            verdana.style(0);
            drawString(verdana, "HP: ", 278, 165);
            drawString(verdana, "Att: ", 278, 195);
            drawString(verdana, "Def: ", 278, 225);
            drawString(verdana, "Speed: ", 278, 255);
            drawString(verdana, "SpAtt: ", 278, 285);
            drawString(verdana, "SpDef: ", 278, 315);

            //display_gender();
            rect.x = poke_z[1]->dims.x + 38;
            rect.y = poke_z[1]->dims.y + poke_z[1]->dims.h + 97;

            drawString(verdana, "Lv 100", rect.x, rect.y);
            change_poke_image(current_zone);
            display_stats();
            polepos(poke_z[current_zone]);
            insert_trNick();
            insert_trInfo();
            insert_trLose();
            insert_trWin();
            break;
        case Trainer:
            desallouer(PokemonField);
            desallouer(nickname);
            desallouer(item);
            desallouer(nature);
            desallouer(advanced);
            desallouer(movepool);
            desallouer(move[0]);
            desallouer(move[1]);
            desallouer(move[2]);
            desallouer(move[3]);
            desallouer(hp_bar);
            desallouer(att_bar);
            desallouer(def_bar);
            desallouer(satt_bar);
            desallouer(sdef_bar);
            desallouer(speed_bar);
            desallouer(ev_bar);

            allouer(trNick);
            allouer(trInfo);
            allouer(trWin);
            allouer(trLose);
            allouer(save);
            allouer(load);
            allouer(exit);

            drawString(verdana, "Trainer Name", PokemonField->dims.x, poke_z[0]->dims.y + poke_z[0]->dims.h + 15);
            drawString(verdana, "Player Info", trInfo->dims.x, trInfo->dims.y - 16);
            drawString(verdana, "Winning Message", trWin->dims.x, trWin->dims.y - 16);
            drawString(verdana, "Losing Message", trLose->dims.x, trLose->dims.y - 16);
            trNick->clear();
            trNick->ecrire(equipe.Trainer_Name);
            polepos(trainer_z);
            break;
    }
}

bool TeamBuilder::recoitMessage(const char *message, MF_Base *fenetre)
{
    //le tab
    if (strcmp(message, "tab") == 0)
    {
        return gereDirection(message, fenetre);
    }
    if (strcmp(message, "right") == 0 || strcmp(message, "left") == 0 || strcmp(message, "up") == 0 || strcmp(message, "down") == 0)
    {
        if (gereDirection(message, fenetre))
        {
            fenetre = pDebut;
            goto clic;
        }

        return false;
    }
    if (strcmp(message, "false-release") == 0)
    {
        MF_BApplet *sender = dynamic_cast<MF_BApplet *> (fenetre);
        if (sender != NULL)
        {
        	sender->clicOn = true;

            return true;
        }

        return false;
    }
    if (strcmp(message, "release") == 0)
    {
        MF_BApplet *sender = dynamic_cast<MF_BApplet *> (fenetre);
        if (sender != NULL)
        {
        	sender->clicOn = true;

            return true;
        }
        MF_TButton *button = dynamic_cast<MF_TButton *> (fenetre);
        if (button != NULL)
        {
            if (button == save)
            {
                save_team();
                allouer(new MF_Alert(this, verdana, "Team saved!", (this->bgColor)));
                return true;
            }
            if (button == load)
            {
                load_team();
                allouer(new MF_Alert(this, verdana, "Team loaded!", (this->bgColor)));
                return true;
            }
            if (button == advanced)
            {
                if (equipe.pokes[current_zone].num == 0)
                    return false;
                allouer(new Team_Av(this));
                return true;
            }
            if (button == exit)
            {
                insert_current();
                envoieMessage("menu");
                return true;
            }
        }
        return false;
    }
    //et là on gère le clic en fonction du bouton
    if (strcmp(message, "clic") == 0)
    {
        clic:
        MF_BApplet *sender = dynamic_cast<MF_BApplet *> (fenetre);
        if (sender != NULL)
        {
            sender->clicOn = true;

            if (sender == trainer_z && current_zone != 6)
            {
                insert_nickname();
                current_zone = 7;
                SwitchTo(Trainer);
            }
            for (int i = 0; i < 6; i++)
            {
            	if (poke_z[i] != sender)
            		poke_z[i]->clicOn = false;
            	else if (current_zone != i) //if (poke_z[i] == sender && current_zone != i)
                {
                    trainer_z->clicOn = false;
                    if (current_zone == 6)
                    {
                        insert_trainer();
                    } else if (current_zone == 7)
                    {
                        current_zone = 6;
                    } else
                    {
                        insert_nickname();
                    }
                    current_zone = i;
                    SwitchTo(Pokemons);
                    reset_move();
                    load_pokemoves(equipe.pokes[i].num);
                    actualise_moves(i);
                    rewrite_nick(i);
                    move[3]->setDirections(sender);
                }
            }

			return true;
        }
        return false;
    }
    if(strncmp(message, "clic: ", 6) == 0)
    {
        if (fenetre == nature || fenetre == item)
        {
            message -= 8;
            goto dclic;
        }
    }
    if(strncmp(message, "double-clic: ", 13) == 0)
    {
        dclic:
    	MF_BarManager *sender = dynamic_cast<MF_BarManager *> (fenetre);
    	if (sender != NULL)
    	{
    		if (sender == PokemonField)
    		{
    			//on a chopé un double clic sur un poké
    			int pokenum = atoi(message+13) + 1;
    			//On met donc le poke qu'on a chopé en poké principal en reloadant ses moves
    			if (reset_poke(current_zone, pokenum) )
                    actualise_moves(current_zone);

    			return true;
    		}
    		if (sender == movepool)
    		{
    		    //on a chopé un double clic sur un move
    		    int movenum = atoi(message+13);

    		    int i;
    		    //on regarde si le move est pas déjà pris
    		    for (i = 0; i < 4; i++)
    		    {
    		        if (equipe.pokes[current_zone].moves[i] == movenum)
    		        {
    		            string msg = poke_names[equipe.pokes[current_zone].num] + " already has the move " + moves[movenum] + "!";
    		            allouer (new MF_Alert(this, verdana, msg.c_str(), bgColor));
    		            return true;
    		        }
    		    }
    		    //on cherche les moves de libre
    		    for (i = 0; i < 4; i++)
    		    {
    		        if (equipe.pokes[current_zone].moves[i] == 0) break;
    		    }
    		    if (i == 4)
    		    {
    		        string msg = poke_names[equipe.pokes[current_zone].num] + " already got 4 moves!";
    		        allouer (new MF_Alert(this, verdana, msg.c_str(), bgColor));
    		        return true;
    		    }
                equipe.pokes[current_zone].moves[i] = movenum;
                move[i]->clear();
                move[i]->ecrire(moves[movenum]);
                actualise_zone(current_zone);

    		    return true;
    		}
    		if (sender == nature)
    		{
    		    unsigned int natnum = atoi(message+13);
    		    if (equipe.pokes[current_zone].nature == natnum)
                    return true;

                equipe.pokes[current_zone].nature = natnum;
                display_stats();

                return true;
    		}
    		if (sender == item)
    		{
    		    unsigned int itemnum = atoi(message+13);
    		    if (equipe.pokes[current_zone].item == itemnum)
                    return true;

                equipe.pokes[current_zone].item = itemnum;
                actualise_zone(current_zone);

                return true;
    		}
    	}
    	return false;
    }

    if(strncmp(message, "enter", 5) == 0)
    {
		{
	    	MF_TextList *sender = dynamic_cast<MF_TextList *> (fenetre);
	    	if (sender != NULL)
	    	{
	    		for (int i = 0; i < 4; i++)
	    		{
	    			if (move[i] == sender)
	    			{
	    			    int movenum = sender->IDselected+1;
	        		    for (int j = 0; j < 4; j++)
                        {
                            if (equipe.pokes[current_zone].moves[j] == movenum)
                            {
                                string msg = poke_names[equipe.pokes[current_zone].num] + " already has the move " + moves[movenum] + "!";
                                sender->clear();
                                sender->ecrire("(None)");
                                allouer (new MF_Alert(this, verdana, msg.c_str(), bgColor));
                                return false;
                            }
                        }
	    				equipe.pokes[current_zone].moves[i] = sender->IDselected+1;
	    				actualise_zone(current_zone);
	    				return true;
	    			}
	    		}
	    		return false;
	    	}
		}
		if (fenetre == nickname)
		{
			insert_nickname();
            polepos(item);
			return true;
		}
		if (fenetre == trNick)
		{
		    insert_trNick();
		    polepos(trInfo);
		    return true;
		}
		if (message[5] == ':' && message[6] == ' ')
		{
		    message -= 7;
		    goto dclic;
		}
    }
    //ev_bar
    if (strcmp(message, "moved") == 0)
    {
        MF_TrayBar *sender = dynamic_cast<MF_TrayBar*>(fenetre);
        if (sender != NULL)
        {
            Uint16 n = sum_evs();
            if (n > 510)
            {
                sender->pos_icon -= (n-510);
                if (sender->pos_icon == sender->lastval)
                    return true;
                ev_bar->pos_icon = 510;
            }
            else
                ev_bar->pos_icon = n;
            if(sender == hp_bar) {
                equipe.pokes[current_zone].hp_ev = hp_bar->pos_icon;
                display_HP();
            } else if (sender == att_bar) {
                equipe.pokes[current_zone].att_ev = att_bar->pos_icon;
                display_Att();
            } else if (sender == def_bar) {
                equipe.pokes[current_zone].def_ev = def_bar->pos_icon;
                display_Def();
            } else if (sender == satt_bar) {
                equipe.pokes[current_zone].satt_ev = satt_bar->pos_icon;
                display_SpAtt();
            } else if (sender == sdef_bar) {
                equipe.pokes[current_zone].sdef_ev = sdef_bar->pos_icon;
                display_SpDef();
            } else if (sender == speed_bar) {
                equipe.pokes[current_zone].speed_ev = speed_bar->pos_icon;
                display_Speed();
            } else
                    return false;

            display_Evs();
            return true;
        }
        return false;
    }

    return false;
}

void TeamBuilder::affiche(Surface &ecran)
{
    if (!updated)
    {
        updated = true;
        MF_Applet::affiche(ecran);
        afficheMF(ecran);
    } else if (pDebut->check_updated() == false)
    {
        pDebut->affiche(ecran);
    }
}

void TeamBuilder::load_pokenames()
{
    ifstream pokes("db/pokemons_en.txt", ios_base::binary);

    for (int i = 0; i < POKE_COUNT; i++)
    {
        getline(pokes, poke_names[i]);
    }
}

void TeamBuilder::load_moves()
{
    ifstream pokes("db/moves_en.txt", ios_base::binary);

    for (int i = 0; i < MOVE_COUNT; i++)
    {
        getline(pokes, moves[i]);
    }

    pokes.close();
    pokes.open("db/move_accuracy.txt", ios_base::binary);

    for (int i = 0; i < MOVE_COUNT; i++)
    {
        getline(pokes, move_acc[i]);
    }

    pokes.close();
    pokes.clear();
    pokes.open("db/move_power.txt", ios_base::binary);

    if (!pokes) throw MF_Exception("impossible d'ouvrir db/move_power.txt");

    for (int i = 0; i < MOVE_COUNT; i++)
    {
        getline(pokes, move_pow[i]);
    }

    pokes.close();
    pokes.clear();
    pokes.open("db/move_pp.txt", ios_base::binary);

    if (!pokes) throw MF_Exception("impossible d'ouvrir db/move_pp.txt");

    for (int i = 0; i < MOVE_COUNT; i++)
    {
        getline(pokes, move_pp[i]);
    }

    pokes.close();
    pokes.clear();
    pokes.open("db/move_types2.txt", ios_base::binary);

    if (!pokes) throw MF_Exception("impossible d'ouvrir db/move_types2.txt");

    for (int i = 0; i < MOVE_COUNT; i++)
    {
        string tmp;
        getline(pokes, tmp);
        move_type[i] = (TYPE)(tmp[0]-'a');
    }

    pokes.close();
    pokes.clear();
    pokes.open("db/move_category.txt", ios_base::binary);

    if (!pokes) throw MF_Exception("impossible d'ouvrir db/category.txt");

    for (int i = 0; i < MOVE_COUNT; i++)
    {
        string tmp;
        getline(pokes, tmp);
        move_cat[i] = (CATEGORY)(tmp[0]-'a');
    }
}

void TeamBuilder::load_pokemoves(int pokenum)
{
    reset_move();

	if (pokenum == 0) return;

    #ifdef DEBUG_ON
    assert(pokenum > 0 && pokenum <= POKE_COUNT);
    #endif

    //on cherche la ligne qui nous intéresse dans chaque fichier db
    current_move_db[0] = *find_line("db/pokes_DP_lvmoves.txt", pokenum);
    current_move_db[1] = *find_line("db/pokes_DP_eggmoves.txt", pokenum);
    current_move_db[2] = *find_line("db/pokes_DP_TMmoves.txt", pokenum);
    current_move_db[3] = *find_line("db/pokes_DP_MTmoves.txt", pokenum);
    current_move_db[4] = *find_line("db/pokes_DP_specialmoves.txt", pokenum);

    for (int i = 0; i < 5; i++)
    {
    	const char *data = current_move_db[i].c_str();

    	while (*data != 0)
    	{
    		char str[4];
    		strncpy(str, data, 3);
    		str[3] = 0;
    		move[0]->insert(moves[atoi(str)].c_str(), atoi(str)-1);
    		int n = atoi(str);
    		movepool->add_option(n, &move_jpg[move_type[n-1]], moves[n].c_str(), move_learning[i], move_pp[n-1].c_str(), move_pow[n-1].c_str(), move_acc[n-1].c_str(), &cat_jpg[move_cat[n-1]]);

    		//each move takes 3 digits, so each time i move the pointer 3 digits
    		data+=3;
    	}
    }

    move[1]->copy(move[0]->mots);
    move[2]->copy(move[0]->mots);
    move[3]->copy(move[0]->mots);
    movepool->sort_options(1);
}

bool TeamBuilder::reset_poke(unsigned char zone, unsigned int pokenum)
{
	assert(zone < 6);

	if (equipe.pokes[current_zone].num == pokenum)
		return false;

	equipe.pokes[current_zone].reset();
	equipe.pokes[current_zone].num = pokenum;
	equipe.pokes[current_zone].level = 100;
	equipe.pokes[current_zone].att_dv = 31;
	equipe.pokes[current_zone].hp_dv = 31;
	equipe.pokes[current_zone].def_dv = 31;
	equipe.pokes[current_zone].satt_dv = 31;
	equipe.pokes[current_zone].sdef_dv = 31;
	equipe.pokes[current_zone].speed_dv = 31;
	equipe.pokes[current_zone].item = 0;

	equipe2.pokes[current_zone].load_stats(pokenum);
    //Le genre
    load_gender(zone);
	//on recharge les moves
	load_pokemoves(pokenum);

	//Et on doit se coltiner le reset de l'affichage
	charge_image(current_zone);

    nature->move_pos_affichage(-nature->pos_affichage_v);
    nature->move_pos_x(-nature->posx);
	actualise_zone(zone);
	return true;
}

void TeamBuilder::insert_in(MF_TextBox *box, unsigned length, string &dest)
{
    if (box->champ_texte.texte.length() > length)
        dest = box->get_content().substr(0, length);
    else
        dest = box->get_content();
}
void TeamBuilder::actualise_zone(unsigned char zone)
{
    unsigned short *mv = equipe.pokes[zone].moves;
    unsigned short num = equipe.pokes[zone].num;

    poke_z[zone]->reset(verdana, poke_names[num].c_str(), item_names[equipe.pokes[zone].item].c_str(), moves[mv[0]].c_str(), moves[mv[1]].c_str(), moves[mv[2]].c_str(), moves[mv[3]].c_str(), equipe2.pokes[zone].mini_avatar);

    return;
}

//remet à jour la page du milieu pour un poké
void TeamBuilder::actualise_moves(unsigned char zone)
{
	assert(zone < 6);

	unsigned short *mv = equipe.pokes[zone].moves;
	for (int i = 0; i < 4; i++)
		move[i]->ecrire(moves[mv[i]].c_str());
	nickname->clear();
	nickname->ecrire(equipe.pokes[zone].nick);
	//les barres :s
	hp_bar->pos_icon = equipe.pokes[zone].hp_ev;
	att_bar->pos_icon = equipe.pokes[zone].att_ev;
	def_bar->pos_icon = equipe.pokes[zone].def_ev;
	satt_bar->pos_icon = equipe.pokes[zone].satt_ev;
	sdef_bar->pos_icon = equipe.pokes[zone].sdef_ev;
	speed_bar->pos_icon = equipe.pokes[zone].speed_ev;
	ev_bar->pos_icon = sum_evs();
	//la nature
    nature->move_pos_affichage(equipe.pokes[current_zone].nature - nature->pos_affichage_v);
    nature->move_pos_x(nature->pos_affichage_v-nature->posx);
    //l'objet
    item->move_pos_affichage(item->get_pos_of(equipe.pokes[current_zone].item) - item->pos_affichage_v);
    item->move_pos_x(item->pos_affichage_v-item->posx);

	change_poke_image(zone);
	display_gender();
	display_stats();
}

void TeamBuilder::change_poke_image(Uint8 zone)
{
    Surface &surf = equipe2.pokes[zone].avatar;
	Rect r (poke_z[1]->dims.x + 11, poke_z[1]->dims.y + poke_z[1]->dims.h + 10, 82, 82);

    FillRect(r, Color(0xFF, 0xFF, 0xFF));
    MF_Box_DrawBorders(r, surface, false);

	if (!surf)
		return;

	//On centre l'image à copier
	r.x += (r.w-surf.w())/2+1;
	r.y += (r.h-surf.h())/2;

	BlitSurface(surf, 0, r.x, r.y);
}

Uint16 TeamBuilder::calculate_base(Uint8 base_stat, Uint8 level, Uint8 dv, Uint8 ev)
{
    return ((2*base_stat + dv+ ev/4)*level)/100 + 5;
}

void TeamBuilder::display_base(Uint16 base, Uint8 ev, Sint8 boost, Sint16 posy)
{
    Color drawingColor;
    switch(boost)
    {
        case -1:
            drawingColor = Color(0, 0, 0xFF);
            break;
        case 0:
            drawingColor = Color(0, 0, 0);
            break;
        case 1:
            drawingColor = Color(0xFF, 0, 0);
        default:
            break;
    }
    Rect destrect (321, posy, 50, 16);
    surface.fill(destrect, bgColor);
    drawString(verdana, toString(base).c_str(), destrect.x, posy, drawingColor);

    destrect.x = 460;
    destrect.w = 30;
    surface.fill(destrect, bgColor);

    drawString(verdana, toString((int)ev).c_str(), destrect.x, posy, drawingColor);
}

void TeamBuilder::display_Evs()
{
    Rect destrect (460, 343, 30,16);
    surface.fill(destrect, bgColor);

    drawString(verdana, toString(sum_evs()).c_str(), destrect.x, destrect.y);
}

void TeamBuilder::display_stats()
{
    display_HP();
    display_Att();
    display_Def();
    display_SpAtt();
    display_SpDef();
    display_Speed();
    display_Evs();
}

void TeamBuilder::display_gender()
{
    Rect rect (poke_z[1]->dims.x + 12, poke_z[1]->dims.y + poke_z[1]->dims.h + 94, 17, 17);

    surface.fill(rect, bgColor);

    if (equipe2.pokes[current_zone].gender_t == 0)
        return;
    if (equipe2.pokes[current_zone].gender_t == 1)
        BlitImage(rect.x, rect.y, "Male.png");
    else if (equipe2.pokes[current_zone].gender_t == 2)
        BlitImage(rect.x, rect.y, "Female.png");
    else if (equipe2.pokes[current_zone].gender_t == 3 && equipe.pokes[current_zone].gender == 0)
        BlitImage(rect.x, rect.y, "Male.png");
    else
        BlitImage(rect.x, rect.y, "Female.png");
}

void TeamBuilder::charge_image(int zone)
{
    equipe2.pokes[zone].load_image(equipe.pokes[zone].num, equipe.pokes[zone].shiney, equipe.pokes[zone].gender, equipe2.pokes[zone].gender_t);
}

void TeamBuilder::set_team()
{
    trNick->clear();
    trNick->ecrire(equipe.Trainer_Name);
    trInfo->clear();
    trInfo->ecrire(equipe.Trainer_Info);
    trLose->clear();
    trLose->ecrire(equipe.Trainer_Lose);
    trWin->clear();
    trWin->ecrire(equipe.Trainer_Win);

    for (int i = 0; i < 6; i++)
    {
        load_gender(i);
        charge_image(i);
        equipe2.pokes[i].load_stats(equipe.pokes[i].num);
        actualise_zone(i);
    }
}

void TeamBuilder::rewrite_nick(Uint8 zone)
{
    nickname->clear();
    nickname->ecrire(equipe.pokes[zone].nick);
}

