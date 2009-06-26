#ifndef MF_TEXT_HH
#define MF_TEXT_HH

#include "MF.hh"

#include "interfont.hpp"
#include <string>
#include <vector>
#include "SDL_gfx.h"
#include <SDL/SDL_image.h>

/* Dans la lignées des Multi-fenêtres voici celles
   qui affichent du texte - 1 ligne seulement */

class MF_Ligne : virtual public MF_Surf
{
    public:
        /* texte contenu*/
        std::string texte;

        bool updated;

        /* police */
        Font police;
        /* couleur du texte */
        Color textColor;

        MF_Ligne();
        ~MF_Ligne();

        /* Met une couleur de texte */
        void setTextColor(const Color &c = 0);

        /* Pour balancer du texte */
        virtual void ecrire(const std::string &texte, int pos);
        virtual void ecrire(const char* texte, int pos);
        virtual void ecrire(char texte, int pos);
        virtual void ecrire(const std::string &texte);
        virtual void ecrire(const char* texte);
        virtual void ecrire(char texte);

        /* pour savoir la pos en nombre de lettres d'une abscisse */
        virtual int  toLen(int w) const;

        /* Pour en effacer */
        virtual void effacer(int depart = 0);
        virtual void effacer(int depart, int nombre);

        /* Pour afficher ^^ */
        virtual void affiche(Surface &surface);

        /* Pour actualiser la surface en mémoire */
        virtual void actualiser();

        /* overload */
        virtual bool check_updated (){ return updated;}
        virtual void set_updated()
        {
            if (pSup != NULL) pSup->updated = false;
            updated = false;
        }

        void setFont(const char *path, Uint16 size);
        void shareFont(Font &police);
};

//juste pour aider
struct xy
{
    int x;
    int y;

    xy(int x=0, int y=0);
};

bool operator > (xy &a, xy &b);
bool operator < (xy &a, xy &b);
bool operator == (xy &a, xy &b);

/* C'est du multiligne cette fois.
   on progresse hein ? avec une barre déroulante en +*/

class MF_MLigne : virtual public MF_Ligne
{
public:
    int capacity;
    unsigned int ncurrentligne;

    /* un vecteur pour donner les pos des lignes (début et fin) */
    std::vector<xy> poslignes;

    /* Maintenant barre déroulante */
    Surface flechehaut;
    Surface flechebas;
    bool barreOn;

    /* pour savoir si on a cliqué sur la barre */
    bool barre_clicOn;

    /* et la pos relative du clic en hauteur par rapport au haut de la fenetre */
    int  barreclic_y;

    /* timerID, quand on clique sur une flèche ou la partie vide de la zone de la barre
       déroulante */
    SDL_TimerID timer_ID;
    bool timer_wasinit;
    //pos clic
    Sint16 posx;
    Sint16 posy;

    /* constructeur et patati ...*/
    MF_MLigne();
    ~MF_MLigne();

    /* Les fonctions pour afficher */
    /* lors du test je les avais oublié... :( */
    virtual void affiche(Surface &ecran) {MF_Ligne::affiche(ecran);}

    /* Redessine tout le texte */
    virtual void actualiser();

    /* pour avoir le nombre de lignes */
    virtual unsigned int get_nblignes() const;

    /* largeur max du texte */
    virtual int get_largeurmax() const;

    /*pour se changer en cas de trop plein de lignes */
    virtual void update_capacity();

    /* pour changer la capacité */
    virtual void setcapacity(int capacite);

    /* Pour ajouter une barre si trop de lignes ou au contraire l'enlever */
    virtual void update_barre();

    /* Pour avoir le num de caractères jusqu'à une largeur particulière */
    virtual int toLen(int noLigne, int w) const;

    /* récupérer la pos d'un caractère à partir de son offset */
    virtual xy get_abspos(int offset);
    /* récupérer la pos pour écrire après le dernier caractère */
    virtual xy get_abspos() const;

    /* récuperer l'offset d'un caractère à partir de sa pos */
    virtual int get_offset(int x, int y) const;//no de ligne + curseury
    virtual int get_offset(const xy &abspos, int y) const; //ligne + curseury
    virtual int get_offset(const xy &abspos) const;//fin de ligne
    virtual int get_offset(int x) const;//debut de ligne
    /* récupérer l'offset pour écrire après le dernier caractère */
    virtual int get_offset() const;

    /* nouvelles fonctions pour écrire */
    virtual void ecrire (const std::string &texte, unsigned int posx, int posy);
    virtual void ecrire (const char *texte, unsigned int posx, int posy);
    virtual void ecrire (char texte, unsigned int posx, int posy);
    virtual void ecrire (const std::string &texte) {ecrire(texte, get_nblignes()-1, poslignes[get_nblignes()-1].y);}
    virtual void ecrire (char texte) {MF_Ligne::ecrire(texte);}
    virtual void ecrire (const char *texte) {MF_Ligne::ecrire(texte);}

    /* pour adapter le contenu des lignes en fonction de la largeur maximale */
    virtual void update_largeur(unsigned int noligne, bool suivantes);

    /* gestion évènements pour faire defiler texte */
    virtual bool gereEvenement(const SDL_Event &event);
    virtual bool gereMotion(const SDL_MouseMotionEvent &motionevent);
    virtual bool gereTouche(const SDL_KeyboardEvent &keyboardevent);
    virtual bool gereClic(const SDL_MouseButtonEvent &buttonevent);

    /* faire monter la barre d'un cran */
    virtual bool montebarre(int crans);
    virtual bool descendbarre(int crans);

    /* pour s'aider */
    virtual int maxlignes() const {return dims.h/police.line_skip();}
    virtual int getbarreh() const {return dims.h/3;}
    virtual int getbarrey(int barreh) const;
    virtual void affichebarre(int barrey, int barreh);
    //A cause du timer
    virtual void resize(Uint16 w, Uint16 h)
    {
        if (timer_wasinit == true)
        {
            SDL_RemoveTimer(timer_ID);
            timer_wasinit = false;
        }
        MF_Surf::resize(w, h);
    }
    virtual void effacer(int depart, int nombre=-1);
};

Uint32 MF_MLigne_fonction_timer(Uint32 interval, void *param);

/* On peut faire plusieurs choses:
   ecrire dedans avec un curseur,
   ou juste utiliser la souris pour selectionner du
   texte. Toujours uniligne */

class MF_WLigne : virtual public MF_Ligne
{
    public:
        /*special*/
        bool second_update;
        /* Trucs normaux */
        int curseury; //position du curseur ^^
        bool ecrivable; //Si on peut écrire
        bool curseurOn; //Si on peut afficher le cuseur
        /* Maintenant pour le surlignage */
        bool clicOn;
        bool surligneOn; //Si highlight on
        int curseuryDeb; //Si highlight, on retient la pos de début
        bool curseurDisp; //etat du curseur
        Uint32 lastCurseur; //Dernier affichage bien du curseur

        MF_WLigne();
        ~MF_WLigne();

        /* Gere clic, motion et touche */
        virtual bool gereEvenement(const SDL_Event &event);
        virtual bool gereTouche(const SDL_KeyboardEvent &keyevent);
        virtual bool gereClic(const SDL_MouseButtonEvent &buttonevent);
        virtual bool gereMotion(const SDL_MouseMotionEvent &motionevent);

        /* Pour ecrire du texte */
        virtual void ecrire(const std::string &texte, int pos);
        virtual void ecrire(const char* texte, int pos);
        virtual void ecrire(char texte, int pos);
        virtual void ecrire(const std::string &texte) {ecrire(texte, curseury);}
        virtual void ecrire(const char* texte) {ecrire(texte, curseury);}
        virtual void ecrire(char texte) {ecrire(texte, curseury);}

        /* effacer là ou c'est surligné - retourne true si modification*/
        virtual bool clearHighlight();
        /* commencer si possible le higlight */
        virtual void startHighlight();
        /* voir si on doit enlever le highlight, ...  -- retourne true si modification*/
        virtual bool updateHighlight();

        /* afficher et actualiser */
        virtual void affiche(Surface &ecran);
        virtual void actualiser();
        /* pour changer le highlight */
        virtual void setPos(bool prems);
        virtual bool check_updated() { return updated && SDL_GetTicks()-lastCurseur < 500 && second_update;}
};

/* c'est une combinaison du texte unilignes pour écrire (MF_uwtext) et du texte
   multilignes (MF_mtext).
   On peut donc écrire sur plusieurs lignes :) */

class MF_MWLigne : virtual public MF_MLigne, virtual public MF_WLigne
{
public:
    /* il reste le curseurx à gérer */
    unsigned int curseurx;
    unsigned int curseurxDeb;
    /* et c'est tout :) */

    /** Fonctions **/
    /*constructeur et destructeur*/
    MF_MWLigne();
    ~MF_MWLigne();

    /* affiche et actualise la surface en mémoire */
    virtual void affiche(Surface &ecran);
    virtual void actualiser();

    /* nouvelles fonctions pour écrire */
    /* elles updatent aussi le curseur :) */
    virtual void ecrire (const std::string &texte, unsigned int posx, unsigned int posy);
    virtual void ecrire (const char *texte, unsigned int posx, unsigned int posy);
    virtual void ecrire (char texte, unsigned int posx, unsigned int posy);
    virtual void ecrire (const std::string &texte){ecrire(texte, curseurx, curseury);}
    virtual void ecrire (char texte){ecrire(texte, curseurx, curseury);}
    virtual void ecrire (const char *texte){ecrire(texte, curseurx, curseury);}
    /* gestion évènements pour faire defiler texte et écrire */
    virtual bool gereEvenement (const SDL_Event &event) {return MF_MLigne::gereEvenement(event);}
    virtual bool gereMotion(const SDL_MouseMotionEvent &motionevent);
    virtual bool gereTouche(const SDL_KeyboardEvent &keyboardevent);
    virtual bool gereClic(const SDL_MouseButtonEvent &buttonevent);
    /* pour monter le curseur ou le descendre :) */
    virtual void montecurseur(int crans);
    virtual void descendcurseur(int crans);
    // pour replacer la barre en fonction du curseur
    virtual void rebarre();
    virtual void updatebarre();
    //highlight
    virtual bool clearHighlight();
    virtual void startHighlight();
    virtual void effacer(int depart, int nombre=-1);
    //debug
    void printstatut();
    //diff /=> mtext: efface dernieres lignes au lieu des premières
    virtual void update_capacity();
    //curseur
    virtual void set_abspos_curseur(int pos);
    virtual int get_offset_curseur();
};

inline MF_Ligne::MF_Ligne()
        :updated(false)
{
    setTextColor(0xFF);
}

inline void MF_Ligne::ecrire(const char* texte, int pos)
{
    std::string tmp = texte;
    ecrire(tmp, pos);
}

inline void MF_Ligne::ecrire(char texte, int pos)
{
    std::string tmp = "";
    tmp.push_back(texte);
    ecrire(tmp, pos);
}

inline void MF_Ligne::ecrire(const std::string &texte)
{
    ecrire(texte, this->texte.length());
}

inline void MF_Ligne::ecrire(const char* texte)
{
    std::string tmp = texte;
    ecrire(tmp);
}

inline void MF_Ligne::ecrire(char texte)
{
    std::string tmp = "";
    tmp.push_back(texte);
    ecrire(tmp);
}

/* Pour en effacer */
/* efface tout à partir de depart */
inline void MF_Ligne::effacer(int depart)
{
    texte.erase(depart);
    set_updated();;
}
/* et nombre de caractère à partir de départ */
inline void MF_Ligne::effacer(int depart, int nombre)
{
    texte.erase(depart, nombre);
    set_updated();;
}

/* Pour afficher */
inline void MF_Ligne::affiche(Surface &surface)
{
    /* On fait juste un blit */
    if (!updated) actualiser();
    this->surface.blitTo(surface, 0, dims);
}

inline MF_WLigne::MF_WLigne()
        :second_update(1), curseury(0), ecrivable(true), curseurOn(true), clicOn(false),
        surligneOn(false), curseurDisp(true), lastCurseur(SDL_GetTicks())
{
}

/* destructeur */
inline MF_WLigne::~MF_WLigne()
{
    /* rien de spécial */
}

inline void MF_WLigne::ecrire(const char* texte, int pos)
{
    std::string tmp = texte;
    ecrire(tmp, pos);
}

inline void MF_WLigne::ecrire(char texte, int pos)
{
    std::string tmp = "";
    tmp.push_back(texte);
    ecrire(tmp, pos);
}

inline void MF_MLigne::ecrire (const char *texte, unsigned int posx, int posy)
{
    std::string tmp(texte);
    ecrire(tmp, posx, posy);
}

inline void MF_MLigne::ecrire (char texte, unsigned int posx, int posy)
{
    std::string tmp("");
    tmp.push_back(texte);
    ecrire(tmp, posx, posy);
}

inline void MF_MLigne::setcapacity(int capacite)
{
    #ifdef DEBUG_ON
    assert(capacite > 0);
    #endif
    capacity = capacite;
    update_capacity();
}

inline unsigned int MF_MLigne::get_nblignes() const
{
    return poslignes.size();
}

inline int MF_MLigne::getbarrey(int barreh) const
{
    int maxlines = maxlignes();
    int difference = get_nblignes() - maxlines;

    return 16 + ((dims.h - barreh - 32) * ncurrentligne) / difference;
}

inline int MF_MLigne::get_largeurmax() const
{
    return dims.w-(barreOn==true)*16;
}

/* récupérer la pos pour écrire après le dernier caractère */
inline xy MF_MLigne::get_abspos() const
{
    return xy(get_nblignes()-1, poslignes[get_nblignes()-1].y);
}

/* récuperer l'offset d'un caractère à partir de sa pos */
inline int MF_MLigne::get_offset(int x, int y) const //no de ligne + curseury
{
    return poslignes[x].x + y;
}

inline int MF_MLigne::get_offset(const xy &abspos, int y) const //ligne + curseury
{
    return abspos.x + y;
}

inline int MF_MLigne::get_offset(const xy &abspos) const //fin de ligne
{
    return abspos.x +abspos.y;
}

inline int MF_MLigne::get_offset(int x) const //debut de ligne
{
    return poslignes[x].x;
}

/* récupérer l'offset pour écrire après le dernier caractère */
inline int MF_MLigne::get_offset() const
{
    return get_offset(get_abspos());
}

inline xy::xy(int x, int y)
        :x(x), y(y)
{
}

inline bool operator == (xy &a, xy &b)
{
    return (a.x == b.x && a.y == b.y);
}

inline MF_MWLigne::MF_MWLigne()
        :curseurx(0), curseurxDeb(0)
{
}

//Rien du tout :D
inline MF_MWLigne::~MF_MWLigne()
{
}

inline void MF_MWLigne::ecrire (const char *texte, unsigned int posx, unsigned int posy)
{
    std::string tmp(texte);
    ecrire(tmp, posx, posy);
}

inline void MF_MWLigne::ecrire (char texte, unsigned int posx, unsigned int posy)
{
    std::string tmp;
    tmp.push_back(texte);
    ecrire(tmp, posx, posy);
}

inline int MF_MWLigne::get_offset_curseur()
{
    return poslignes[curseurx].x + curseury;
}

inline void MF_MWLigne::set_abspos_curseur(int pos)
{
    xy a = get_abspos(pos);
    curseurx = a.x;
    curseury = a.y;
}



#endif
