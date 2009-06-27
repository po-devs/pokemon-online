#ifndef MF_HH
#define MF_HH

#ifndef MF_RETOUR_LIGNE
 #define MF_RETOUR_LIGNE
#endif
#ifndef MF_TEXT_ACCENTUE
 #define MF_TEXT_ACCENTUE
#endif

#include <cassert>
#include <iostream>
#include <cstring>

#include "intervideo.hpp"
#include "ressource_manager.hpp"

using namespace interface;

#ifndef TRACE
 #define TRACE(ARG) cout << #ARG << endl; ARG
#endif
#ifndef MIN
 #define MIN(a, b) (a < b ? a : b)
#endif
#ifndef MAX
 #define MAX(a, b) (a > b ? a : b)
#endif
#ifndef SHIFT_ON
 #define SHIFT_ON (SDL_GetModState() & KMOD_SHIFT)
#endif
#ifndef CTRL_ON
 #define CTRL_ON (SDL_GetModState() & KMOD_CTRL)
#endif
#ifndef ALT_ON
 #define ALT_ON (SDL_GetModState() & KMOD_ALT)
#endif
#ifndef BUTTON_LEFT_ON
 #define BUTTON_LEFT_ON (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))
#endif

/* Le but de cet utilitaire est de creer des 'fenetres'
 *
 * En fait tout se passe dans la fenêtre principale, et à l'interieur
 * d'autres simili de fenetres sont creees. Il s'agit d'une multitude
 * de classes derivant toutes d'une classe hyperbasique (qu'on pourrait
 * mettre en abstrait vu que c'est un simple rectangle).
 *
 * Les fenetres doivent se comporter de facon autonome, c'est a dire
 * que le code du programme doit y resider, dans la gestion des evenements.
 *
 * C'est a dire que l'utilisateur devra lui meme creer ses propres sous-classe */

/* Toutes les fenetres passent par une fenetres qui les gerent, et une architecture
 * a plusieurs niveaux peuvent se construire. D'ailleurs ce qui sera au final utilisé
 * ne sera probablement que des aggregations de fenetres basiques qui donnent
 * ensemble un look sympa (une pour displayr le texte, une pour le bouton oui, une
 * pour le non, et voila une fenetre alert ;)) */

/* Pour éviter de charger deux fois des BMPs, polices ... */
static ImageManager ImageMan("db/");
static FontManager FontMan("db/");

/* La classe qui gere des sous-fenetres */
class MF_Boss;

/* fenetre basique de chez basique... */
class MF_Base
{
    public:
        /* Pointeurs de la liste chainée */
        MF_Base *pPrev;
        MF_Base *pNext;
        /* Vers la classe SUP */
        MF_Boss *pSup;
        /* ID et role, utilisés par la classe SUP */
        /* NE CHANGEZ PAS L'ID VOUS MEME OK? */
        Uint16 ID;
        Uint16 role;
        //si souris par dessus
        bool hovered;
        //si en premier
        bool isFirst;

        /* Couleur de fond */
        Color bgColor;
        /* position et dimensions */
        Rect dims;

        /** Fonctions membres **/
        /** @name   MF_Base
            @brief  Initialise le rectangle et la couleur BG
            @param  x  dims.x
            @param  y  dims.y
            @param  w  dims.w
            @param  h  dims.h
            @param  r  bgColor.r
            @param  g  bgColor.g
            @param  b  bgColor.b
            */
        MF_Base(Sint16 x = 0, Sint16 y = 0, Uint16 w = 0, Uint16 h = 0, const Color &c = 0);
        /** @name   ~MF_Base
            @brief  Destructeur qui détruit l'élément d'après */
        virtual ~MF_Base();

        /** @name   resize
            @brief  A appeler plutôt que setRect pour un redimensionnement
            @param  w  nouvelle largeur
            @param  h  nouvelle hauteur
            */
        virtual void resize(Uint16 w, Uint16 h);
        /** @name   move
            @brief  A appeler plutot que setRect pour bouger la fenêtre
            @param  x  nouvelle abscisse
            @param  y  nouvelle ordonnée
            */
        virtual void move(Sint16 x, Sint16 y);
        /** @name   setRect
            @brief  La première fois, met les bonnes dimensions à la fenêtre
            @param  x  abscisse
            @param  y  ordonnée
            @param  w  largeur
            @param  y  hauteur
            */
        virtual void setRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h);
        /** @name   setColor
            @brief  change la couleur de fond
            @param  r  teneur en rouge
            @param  g  teneur en vert
            @param  b  teneur en bleu
            */
        void setColor(const Color &c);
        /** @name   display
            @brief  display sur la surface passée en paramètre
            @param  surface  La surface sur laquelle on display
            @return  le code d'erreur de SDL (0: ca va, -1 et -2: ca craint) */
        virtual void display(Surface &surface){;};
        /** @name   deal_w_Event
            @brief  Gère un évènement SDL
            @param  event  L'évènement en question
            @return  false si l'évènement n'est pas géré, true s'il l'est totalement

            Active une fonction virtuelle lorsque la souris bouge, pour dire si
            elle survole la fenêtre */
        virtual bool deal_w_Event(const SDL_Event &event){
            return false;
        }
        /** @name   sendToBoss
            @brief  envoie le message donné à la fenêtre qui gère tout
            @param  message  le message à envoyer */
        bool sendToBoss(const char *message);
        /** @name   isIn
            @brief  vérifie si les coordonnées sont dans la fenêtre
            @param  x  abscisse
            @param  y  ordonnée
            @return  true ou false, si les coordonnées appartiennent à la fenêtre */
        virtual bool isIn(int x, int y) const;
        /** @name   set_hover_state
            @brief  change @c hover
            @param  enable  -1: @c hover inchangé, 0: @c hover passe à true, 1: @c hover pass à false
            @return  true si modification, ou @c hover si @c enable vaut -1 */
        virtual int set_hover_state(int enable) //-1 pour le laisser inchangé, 0 pour l'annuler et 1 pour le mettre
        {
            //retourne true si modification
            if (enable != -1)
            {
                return (hovered != (hovered = (enable != 0)));
            }
            //ou l'état du hover
            return hovered;
        }
        /** @name   setPos
            @brief  change @c isFirst
            @param  prems  @c isFirst devient @c prems */
        virtual void setPos(bool prems) { isFirst = prems;}
        /** @name   check_updated
            @brief  sert à vérifier si besoin d'un nouvel affichage
            @return true si besoin, false sinon

            Sert essentiellement pour les gestionnaires avancés. Si tous les updated sont true alors pas
            besoin d'displayr! Les gestionnaires avancés ne vérifient que celui de la première fenêtre cependant,
            pour forcer un réaffichage c'est set_updated() */
        virtual bool check_updated() { return true;}
        /** @name   set_updated
            @brief  méthode bourrin pour tout rédisplayr */
        virtual void set_updated();
};

/* Classe Sup!!
 * Elle gere les autres en liste chainee */
class MF_Boss : virtual public MF_Base
{
    public:
        /* Sous-fenetre */
        MF_Base *pStart;
        MF_Base *pEnd;
        /* nombre de sous-fenetres */
        Uint16 nbMF;
        /* updated -- non nécessaire mais permet un gain de CPU énorme */
        bool updated;

        /** Fonctions membres */
        MF_Boss();
        /* Destructeur */
        ~MF_Boss();

        /* utilisée internallement */
        /** @name   getfreeID
            @brief  Donne une ID non utilisee par les sous-fenetres
            @return  L'ID en question */
        Uint16 getfreeID() const;
        /* gestion de fenetres */
        /** @name   allocate
            @brief  Integre la MF_Base donnée en première position dans la liste chainée
            @return La MF_Base en question */
        virtual MF_Base * allocate(MF_Base *fenetre);
        /** @name   reallocate
            @brief  Integre la MF_Base donnée en première position dans la liste chainée
            @return La MF_Base en question

            Contrairement à allocate(), cettre fonction délie la MF_Base donnée de son précédent propriétaire*/
        virtual MF_Base * reallocate(MF_Base *fenetre);
        /** @name   desallocate
            @brief  Retire la MF_Base donnée de la liste chainée, et coupe tout ses liens avec la liste
            @return La MF_Base passée en paramètre */
        virtual MF_Base * desallocate(MF_Base *fenetre);
        /* Creation+allocation / deallocation+Destruction */
        /** @name   creerMF
            @brief  Créer (constructeur par défaut) et alloue une MF_Base du type donné
            @return La MF_Base passée en paramètre */
        template<class T>
        MF_Base *creerMF();
        /** @name   destroyMF
            @brief  désalloue et détruit la MF_Base passée en paramètre */
        void destroyMF(MF_Base *fenetre);
        /* pour en mettre une en premier (pole position)*/
        /** @name   polepos
            @brief  Met la MF_base passée en paramètre en première position dans la liste */
        virtual void polepos(MF_Base *fenetre);
        /* pour avoir le n-ième membre, commençant à 0 */
        /** @name   get_member
            @brief  retourne le i-ème membre, c-à-d liste[i], si on était dans un tableau */
        virtual MF_Base * get_member(int index);

        /* autres... */
        /** @name   move
            @brief  se déplace au coordonnées indiquées, ainsi que toutes les sous-MF */
        virtual void move(Sint16 x, Sint16 y);

        /* affichage */
        /** @name   displayMF
            @brief  declenche l'affichage des sous-MF sur surface, en partant de pEnd
            @return 0 si aucun problème, un nombre négatif autrement

            Cette fonction existe pour faciliter l'héritage, en effet pour les classes héritant de
            MF_Boss et d'une autre classe MF_Lambda, la fonction display finale sera le plus souvent:

            X::display(SDL_Surface *surface)
            {
                int status = MF_Lambda::display(surface);
                if (status != 0) {
                    return status;
                }
                return displayMF(surface)
            }
            */
        int displayMF(Surface &surface);
        /** @name   display
            @brief  Appelle MF_Base::display et displayMF. */
        virtual void display(Surface &surface);
        /* gestion d'évènements, appelle la bonne fonction pour
           gérer l'évènement */
        virtual bool deal_w_Event(const SDL_Event &event);
        /* gestion d'évènement si l'évènement concerne CETTE fenêtre,
           et pas une sous fenêtre */
        virtual int evntPerso(const SDL_Event &event);
        /* reception de messages */
        /** @name   RecvFromSub
            @brief  gere un message envoyé par une fenêtre
            @param  message  Le message envoyé
            @param  fenetre  La fenetre qui envoie @c message
            @return true si @c message est géré/accepté, false sinon.

            Au lieu de pointeurs de fonctions, etc.. j'ai choisi d'utiliser des messages pour
            communiquer de la fenêtre de base vers son gérant. Ainsi, vu que SDL a une programmation
            évènementielle, le corps du programme sera dans les fonctions RecvFromSub de vos classes
            que vous hériterez de MF_Boss, indirectement ou pas

            L'utilité du booléen de retour? La fenetre qui envoie le message voit ce booléen,
            par exemple admettons que nous ayons une textbox T. L'utilisateur appuie sur entrée,
            donc T envoie le message 'enter', puis le message 'tab'. Dans certains cas, (par exemple contenu invalide),
            on ne veut pas changer de fenetre (et donc ne pas recevoir le message 'tab'),

            Il suffit alors à RecvFromSub de retourner false. La textbox n'enverra pas le message 'tab'*/
        virtual bool RecvFromSub(const char *message, MF_Base *fenetre){return false;}
        /* check_updated  -- on utilise la variable updated */
        virtual bool check_updated() { return updated && pStart->check_updated();}
        /* cette fois on considère aussi la variable updated */
        virtual void set_updated()
        {
            updated = false;
            if (pSup != NULL) pSup->set_updated();
        }
};

//pour donner des directions : genre tab va au prochain, aussi les touches directionnelles
class MF_Directions : virtual public MF_Base
{
    public:
    MF_Base *left, *right, *up, *down, *tab;

    MF_Directions(Sint16 x = 0, Sint16 y = 0, Uint16 w = 0, Uint16 h = 0, const Color &c = 0)
        : MF_Base(x,y,w,h, c), left(NULL), right(NULL), up(NULL), down(NULL), tab(NULL) {}
    ~MF_Directions(){}

    /* On gère les touches différement (gauche, droite, tab, ...)
        On envoie le message correspondant (left, right, tab, up..) au gérant */
    virtual bool deal_w_Event(const SDL_Event &event);
    virtual bool deal_w_key(const SDL_KeyboardEvent &event);
    /* Pour régler les fenetres liées à celle-ci */
    virtual void setDirections(MF_Base *tab = NULL, MF_Base *up = NULL, MF_Base *down = NULL, MF_Base *left = NULL, MF_Base *right = NULL);
};

//Pour gérer les directions:
class MF_BDirections : virtual public MF_Boss
{
    public:
    MF_BDirections():MF_Boss(){}
    ~MF_BDirections(){}

    /** @name   RecvFromSub
        @brief  Cette fonction change de pStart en fonction du message reçu(tab, up, ...)
        @return true si la direction est effectivement changée, false sinon

        Par exemple, si elle recoit le message 'down', que fenetre est de classe dérivée de MF_Direction,
        et que fenetre->down != NULL, alors la fonction fait un polePos sur fenetre->down */
    virtual bool RecvFromSub(const char* message, MF_Base *fenetre);
    /* en fait, tout ce qui est décrit ci-dessus, c'est cette fonction qui le gère. */
    virtual bool gereDirection(const char *message, MF_Base *fenetre);
};

//pour les MF qui ont une surface en mémoire:
//et le nécessaire pour gérer la surface
class MF_Surf : virtual public MF_Base
{
    public:
    Surface surface;

    MF_Surf();
    ~MF_Surf();

    //nouvelles fonctions setRect et resize, en gérant la surface
    virtual void resize(Uint16 w, Uint16 h);
    virtual void setRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h);
    //fonction pour displayr
    virtual void display(Surface &surface);
};

//Pour les fenêtres pouvant se déplacer
class MF_MoveAbleBoss : virtual public MF_Boss
{
    public:
        //Si on est cliqué
        bool clicOn;
        //Les coordonnées d'où la souris a cliqué
        Uint16 pos_clic_x, pos_clic_y;

        MF_MoveAbleBoss() : clicOn(0) {}
        ~MF_MoveAbleBoss() {}

        //Une nouvelle fonction pour gérer les évènements
        virtual bool deal_w_Event(const SDL_Event &event);
        virtual int evntPerso(const SDL_Event &event);
};

//Fenêtres comme les alerts, qui nous empêchent de cliquer derrière
class MF_Prio : virtual public MF_Base
{
    public:
        //La prio est toujours en premier
        virtual bool isIn(int x, int y) const {return true;}
};

template<class T>
MF_Base *MF_Boss::creerMF()
{
    /* création d'un MF_Base* du type de T */
    MF_Base* fenetre = static_cast<MF_Base*>(new T);

    /* et finalement allocation ^^ */
    return allocate(fenetre);
}

/* Constructeur !! */
inline MF_Base::MF_Base(Sint16 x, Sint16 y, Uint16 w, Uint16 h, const Color &c)
        : pPrev(NULL), pNext(NULL), pSup(NULL), hovered(false), bgColor(c)
{
    setRect(x,y,w,h);
}

/* destructeur */
inline MF_Base::~MF_Base()
{
    /* on détruit le maillon suivant ^^ */
    delete pNext;
}

/* envoie un message à la classe SUP */
inline bool MF_Base::sendToBoss(const char *message)
{
    if (pSup != NULL)  return pSup->RecvFromSub(message, this);
    return false;
}

inline bool MF_Base::isIn(int x, int y) const
{
    return (x>=dims.x && y>=dims.y && x<dims.w+dims.x && y<dims.y+dims.h);
}

inline void MF_Base::set_updated()
{
    if (pSup != NULL) pSup->set_updated();
}

/* On se contente du constructeur de MF_Base */
inline MF_Boss::MF_Boss()
        :pStart(NULL), pEnd(NULL), nbMF(0), updated(false)
{
}

inline MF_Boss::~MF_Boss()
{
    /* Ac la liste chainée, on détruit tout ^^ */
    delete pStart;
}

/* On considère que la fenêtre appartient à une autre
   classe donc on la desalloue et l'alloue normalement */
/* C'est *légèrement* plus rapide que desallocate puis allocate */
inline MF_Base * MF_Boss::reallocate(MF_Base *fenetre)
{
    fenetre->pSup->desallocate(fenetre);
    return allocate(fenetre);
}

/* gestion d'évènement si l'évènement concerne CETTE fenêtre,
    et pas une sous fenêtre */
inline int MF_Boss::evntPerso(const SDL_Event &event)
{
    return false;
}

/* Trop simmple ^^ */
inline void MF_Boss::destroyMF(MF_Base *fenetre)
{
    desallocate(fenetre);
    delete fenetre;
}

inline bool MF_Directions::deal_w_Event(const SDL_Event &event)
{
    if (event.type != SDL_KEYDOWN) return MF_Base::deal_w_Event(event);
    return this->deal_w_key(event.key);
}

inline MF_Surf::MF_Surf()
{
}

inline bool MF_BDirections::RecvFromSub(const char *message, MF_Base *fenetre)
{
    return gereDirection(message, fenetre);
}

#endif
