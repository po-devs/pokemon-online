#include "MF.hh"
#include "SDL_gfx.h"

using namespace std;

/* Change hauteur et largeur */
void MF_Base::resize(Uint16 w, Uint16 h)
{
    dims.w = w;
    dims.h = h;
    set_updated();

    if (pSup == NULL) return;
}

/* Bouge la fenetres */
void MF_Base::move(Sint16 x, Sint16 y)
{
    if (dims.x == x && dims.y == y) return;
    //SDL fait que l'affichage merdouille avec des vals négatives
    dims.x = MAX(x, 0);
    dims.y = MAX(y, 0);
    set_updated();
}

/* change de manière brute le rectangle de dimensions, sans
   aucun autre controle */
void MF_Base::setRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    set_updated();
    dims.x = x;
    dims.y = y;
    dims.w = w;
    dims.h = h;
}

/* Change la couleur de fond */
void MF_Base::setColor(const Color &c)
{
    set_updated();
    bgColor = c;
}

/** Classe Sup!! **/

/* utilisée internallement */
/* On recherche une ID de libre
   à donner.. */
Uint16 MF_Boss::getfreeID() const
{
    /* création d'un tableau pour dire si les ID sont prises ou pas */
    bool IDtable[nbMF];

    /* On remplit le tableau en parcourant tout */
    for (MF_Base *tmp = pStart; tmp != NULL; tmp = tmp->pNext)
    {
        if (tmp->ID < nbMF)
            IDtable[tmp->ID] = true;
    }

    /* Maintenant on cherche la première libre */

    for (Uint16 i=0; i < nbMF; ++i)
    {
        if (IDtable[i] == false)
            return i;
    }

    /* Si la recherche n'a pas été fructueuse, toutes les cases sont
       remplies; il y a autant de cases que de fenêtres. Donc celle
       d'après est forcément vide. */
    return nbMF;
}

/* gestion de fenetres */
/* on donne un place et met en premier une nouvelle fenetre donnée */
MF_Base * MF_Boss::allocate(MF_Base *fenetre)
{
    if (fenetre->pSup != false)
    {
        return NULL;
    }

    /* On augmente le nombre de fenetres (+1) */
    ++nbMF;

    /* On récupère une ID libre*/
    fenetre->ID = getfreeID();

    /* et on l'intègre en première position de la liste chainée */
    if (pStart != NULL)
    {
        pStart->pPrev = fenetre;
        pStart->setPos(false);
    }
    fenetre->pNext = pStart;
    pStart = fenetre;
    fenetre->setPos(true);
    if (pEnd == NULL)
        pEnd = fenetre;
    fenetre->pPrev = NULL;
    fenetre->pSup = this;

    set_updated();

    return fenetre;
}

/* désalloue en sorte que lors de la destruction
   de fenetre celles d'après ne soient pas entrainée
   par la liste chainée */
MF_Base * MF_Boss::desallocate(MF_Base *fenetre)
{
    //Si c'est pas la notre, ca peut entrainer des bugs, difficiles à localiser
    if (fenetre->pSup != this)
        return fenetre;
    /* réduction du nombre de fenêtres */
    --nbMF;

    /* reconstruction de la chaîne */
    if (pStart == fenetre)
    {
        pStart = fenetre->pNext;
        if (pStart != NULL)
            pStart->setPos(true);
    }
    else if (fenetre->pPrev != NULL)
        fenetre->pPrev->pNext = fenetre->pNext;
    if (pEnd == fenetre)
        pEnd = fenetre->pPrev;
    else if (fenetre->pNext != NULL)
        fenetre->pNext->pPrev = fenetre->pPrev;

    /* oubli des autres membres d'avant et d'après de la fenetre */
    fenetre->pPrev = NULL;
    fenetre->pNext = NULL;
    fenetre->pSup = NULL;

    set_updated();

    return fenetre;
}

/* On l'arrache de sa position et on la met en première */
void MF_Boss::polepos(MF_Base *fenetre)
{
    /* si déjà prems */
    if (pStart == fenetre)
        return;

    /* arrachement... */
    if (fenetre->pNext != NULL){
        fenetre->pNext->pPrev = fenetre->pPrev;
    }

    if (fenetre->pPrev != NULL)
    {
        fenetre->pPrev->pNext = fenetre->pNext;
        if (fenetre == pEnd)
            pEnd = fenetre->pPrev;
    }

    /* puis replacement */
    fenetre->pPrev = NULL;
    fenetre->pNext = pStart;
    pStart->setPos(false);
    fenetre->pNext->pPrev = fenetre;
    pStart = fenetre;
    pStart->setPos(true);
}
/* Avoir le n-ième membre */
MF_Base* MF_Boss::get_member(int index)
{
    MF_Base *it = pStart;

    while (index > 0 && it->pNext != NULL)
    {
        it = it->pNext;
        index --;
    }

    return it;
}

/* idem */
void MF_Boss::move(Sint16 x, Sint16 y)
{
    if (x == dims.x && y == dims.y) return;

    //MF_Base ne va pas forcément aux x et y indiqués, donc je sauvegarde les
    //ancienne pos, au lieu de faire ce que j'ai à faire et d'exécuter MF_Base
    //avant
    Sint16 old_x = dims.x;
    Sint16 old_y = dims.y;

    MF_Base::move(x, y);
    for (MF_Base *tmp = pStart; tmp != NULL; tmp=tmp->pNext)
    {
        tmp->move(dims.x-old_x+tmp->dims.x, dims.y-old_y+tmp->dims.y);
    }

    updated = false;
}

/* affichage */
void MF_Boss::display(Surface &surface)
{
    updated = true;

    /* On display d'abord soi-même */
    MF_Base::display(surface);

    /* Puis les petites fenêtres */
    displayMF(surface);
}

/* display seulement les sous-fenêtres
   ainsi lors de l'héritage cette fonction sera constante,
   ce qui évite de coller le morceau de code dans la fonction
   display à chaque fois en faisant un simple appel de fonction. */
int MF_Boss::displayMF(Surface &surface)
{
    /* boucle classique */
    /* Apres si vous voulez vous pouvez vous compliquer la
       vie pour n'displayr que ce qui est nécessaire (pas
       ce qui est caché) mais zzzzzzzzzzzzzzzzzzzzzzzz   */
    for (MF_Base *tmp = pEnd; tmp!=NULL; tmp=tmp->pPrev)
    {
        tmp->display(surface);
    }
    return 0;
}

/* gestion d'évènements */
bool MF_Boss::deal_w_Event(const SDL_Event &event)
{
    /* Si pas de fenêtres */
    if (pStart == NULL)
    {
        return evntPerso(event);
    }

    bool changed = false;

    switch(event.type)
    {
        /* si motion ou touche, c'est la première fenêtre qui en bénéficie */
        case SDL_MOUSEMOTION:
        //on fait le hover
        {
            bool finished = false;
            for (MF_Base *tmp = pStart; tmp != NULL; tmp = tmp->pNext)
            {
                if (!finished && tmp->isIn(event.motion.x, event.motion.y))
                {
                    changed |= tmp->set_hover_state(true);
                    finished = true;
                } else
                {
                    changed |= tmp->set_hover_state(false);
                }
            }
        }
        case SDL_MOUSEBUTTONUP:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return pStart->deal_w_Event(event) | changed;
        /* Pour le clic on cherche dans quelle fenêtre on a cliqué! */
        case SDL_MOUSEBUTTONDOWN:
        if (event.button.button != SDL_BUTTON_LEFT && event.button.button != SDL_BUTTON_RIGHT && event.button.button != SDL_BUTTON_MIDDLE) return pStart->deal_w_Event(event);
        for (MF_Base *tmp = pStart; tmp != NULL; tmp = tmp->pNext)
        {
            /* si isIn, on place la fenetre en pos 1 */
            if (tmp->isIn(event.button.x, event.button.y))
            {
                if (tmp == pStart)
                    return tmp->deal_w_Event(event);
                else
                {
                    polepos(tmp);
                    tmp->deal_w_Event(event);
                    return true;
                }
            }
        }
        /* Si aucune fenetre */
        changed = evntPerso(event);
        return pStart->deal_w_Event(event) | changed;
        default:
        return false;
    }
}

void MF_Directions::setDirections(MF_Base *tab, MF_Base *up, MF_Base *down, MF_Base *left, MF_Base *right)
{
    this->up = up;
    this->tab = tab;
    this->left = left;
    this->right = right;
    this->down = down;
}

bool MF_Directions::deal_w_key(const SDL_KeyboardEvent &event)
{
    switch (event.keysym.sym)
    {
        case SDLK_TAB:
            sendToBoss("tab");
            return 1;
        case SDLK_LEFT:
            sendToBoss("left");
            return 1;
        case SDLK_RIGHT:
            sendToBoss("right");
            return 1;
        case SDLK_UP:
            sendToBoss("up");
            return 1;
        case SDLK_DOWN:
            sendToBoss("down");
            return 1;
        case SDLK_RETURN:
            sendToBoss("enter");
            return 1;
        default:
            return 0;
    }
}

bool MF_BDirections::gereDirection(const char* message, MF_Base *fenetre)
{
    MF_Directions *fen = dynamic_cast<MF_Directions *>(fenetre);
    if (fen == NULL) return false; //on gère que les directions, pas les fenêtres normales
    //les messages right, left, down et up et tab permettent de changer de fenêtre
    if (strcmp(message, "right") == 0)
        if (fen->right == NULL) return false;
        else {
            polepos(fen->right);
            return true;
        }
    if (strcmp(message, "left") == 0)
        if (fen->left == NULL) return false;
        else {
            polepos(fen->left);
            return true;
        }
    if (strcmp(message, "up") == 0)
        if (fen->up == NULL) return false;
        else {
            polepos(fen->up);
            return true;
        }
    if (strcmp(message, "down") == 0)
        if (fen->down == NULL) return false;
        else {
            polepos(fen->down);
            return true;
        }
    if (strcmp(message, "tab") == 0)
        if (fen->tab == NULL) return false;
        else {
            polepos(fen->tab);
            return true;
        }
    //la commande ne concerne pas les directions
    return false;
}

MF_Surf::~MF_Surf()
{
}

void MF_Surf::display(Surface &surface)
{
    this->surface.blitTo(surface, 0, dims);
}

void MF_Surf::setRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    dims.x = x;
    dims.y = y;
    if (w != dims.w || h != dims.h)
    {
        resize(w, h);
    } else
    {
        set_updated();
    }
}

void MF_Surf::resize(Uint16 w, Uint16 h)
{
    /* On fait le boulot normal */
    MF_Base::resize(w, h);

    /* création de la nouvelle surface*/
    surface.create(8,w,h,SDL_HWSURFACE);
    surface.adapt();

    surface.fill(0, bgColor);
}

bool MF_MoveAbleBoss::deal_w_Event(const SDL_Event &event)
{
    //On recherche les evenements quand on est cliqué,
    //ou les clics sur nous, ce qui nous retombera dessus
    //avec le deal_w_Event
    clicOn &= BUTTON_LEFT_ON;
    if (!clicOn) return MF_Boss::deal_w_Event(event);
    return evntPerso(event);
}

int MF_MoveAbleBoss::evntPerso(const SDL_Event &event)
{
    //Si c'est un clic
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && SDL_IsInRect(&dims, event.button.x, event.button.y))
    {
        pos_clic_x = event.button.x - dims.x;
        pos_clic_y = event.button.y - dims.y;
        clicOn = true;

        pStart->deal_w_Event(event);
        return true;
    }

    //Si c'est une motion
    if (event.type != SDL_MOUSEMOTION || !clicOn)
        return false;

    move(event.motion.x - pos_clic_x, event.motion.y - pos_clic_y);
    return true;
}
