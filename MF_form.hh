#ifndef MF_FORM_HH
#define MF_FORM_HH

//MF_form.hh
#include "MF_text.hh"
#include "intergfx.hpp"
#include <limits>
#include <set>
#include <cstdarg>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

struct MF_ArrowSet
{
    Uint8 dim_fleches;
    Color bgColor;
    Color borderColor;
    Color spacerColor;
    Color fillingColor;
    Surface flechehaut;
    Surface flechebas;
    Surface flechegauche;
    Surface flechedroite;

    MF_ArrowSet();
    ~MF_ArrowSet();
};

class MF_SmallASet : public MF_ArrowSet
{
    public:
        MF_SmallASet();
};

class MF_BigASet : public MF_ArrowSet
{
    public:
        MF_BigASet();
};

template <class T>
string toString(T param)
{
    ostringstream out;
    out << param;
    return out.str();
}

struct MF_Base_Type {
    virtual MF_Base* create ( void ) const {
        throw;
    }
    Uint16 w, h;
};

template < class T >
class MF_Type : public MF_Base_Type
{
        typedef T mytype;
        virtual MF_Base* create ( void ) const {
            return dynamic_cast<MF_Base *> (new mytype);
        }
};

struct MF_Strcmp : public std::binary_function<const char*, const char*, bool>
{
    bool operator () (const char *a, const char *b) const
    {
        return strcasecmp(a, b) == 0;
    }
};

struct MF_Strnicmp : public std::binary_function<const char*, const char*, bool>
{
    bool operator () (const char *a, const char *b) const
    {
        int i = strlen(a), j = strlen(b);
        if (i > j) return false;
        return strncasecmp(a, b, i) == 0;
    }
};

class MF_Applet;

//Comme un champ texte, mais plus facile d'utilisation
class MF_TextBox : virtual public MF_Directions
{
    public:
        //Si = 0, il faudra faire shift+enter ou shift+tab pour en faire, sinon le message tab ou enter est envoyé à pSup
        bool enter;
        bool tab;
        MF_MWLigne champ_texte;

        MF_TextBox(Font &police, Uint16 w, Uint16 h, int capacity = 1, const char* texte="", Sint16 x = 0, Sint16 y =0, int offsetx = 2, int offsety = 2, bool enter = true, bool tab = false);
        ~MF_TextBox(){
            ;
        }

        virtual void loadFont(const char *fichier, int ptsize){
            champ_texte.police.load(fichier, ptsize);
        }
        virtual void shareFont(const Font &font){
            champ_texte.police = font;
        }

        virtual bool gereEvenement(const SDL_Event &event);
        virtual void affiche(Surface &ecran);
        virtual std::string &get_content() {
            return champ_texte.texte;
        }
        virtual void actualiser() {
            champ_texte.actualiser();
        }
        virtual void setPos(bool prems)
        {
            MF_Base::setPos(prems);
            champ_texte.setPos(prems);
            if (!champ_texte.updated || !champ_texte.second_update) set_updated();
        }
        virtual void clear() {
            champ_texte.effacer(0,-1);
        }
        virtual void ecrire(std::string texte) {
            champ_texte.ecrire(texte);
        }
        virtual void ecrire(const char *texte) {
            champ_texte.ecrire(texte);
        }
        virtual void move(Sint16 x, Sint16 y)
        {
            MF_Base::move(x, y);
            champ_texte.move(x+2, y+2);
        }

        virtual bool check_updated() {
            return champ_texte.check_updated();
        }
};

class MF_TButton : public MF_Applet, virtual public MF_Directions
{
    public:
        //si cliqué
        bool clicOn;

        MF_TButton(Uint16 w, Uint16 h, Font &font, const char *texte, Sint16 x = 0, Sint16 y = 0, const Color &rgb = Color(0xDD, 0xDD, 0xDD));
        ~MF_TButton(){
            ;
        }

        //Affichage
        virtual void affiche(Surface &surface);
        //pour les clics
        virtual bool gereEvenement(const SDL_Event &event);
};

//Pour afficher des bordures
void MF_Box_DrawBorders(const Rect &dims, Surface &ecran, bool inverted = false);

//Une sorte de champ texte,
//il contient une liste de mots, qui s'affiche en surligné pour compléter les mots déjà existant
class MF_TextList : public MF_TextBox, virtual public MF_Directions
{
    struct MF_TextList_Data
        {
            const char* mot;
            int datanum;
            MF_TextList_Data():mot(NULL), datanum(-1){};
            MF_TextList_Data(const char* mot_, int datanum_):mot(mot_), datanum(datanum_){};
        };
    struct MF_Strcmp : public std::binary_function<MF_TextList_Data, MF_TextList_Data, bool>
        {
            bool operator () (MF_TextList_Data a, MF_TextList_Data b) const
            {
                return strcasecmp(a.mot, b.mot) <= 0;
            }
        };

    public:
        int IDselected;
        std::set<MF_TextList_Data, MF_Strcmp> mots;
        bool updated;

        MF_TextList(Font &police, Uint16 w, Uint16 h, int capacity = 1, const char* texte="", Sint16 x = 0, Sint16 y =0, int offsetx = 2, int offsety = 2);
        ~MF_TextList(){
            ;
        }

        //fonctions pour filtrer les évènements
        //retourne 1 si l'évènement est viable, 0 sinon
        virtual bool EventFilter(const SDL_Event &event);
        //après avoir écrit, on appelle cette fonction pour fouiller dans le set de mots
        //et afficher le mot correspondant
        void post_event(const SDL_Event &event);
        //On redéfinit les fonctions pour y inclure les deux fonctions ci-dessus
        virtual bool gereEvenement(const SDL_Event &event)
        {
            if (EventFilter(event) == 0) return true;
            bool result = MF_TextBox::gereEvenement(event);
            post_event(event);
            return result;
        }
        //Pour copier un set et le mettre dans la classe
        void copy(std::set<MF_TextList_Data, MF_Strcmp> &src) {
            mots = src;
        }
        //insérer un élément
        void insert(const char* item, int num) {
            mots.insert(MF_TextList_Data(item, num));
        }
        //trouver un mot par rapport au champ de texte
        const char * find_word();
        //ecrire
        template <class T>
        void ecrire(T data){
            champ_texte.ecrire(data);
            find_word();
        }
        virtual void affiche(Surface &ecran)
        {
            if (!updated) {
                actualiser();
                updated = true;
            }
            MF_TextBox::affiche(ecran);
        }
        virtual void set_updated()
        {
            updated = false;
            if (pSup != NULL) pSup->set_updated();
        }
        //faire un reset
        virtual void reset();
};

//fonction timer
Uint32 MF_BarManager_Timer(Uint32 interval, void *param);
class MF_BarManager;

class MF_DataBar : virtual public MF_Base
{
    public:
        virtual MF_DataBar * init(Uint16 w, Uint16 h, void *param) {
            resize(w, h);
            return dynamic_cast<MF_DataBar *> (pApres);
        };
        virtual void affiche(MF_BarManager *boss, Rect &pos_barre);
        virtual void affiche_next(MF_BarManager *boss, Rect &pos_barre);
        virtual int compare(MF_DataBar *member2) {
            return 0;
        }
};

class MF_DataLine : virtual public MF_Boss
{
    public:
        MF_DataLine(vector<MF_Base_Type *> &cont);
        Uint16 bar_id;
        virtual void affiche(MF_BarManager *boss, Rect &pos_barre);
        virtual void setPos(bool prems)
        {
            MF_Base::setPos(prems);
            updated = false;
        }
};

enum BorderType
{
    wo = 0,
    RegularBorder = 1
};

class MF_BarManager : virtual public MF_Boss, virtual public MF_Surf, virtual public MF_Directions
{
    public:
        //la barre courante
        //verticale
        unsigned int posx;
        unsigned int pos_affichage_v;
        MF_DataLine *MF_pos;
        MF_DataLine *MF_pos_affichage;

        //horizontale
        Uint16 plus_grande_largeur;
        Uint16 pos_affichage_l;

        std::vector<MF_Base_Type*> type_cont;
        int num_types;

        //Pour les double-clics
        Uint32 last_clic;

        //barre
        bool barre_v_clicon;
        bool barre_h_clicon;
        bool barre_v_on;
        bool barre_h_on;
        Sint16 barre_v_pos;
        Uint16 barre_v_h;
        Sint16 barre_h_pos;
        Uint16 barre_h_w;

        //le timer
        SDL_TimerID timer;
        bool timer_wasinit;
        xy posclic;
        xy posmotion;

        //la couleur de surlignage, quand premier plan ou pas
        Color VIP_HLColor;
        Color DEF_HLColor;

        //la taille des options;
        Uint8 option_h;

        //la police
        Font police;

        //Les données des flèches
        MF_ArrowSet *aff_data;

        //la bordure
        BorderType btype;

        //constructeur
        MF_BarManager(Uint16 w, Uint16 h, Sint16 x, Sint16 y, Font &font);
        MF_BarManager(Uint16 w, Uint16 h, Sint16 x, Sint16 y, const char *font_path, Uint8 ptsize);
        //destructeur
        ~MF_BarManager();

        virtual void resize(Uint16 w, Uint16 h);
        void shareFont(Font &font);
        void loadFont(const char *font_path, Uint8 ptsize);
        virtual void clear_options();
        virtual bool add_option(int no_use, ...);
        virtual void start_timer();
        virtual void sort_options(int field);
        virtual void stop_timer(void);
        virtual unsigned int maxlignes(){
            return (dims.h-4-aff_data->dim_fleches*barre_h_on)/option_h;
        }
        virtual unsigned int maxlargeur(){
            return (dims.w-4-aff_data->dim_fleches*barre_v_on);
        }
        virtual void actualiser();
        virtual bool gereEvenement(const SDL_Event &event);
        virtual bool gereTouche(const SDL_KeyboardEvent &event);
        virtual bool gereClic(const SDL_MouseButtonEvent &event);
        virtual bool gereMotion(const SDL_MouseMotionEvent &event);
        virtual void update_barre();
        virtual void affiche_barre();
        virtual int descendcurseur(int crans);
        virtual int montecurseur(int crans);
        virtual int montebarre_crans(int crans);
        virtual int descendbarre_crans(int crans);
        virtual int montebarre(int px);
        virtual int descendbarre(int px);
        virtual int gauchebarre(int px);
        virtual int droitebarre(int px);
        virtual bool gauchebarre_crans(int crans);
        virtual bool droitebarre_crans(int crans);
        virtual bool repos_barre();
        virtual void reset_pos() {
            posx = pos_affichage_v = 0;
            MF_pos = MF_pos_affichage = dynamic_cast<MF_DataLine*>(pFin);
            set_updated();
        }
        virtual int get_barre_v_h();
        virtual int get_barre_v_pos();
        virtual int get_barre_h_w();
        virtual int get_barre_h_pos();
        virtual int get_pos_of(Uint16 id);
        virtual void affiche(Surface &ecran);
        virtual void setPos(bool prems);
        virtual int move_pos_x(int steps);
        virtual int move_pos_affichage(int steps);
        template <class T>
        void add_type(Uint16 w, Uint16 h);
        void del_types()
        {
            for (vector<MF_Base_Type*>::iterator it = type_cont.begin(); it !=type_cont.end(); it++)
                delete *it;
            type_cont.clear();
        }
        template<class T>
        void add_set();
        virtual bool check_updated(){
            return updated;
        }
};

template <class T>
void MF_BarManager::add_type(Uint16 w, Uint16 h)
{
#ifdef DEBUG_ON
    assert(nbMF == 0);
#endif
    type_cont.push_back(new MF_Type<T>);
    type_cont[type_cont.size()-1]->w = w;
    type_cont[type_cont.size()-1]->h = h;
    if (option_h < h)
        option_h = h;
}

template <class T>
void MF_BarManager::add_set()
{
    delete aff_data;
    aff_data = new T;
}

class MF_TextBar : virtual public MF_DataBar
{
    public:
        const char *texte;
        virtual MF_DataBar* init(Uint16 w, Uint16 h, void *param)
        {
            texte = (const char*)param;
            return MF_DataBar::init(w, h, param);
        }
        virtual void affiche(MF_BarManager *boss, Rect &pos_barre);
        virtual int compare(MF_DataBar *member2)
        {
            MF_TextBar *it = dynamic_cast<MF_TextBar*>(member2);
#ifdef DEBUG_ON
            assert(it != NULL);
#endif
            return strcmp(texte, it->texte);
        }
        MF_TextBar(){};
        ~MF_TextBar(){};
};

class MF_ImageBar: virtual public MF_DataBar
{
    public:
        Surface image;
        virtual MF_DataBar* init(Uint16 w, Uint16 h, void *param)
        {
            image = *((Surface *)param);
            return MF_DataBar::init(w, h, param);
        }
        virtual void affiche(MF_BarManager *boss, Rect &pos_barre);
        virtual int compare(MF_DataBar *member2)
        {
            MF_ImageBar *it = dynamic_cast<MF_ImageBar*>(member2);
#ifdef DEBUG_ON
            assert(it != NULL);
#endif
            if (image.s < it->image.s) return -1;
            if (image.s == it->image.s) return 0;
            return 1;
        }
        MF_ImageBar(){};
        ~MF_ImageBar(){};
};

//alert - style javascript
class MF_Alert : virtual public MF_MoveAbleBoss, virtual public MF_Prio
{
    public:
        MF_TButton *ok;
        MF_Applet *inside;

        //Constructeur
        MF_Alert(MF_Boss *src, Font &police, const char *texte, const Color &bgcolor = Color(0xDD,0xDD,0xDD));
        ~MF_Alert();

        virtual void affiche(Surface &surface);
        virtual bool recoitMessage(const char *message, MF_Base *fenetre);
        virtual void move(Sint16 x, Sint16 y)
        {
            allouer(inside);
            MF_Boss::move(x, y);
            desallouer(inside);
        }
};

class MF_TrayBar : virtual public MF_Directions, virtual public MF_Surf
{
    public:
        Surface MF_Tray_Icon;
        Sint16 pos_icon;
        Uint16 num_vals;
        Uint16 pas;
        Sint16 min_val;
        static const int height = 15;
        Sint16 pos_clic_x;
        //bgColor is used as inside-filling color
        Color void_color;
        Sint16 lastval;
        Color colorkey;
        bool clicOn;
        bool clicAble;

        MF_TrayBar(Sint16 x, Sint16 y, Uint16 w, Uint16 numvals, const Color &fcolor = Color(0,0,0), Uint16 step = 1, Sint16 min_val = 0, bool clicAble = true);
        ~MF_TrayBar();

        virtual void actualiser();
        virtual void affiche(Surface &surface);
        virtual bool gereEvenement(const SDL_Event &event);
        virtual Sint16 get_posx() {
            return ((pos_icon-min_val)*(dims.w-MF_Tray_Icon.w()+1))/num_vals;
        }

        virtual bool check_updated (){
            return pos_icon == lastval;
        }
};

class MF_ListeDeroulante : public MF_BarManager
{
    public:
        bool deroulee;
        int small_height;
        int big_height;

        MF_ListeDeroulante(Uint16 w, Uint16 sh, Uint16 bh, Sint16 x, Sint16 y, Font &font);
        MF_ListeDeroulante(Uint16 w, Uint16 sh, Uint16 bh, Sint16 x, Sint16 y, const char *font_path, Uint8 ptsize);

        virtual bool gereClic(const SDL_MouseButtonEvent &bevent);
        virtual bool gereTouche(const SDL_KeyboardEvent &kevent);
        virtual bool dedans(int x, int y) const;
        bool change_deroulee(bool deroul);
        virtual void setPos(bool prems)
        {
            if (prems == false)
                change_deroulee(false);

            MF_BarManager::setPos(prems);
        }
};

class MF_Radio : public MF_Applet
{
    public:
        bool selected;
        static const Color drawing_color;

        MF_Radio(Font &police, const char *texte, bool selected, Sint16 x, Sint16 y, const Color &colorkey = Color(222,39,208));
        virtual void setPos(bool prems){
            MF_Base::setPos(prems);
            envoieMessage("select");
            set_updated();
        }
        virtual void affiche(Surface &surf);
        virtual bool check_updated() {
            return selected == isFirst;
        }
        /* PRIVATE */
        virtual int drawRadio();
};

class MF_BRadio : public MF_Boss
{
    public:
        /* ATTENTION!! Les autres méthodes (allouer, ...)
           peuvent être considérées comme private, faites très gaffe! */
        virtual MF_Base *add_option(Font &police, const char *texte, Uint16 role);
        virtual bool recoitMessage(const char *message, MF_Base *fenetre);
};

class MF_CheckBox : public MF_Applet
{
    public:
        bool checked;
        bool updated;

        MF_CheckBox(Font &police, const char *texte, bool checked, Sint16 x, Sint16 y, const Color &colorkey = Color(222,39,208));
        virtual void affiche(Surface &surf);
        virtual bool check_updated() {
            return updated;
        }
        virtual void set_check(bool checked);
        virtual void set_updated();
        /* PRIVATE */
        virtual int drawCross();
        virtual bool gereEvenement(const SDL_Event &event);
};
#endif
