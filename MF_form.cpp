#include "MF_applet.hh"
#include <string.h>
#include "exception.hpp"

using namespace std;

MF_ArrowSet::MF_ArrowSet()
{
}

MF_ArrowSet::~MF_ArrowSet()
{
}

MF_SmallASet::MF_SmallASet()
{
    flechehaut = Man.LoadRessource("ffhaut.bmp");
    flechebas = Man.LoadRessource("ffbas.bmp");
    flechegauche = Man.LoadRessource("ffgauche.bmp");
    flechedroite = Man.LoadRessource("ffdroite.bmp");
    dim_fleches = flechehaut.h();
    bgColor = Color(0xDD, 0xFF, 0xDD);
    fillingColor = Color(0xEE, 0xEE, 0xEE);
    spacerColor = Color(0xFF, 0, 0);
    borderColor = Color(0,0,0);
}

MF_BigASet::MF_BigASet()
{
    flechehaut = Man.LoadRessource("Arrow2.png");
    flechebas = Man.LoadRessource("Arrow.png");
    dim_fleches = flechehaut.h();
    bgColor = Color(240, 208, 130);
    fillingColor = Color(65, 172, 205);
    spacerColor = Color(0xFF, 0xFF, 0xFF);
    borderColor = Color(1,58,209);
}

MF_TextBox::MF_TextBox(Font &police, Uint16 w, Uint16 h, int capacity, const char* texte, Sint16 x, Sint16 y, int offsetx, int offsety, bool enter, bool tab)
{
    this->enter = enter;
    this->tab = tab;
    shareFont(police);
    champ_texte.bgColor = Color(0xFF, 0xFF, 0xFF);
    champ_texte.textColor = Color(0, 0, 0);
    champ_texte.setRect(x+2+offsetx, y+2+offsety, w-4-offsetx, h-4-offsety);
    champ_texte.setcapacity(capacity);
    dims = Rect(x,y,w,h);
    if (police) champ_texte.ecrire(texte);
}

void MF_TextBox::affiche(Surface &ecran)
{
    ecran.fill(dims, Color(0xFF,0xFF,0xFF));

    champ_texte.affiche(ecran);

    //Bordures
    MF_Box_DrawBorders(dims, ecran);
}

bool MF_TextBox::gereEvenement(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN && !(SHIFT_ON))
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_RETURN:
                if (enter == true)
                    break;
                envoieMessage("enter");
                return 1;
            case SDLK_TAB:
                if (tab == true)
                    break;
                envoieMessage("tab");
                return 1;
            default:
                break;
        }
    }
    return champ_texte.gereEvenement(event);
}

void MF_Box_DrawBorders(const Rect &dims, Surface &ecran, bool inverted)
{
    Color c1, c2, c3, c4;
    if (inverted == false)
    {
        c1 = Color(0x22, 0x22, 0x22);
        c2 = Color(0x55, 0x55, 0x55);
        c3 = Color(0xBB, 0xBB, 0xBB);
        c4 = Color(0xFF, 0xFF, 0xFF);
    } else
    {
        c1 = Color(0xFF, 0xFF, 0xFF);
        c2 = Color(0xBB, 0xBB, 0xBB);
        c3 = Color(0x55, 0x55, 0x55);
        c4 = Color(0x22, 0x22, 0x22);
    }
    //Les 4 couleurs
    //barres horizontales
    Rect r (dims.x, dims.y, dims.w-1, 1);
    ecran.fill(r, c1);

    r.x++, r.y++, r.w -= 2;
    ecran.fill(r, c2);

    r.y = dims.y+dims.h-2, r.w++;
    ecran.fill(r, c3);

    r.x--, r.y++, r.w += 2;
    ecran.fill(r, c4);

    //barres verticales
    r.y = dims.y + 1, r.w = 1, r.h = dims.h-2;
    ecran.fill(r, c1);

    r.y++, r.x++, r.h-=2;
    ecran.fill(r, c2);

    r.y--, r.h++, r.x = dims.w+dims.x-2;
    ecran.fill(r, c3);

    r.y--, r.h+=2, r.x++;
    ecran.fill(r, c4);
}

MF_TButton::MF_TButton(Uint16 w, Uint16 h, Font &font, const char *texte, Sint16 x, Sint16 y, const Color &rgb)
        :MF_Applet(w, h, rgb, x, y), clicOn(false)
{
    Color noir(0,0,0);
    Surface tmp = font.render_shaded(texte, noir, bgColor);

    //on centre le texte
    int posx = (w - tmp.w())/2;
    int posy = (h - tmp.h())/2;

    //Et on le copie
    Rect rect(posx, posy, w, h);
    tmp.blitTo(surface, 0, rect);
}

//pour les clics
bool MF_TButton::gereEvenement(const SDL_Event &event)
{
    switch (event.type)
    {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && dedans(event.button.x, event.button.y))
            {
                if (!clicOn)
                {
                    clicOn = true;
                    set_updated();
                }
                envoieMessage("clic");
                return 1;
            }
            return 0;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT && clicOn == true)
            {
                if (clicOn)
                {
                    clicOn = false;
                    set_updated();
                }
                if (dedans(event.button.x, event.button.y))
                    envoieMessage("release");
                else
                {
                    envoieMessage("false-release");
                }
                return 1;
            }
            return 0;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
            {
                envoieMessage("clic");
                envoieMessage("release");
                return 1;
            }
            return MF_Directions::gereTouche(event.key);
        default:
            return MF_Directions::gereEvenement(event);
    }
}

//encore une nouvelle fonction pour afficher!
void MF_TButton::affiche(Surface &ecran)
{
    //Fond
    ecran.fill(dims, bgColor);

    //si le bouton est cliqué, on le décale légèrement sur le coté
    if (clicOn)
    {
        Rect rdest (dims.x + 1, dims.y + 1, dims.w - 1, dims.h - 1);
        Rect rsource (0, 0, dims.w - 1, dims.h - 1);
        surface.blitTo(ecran, rsource, rdest);
    } else
    {
        surface.blitTo(ecran, 0, dims);
    }

    //Si cliqué, on inverse la bordure pour donner un effet
    MF_Box_DrawBorders(dims, ecran, !clicOn);
}



MF_TextList::MF_TextList(Font &police, Uint16 w, Uint16 h, int capacity, const char* texte, Sint16 x, Sint16 y, int offsetx, int offsety)
        :MF_TextBox(police, w, h, capacity, texte, x, y, offsetx, offsety), updated(false)
{
    IDselected = -1;
}

bool MF_TextList::EventFilter(const SDL_Event &event)
{
    if (event.type != SDL_KEYDOWN) return true;
    switch (event.key.keysym.sym)
    {
        case SDLK_UP:
            envoieMessage("up");
            champ_texte.surligneOn = false;
            set_updated();
            return 0;
        case SDLK_DOWN:
            envoieMessage("down");
            champ_texte.surligneOn = false;
            set_updated();
            return 0;
        case SDLK_TAB:
            find_word();
            envoieMessage("tab");
            champ_texte.surligneOn = false;
            set_updated();
            return 0;
        case SDLK_RETURN:
            find_word();
            if (envoieMessage("enter"))
                envoieMessage("tab");
            champ_texte.surligneOn = false;
            set_updated();
            return 0;
        default:
            return 1;
    }
}

void MF_TextList::post_event(const SDL_Event &event)
{
    if (event.type != SDL_KEYDOWN || event.key.keysym.unicode == 0 || champ_texte.texte[0] == 0 || event.key.keysym.sym == SDLK_BACKSPACE) return;

    const char *mot = find_word();
    if (mot[0] == '\0') return;

    //On a trouvé un mot correspondant au début de mot dans le champ texte
    int len = champ_texte.texte.length();

    //on ajoute la fin du mot
    champ_texte.effacer(0);
    champ_texte.ecrire( mot );
    //et on met le highlight
    champ_texte.curseury = len;
    champ_texte.startHighlight();
    champ_texte.curseury = champ_texte.texte.length();
    set_updated();
}

//chercher un mot
const char* MF_TextList::find_word()
{
    //les mots sont déjà classés par ordre alphabétique.
    //donc dès que la comparaison < 0 (res = -1), ça sert à rien de continuer
    //si la longueur du mot > longueur de *it, on passe au suivant
    std::set<MF_TextList_Data, MF_Strcmp>::iterator it;
    const char *mot = champ_texte.texte.c_str();

    int len = strlen(mot);
    if (len == 0) goto badway;

    for (it = mots.begin(); it != mots.end(); it++)
    {
        int l2 = strlen((*it).mot);
        int i = min(l2, len);

        int res = strncasecmp(mot, (*it).mot, i);
        if (res == -1)
        {
            it = mots.end();
            break;
        } else if (l2 >= len && res == 0)
        {
            break;
        }
    }

    if (it == mots.end())
    {
    badway:
        IDselected = -1;
        return "";
    }

    IDselected = (*it).datanum;
    return (*it).mot;
}

void MF_TextList::reset()
{
    IDselected = -1;
    mots.clear();
    clear();
}

MF_BarManager::MF_BarManager(Uint16 w, Uint16 h, Sint16 x, Sint16 y, Font &font): posx(0), pos_affichage_v(0), MF_pos(NULL), MF_pos_affichage(NULL), plus_grande_largeur(1), pos_affichage_l(0), last_clic(0), barre_v_clicon(false),
        barre_h_clicon(false), barre_v_on(false), barre_h_on(false), timer_wasinit(false), VIP_HLColor(153,202,242), DEF_HLColor(200,200,200), aff_data(NULL), btype(RegularBorder)
{
    //police
    shareFont(font);
    //couleur & rect
    setColor(0xFF);
    setRect(x, y, w, h);
    this->option_h = 0;
}

MF_BarManager::MF_BarManager(Uint16 w, Uint16 h, Sint16 x, Sint16 y, const char *font_path, Uint8 ptsize): posx(0), pos_affichage_v(0), MF_pos(NULL), MF_pos_affichage(NULL), plus_grande_largeur(1), pos_affichage_l(0), last_clic(0), barre_v_clicon(false),
        barre_h_clicon(false), barre_v_on(false), barre_h_on(false), timer_wasinit(false), VIP_HLColor(153,202,242), DEF_HLColor(200,200,200), aff_data(NULL), btype(RegularBorder)
{
    //police
    loadFont(font_path, ptsize);
    //couleur & rect
    setColor(0xFF);
    setRect(x, y, w, h);
    this->option_h = 0;
}

MF_BarManager::~MF_BarManager()
{
    del_types();
}

void MF_BarManager::actualiser()
{
    updated = true;

    //Sans les bordure
    Rect dims (this->dims.x+2, this->dims.y+2, this->dims.w-4, this->dims.h-4);

    /* remplissage */
    surface.fill(0, bgColor);

    if (MF_pos_affichage != NULL)
    {
        Rect pos_barre (3,2,dims.w,dims.h);

        //la c'est les objets de la liste chainée qui vont gérer l'affichage comme des grands
        MF_pos_affichage->affiche(this, pos_barre);

        /* et la barre!! */
        if (barre_v_on || barre_h_on)
        {
            affiche_barre();
        }
    }

    //enfin la bordure
    if (btype == RegularBorder)
        MF_Box_DrawBorders(Rect(0,0,surface.w(), surface.h()), surface);
}

//gérer fleche haut et bas et clic
bool MF_BarManager::gereTouche(const SDL_KeyboardEvent &keyevent)
{
    if (keyevent.type == SDL_KEYUP) return 0;
    //si flèche haut ou bas
    if (keyevent.keysym.sym == SDLK_DOWN)
    {
        descendcurseur(1);
        return 1;
    }
    if (keyevent.keysym.sym == SDLK_UP)
    {
        montecurseur(1);
        return 1;
    }
    if (keyevent.keysym.sym == SDLK_RETURN)
    {
        string msg = "enter: " + toString(MF_pos->bar_id);
        envoieMessage(msg.c_str());
        return 1;
    }
    return MF_Directions::gereTouche(keyevent);
}

bool MF_BarManager::gereClic(const SDL_MouseButtonEvent &bevent)
{
    //release
    if (bevent.state == SDL_RELEASED)
    {
        if (bevent.button != SDL_BUTTON_LEFT) return 0;
        if (timer_wasinit)
        {
            stop_timer();
        }
        if (barre_h_on && barre_h_clicon)
        {
            barre_h_clicon = false;
            return 1;
        }
        if (barre_v_on && barre_v_clicon)
        {
            barre_v_clicon = false;
            return 1;
        }
        return 0;
    }

    //bouton droit et appui sur roulette
    if (bevent.button == SDL_BUTTON_RIGHT || bevent.button == SDL_BUTTON_MIDDLE)
    {
        return 0;
    }
    //roulette
    if (bevent.button == SDL_BUTTON_WHEELUP)
    {
        montecurseur(1);
        return 1;
    }
    if (bevent.button == SDL_BUTTON_WHEELDOWN)
    {
        descendcurseur(1);
        return 1;
    }

    //Sans les bordure
    Rect dims (this->dims.x+2, this->dims.y+2, this->dims.w-4, this->dims.h-4);
    xy pos (bevent.x - dims.x, bevent.y - dims.y);

    posclic.x = bevent.x;
    posclic.y = bevent.y;
    posmotion.x = bevent.x;
    posmotion.y = bevent.y;

    //nulle part
    if (!SDL_IsInRect(&dims, posclic.x, posclic.y)) return 0;

    //barre verticale
    if (barre_v_on)
    {
        Rect barre_v_scope(dims.w-aff_data->dim_fleches, 0, aff_data->dim_fleches, dims.h - barre_h_on*aff_data->dim_fleches);
        if (SDL_IsInRect(&barre_v_scope, pos.x, pos.y))
        {
            //fleche haut
            if (pos.y < aff_data->dim_fleches)
            {
                montecurseur(1);
                start_timer();
                return 1;
            }
            //fleche basactualiser
            if (pos.y >= dims.h-aff_data->dim_fleches)
            {
                descendcurseur(1);
                start_timer();
                return 1;
            }
            //barre
            if (pos.y >= barre_v_pos-2 && pos.y < barre_v_pos+barre_v_h)
            {
                barre_v_clicon = true;
                if (timer_wasinit) stop_timer();
                return 1;
            }
            //au-dessus de la barre
            if (pos.y < barre_v_pos)
            {
                montebarre_crans(maxlignes());
                start_timer();
                return 1;
            }
            //en dessous de la barre
            else
            {
                descendbarre_crans(maxlignes());
                start_timer();
                return 1;
            }
        }
    }
    //barre horizontale
    if (barre_h_on)
    {
        Rect barre_h_scope (0, dims.h-aff_data->dim_fleches, dims.w - barre_h_on*aff_data->dim_fleches, aff_data->dim_fleches);
        if (SDL_IsInRect(&barre_h_scope, pos.x, pos.y))
        {
            //fleche gauche
            if (pos.x < aff_data->dim_fleches)
            {
                gauchebarre_crans(1);
                start_timer();
                return 1;
            }
            //fleche droite
            if (pos.x >= dims.w-aff_data->dim_fleches)
            {
                droitebarre_crans(1);
                start_timer();
                return 1;
            }
            //barre
            if (pos.x >= barre_h_pos && pos.x < barre_h_pos+barre_h_w)
            {
                barre_h_clicon = true;
                if (timer_wasinit) stop_timer();
                return 1;
            }
            //à gauche de la barre
            if (pos.x < barre_h_pos)
            {
                gauchebarre(dims.w-2);
                start_timer();
                return 1;
            }
            //à droite de la barre
            else
            {
                droitebarre(dims.w-2);
                start_timer();
                return 1;
            }
        }
    }
    stop_timer();

    //sur une ligne?
    if (barre_v_on == false && (unsigned)pos.y > option_h * nbMF) return 0;
    //sur une ligne!
    pos.x = (pos.y-2)/option_h + (barre_v_on*pos_affichage_v);

    move_pos_x((int)pos.x - (int)this->posx);

    if (SDL_GetTicks() - last_clic < 400)
    {
        string msg = "double-clic: " + toString(MF_pos->bar_id);
        envoieMessage(msg.c_str());
        last_clic = SDL_GetTicks() - 400;
    } else
    {
        string msg = "clic: " + toString(MF_pos->bar_id);
        envoieMessage(msg.c_str());
        last_clic = SDL_GetTicks();
    }

    set_updated();

    return 1;
}

//les mouvements
bool MF_BarManager::gereMotion(const SDL_MouseMotionEvent &mevent)
{
    stop_timer();
    last_clic = SDL_GetTicks() - 400;

    //Sans les bordure
    Rect dims (this->dims.x+2, this->dims.y+2, this->dims.w-4, this->dims.h-4);

    xy pos (mevent.x - dims.x, mevent.y - dims.y);
    xy posmotion(this->posmotion.x - dims.x, this->posmotion.y - dims.y);

    if (barre_h_on && barre_h_clicon)
    {
        int decal_x = pos.x-posmotion.x;
        if (decal_x >= 0)
        {
            this->posmotion.x += droitebarre(decal_x);
            return 1;
        } else
        {
            this->posmotion.x -= gauchebarre(-decal_x);
            return 1;
        }
    }
    if (barre_v_on && barre_v_clicon)
    {
        int decal_y = pos.y-posmotion.y;
        if (decal_y >= 0)
        {
            this->posmotion.y += descendbarre(decal_y);
            return 1;
        } else
        {
            this->posmotion.y -= montebarre(-decal_y);
            return 1;
        }
    }
    return 0;
}

bool MF_BarManager::gereEvenement(const SDL_Event &event)
{
    switch (event.type)
    {
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            return gereTouche(event.key);
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            return gereClic(event.button);
        case SDL_MOUSEMOTION:
            return gereMotion(event.motion);
        default:
            return MF_Directions::gereEvenement(event);
    }
}

void MF_BarManager::sort_options(int field)
{
    MF_DataLine *tab[nbMF];
    MF_Base *it = pDebut;

    for (int i = 0; i < nbMF; i++, it = it->pApres)
    {
#ifdef DEBUG_ON
        assert(it != NULL);
        tab[i] = dynamic_cast<MF_DataLine*>(it);
        assert(tab[i]);
#else
        tab[i] = dynamic_cast<MF_DataLine*>(it);
#endif
    }

    //tri par insertion
    for (int i = 1; i < nbMF; i++)
    {
        MF_DataLine *key = tab[i];
        MF_DataBar *member = dynamic_cast<MF_DataBar*>(tab[i]->get_member(field));
        int j;
        for (j = i-1; j >= 0; j--)
        {
            MF_DataBar *member2 = dynamic_cast<MF_DataBar*>(tab[j]->get_member(field));
            if (member->compare(member2) <= 0)
                break;
            tab[j+1] = tab[j];
        }
        tab[j+1] = key;
    }

    //Puis on arrange les divers paramètres
    pDebut = tab[0];
    pFin = tab[nbMF - 1];
    pDebut->pAvant = NULL;
    pFin->pApres = NULL;
    for (int i = 0; i < nbMF - 1; i++)
    {
        tab[i]->pApres = tab[i+1];
        tab[i+1]->pAvant = tab[i];
    }
    MF_pos = dynamic_cast<MF_DataLine*>(get_member(nbMF - posx - 1));
    MF_pos_affichage = dynamic_cast<MF_DataLine*>(get_member(nbMF - pos_affichage_v - 1));
}

void MF_BarManager::start_timer()
{
    if (timer_wasinit) return;

    timer = SDL_AddTimer(SDL_DEFAULT_REPEAT_DELAY, MF_BarManager_Timer, (void*)(this));
    timer_wasinit = true;
}

void MF_BarManager::stop_timer()
{
    if (!timer_wasinit) return;

    SDL_RemoveTimer(timer);
    timer_wasinit = false;
}

void MF_BarManager::clear_options()
{
    delete pDebut;
    pDebut = NULL;
    pFin = NULL;
    MF_pos = NULL;
    MF_pos_affichage = NULL;
    pos_affichage_v = 0;
    pos_affichage_l = 0;
    posx = 0;
    nbMF = 0;
    set_updated();
    barre_v_on = false;
    barre_h_on = false;
    barre_h_clicon = false;
    barre_v_clicon = false;
}
void MF_BarManager::update_barre()
{
    //on regarde l'état des deux barres
    barre_v_on = maxlignes() < nbMF;
    barre_h_on = (dims.w-aff_data->dim_fleches*barre_v_on) < plus_grande_largeur;
    barre_v_on = maxlignes() < nbMF;

    //puis on règle leur taille
    if (barre_v_on)
    {
        barre_v_h = get_barre_v_h();
        barre_v_pos = get_barre_v_pos();
    }
    if (barre_h_on)
    {
        barre_h_w = get_barre_h_w();
        barre_h_pos = get_barre_h_pos();
    }

    set_updated();
}

void MF_BarManager::affiche_barre()
{
    Rect dims (2, 2, this->dims.w-4, this->dims.h-4);
    Rect r;

    //barre verticale
    if (barre_v_on)
    {
        //fond
        r = Rect(dims.x+dims.w-aff_data->dim_fleches,dims.y,aff_data->dim_fleches,dims.h);
        surface.fill(r, aff_data->bgColor);
        //barre
        r.y = barre_v_pos;
        r.h = barre_v_h;
        DrawRect(surface, r, aff_data->borderColor, 1);
        r.h-=2;
        r.w-=2;
        r.x ++;
        r.y ++;
        DrawFilledRect(surface, r, aff_data->spacerColor, aff_data->fillingColor, 1);
        r.x--;
        r.y = dims.y;
        //flèche haut
        aff_data->flechehaut.blitTo(surface, 0, r);
        //flèche bas
        r.y = dims.y+dims.h-aff_data->dim_fleches-aff_data->dim_fleches*barre_h_on;
        aff_data->flechebas.blitTo(surface, 0, r);
    }
    //barre horizontale
    if (barre_h_on)
    {
        //fond
        r = Rect(dims.x, dims.y+dims.h-aff_data->dim_fleches, dims.w, aff_data->dim_fleches);
        surface.fill(r, bgColor);
        //flèche gauche
        aff_data->flechegauche.blitTo(surface, 0, r);
        //flèche droite
        r.x = dims.x+dims.w-aff_data->dim_fleches-aff_data->dim_fleches*barre_v_on;
        aff_data->flechedroite.blitTo(surface, 0, r);
        //barre
        r.x = barre_h_pos;
        r.w = barre_h_w;
        r.h-=2;
        r.w-=2;
        r.x ++;
        r.y ++;
        DrawFilledRect(surface, r, aff_data->spacerColor, aff_data->fillingColor, 1);
    }
}

int MF_BarManager::descendcurseur(int crans)
{
#ifdef DEBUG_ON
    assert (crans >= 0);
#endif

    int true_crans = min(nbMF - 1 - posx, (unsigned)crans);
    if (true_crans == 0) return 0;

    set_updated();

    move_pos_x(true_crans);
    repos_barre();

    return true_crans;
}

bool MF_BarManager::repos_barre()
{
    if (barre_v_on)
    {
        int max = maxlignes();
        if (posx >= pos_affichage_v+max)
        {
            int true_pos = posx-max-pos_affichage_v+1;
            descendbarre_crans(true_pos);
            return true;
        }
        if (posx < pos_affichage_v)
        {
            int true_pos = pos_affichage_v-posx;
            montebarre_crans(true_pos);
            return true;
        }
    }
    return false;
}

int MF_BarManager::montecurseur(int crans)
{
#ifdef DEBUG_ON
    assert (crans >= 0);
#endif

    int true_crans = min(crans, (int)posx);
    if (true_crans == 0) return 0;

    set_updated();

    move_pos_x(-true_crans);
    repos_barre();

    return true_crans;
}

int MF_BarManager::montebarre_crans(int crans)
{
    assert (crans >= 0);

    int true_crans = min(crans, (int)pos_affichage_v);
    if (true_crans == 0) return 0;

    move_pos_affichage(-true_crans);

    update_barre();
    set_updated();

    return true_crans;
}

int MF_BarManager::descendbarre_crans(int crans)
{
    assert (crans >= 0);

    int true_crans = min(pos_affichage_v+maxlignes()-nbMF, (unsigned)crans);

    if (true_crans == 0) return 0;

    move_pos_affichage(true_crans);

    update_barre();
    set_updated();

    return true_crans;
}

Uint32 MF_BarManager_Timer(unsigned int time, void *param)
{
    MF_BarManager *item = (MF_BarManager*)param;

    //si la classe n'est plus la première de son MF_Boss, on retourne
    //ou si la souris n'est plus enfoncée
    if ((item->isFirst == false) || !BUTTON_LEFT_ON || !item->dedans(item->posclic.x, item->posclic.y)) {
        //on enlève le timer
        item->stop_timer();
        return 0;
    }

    //Et on simule un clic
    SDL_Event event;
    SDL_MouseButtonEvent bevent;
    bevent.type = SDL_MOUSEBUTTONDOWN;
    bevent.button = SDL_BUTTON_LEFT;
    bevent.state = SDL_PRESSED;
    bevent.x = item->posclic.x;
    bevent.y = item->posclic.y;
    event.button = bevent;

    SDL_PushEvent(&event);
    return SDL_DEFAULT_REPEAT_INTERVAL;
}

int MF_BarManager::montebarre(int px)
{
    assert (px >= 0);

    Rect dims = Rect(2, 2, this->dims.w-4, this->dims.h-4);

    int true_px = min(px, (int)barre_v_pos-aff_data->dim_fleches-dims.y);
    if (true_px == 0) return 0;

    barre_v_pos -= true_px;

    //maintenant il faut calculer à quelle ligne correspond la nouvelle pos
    unsigned int newpos = ((nbMF-maxlignes())*(barre_v_pos-dims.y-aff_data->dim_fleches))/(dims.h-aff_data->dim_fleches*(2+barre_h_on)-barre_v_h);
    if (newpos != pos_affichage_v)
    {
        set_updated();
        move_pos_affichage(newpos-pos_affichage_v);
    } else
    {
        affiche_barre();
        if (pSup != NULL)
            pSup->updated = false;
    }
    return true_px;
}

int MF_BarManager::get_pos_of(Uint16 id)
{
    MF_DataLine *it = dynamic_cast<MF_DataLine *>(pFin);
    for (int i = 0; it != NULL; i++)
    {
        if (it->bar_id == id)
        {
            return i;
        }
        it = dynamic_cast<MF_DataLine *>(it->pAvant);
    }
    return -1;
}

int MF_BarManager::descendbarre(int px)
{
    assert (px >= 0);

    Rect dims (2, 2, this->dims.w-4, this->dims.h-4);

    //true_px correspond au nombre de pixels desquels la barre va être déplacée
    int true_px = min(px, (int)dims.y+dims.h-(barre_h_on+1)*aff_data->dim_fleches-barre_v_h-barre_v_pos);
    if (true_px == 0) return 0;

    barre_v_pos += true_px;

    //maintenant il faut calculer à quelle ligne correspond la nouvelle pos
    unsigned int newpos = ((nbMF-maxlignes())*(barre_v_pos-dims.y-aff_data->dim_fleches))/(dims.h-aff_data->dim_fleches*(2+barre_h_on)-barre_v_h);
    if (newpos != pos_affichage_v)
    {
        set_updated();
        move_pos_affichage(newpos-pos_affichage_v);
    } else
    {
        affiche_barre();
        if (pSup != NULL)
            pSup->updated = false;
    }

    return true_px;
}

int MF_BarManager::gauchebarre(int px)
{
    assert (px >= 0);

    Rect dims (2, 2, this->dims.w-4, this->dims.h-4);

    int true_px = min(px, (int)barre_h_pos-aff_data->dim_fleches-dims.x-1);
    if (true_px == 0) return 0;

    barre_h_pos -= true_px;

    //maintenant il faut calculer à quelle ligne correspond la nouvelle pos
    int newpos = ((plus_grande_largeur-maxlargeur())*(barre_h_pos-dims.x-aff_data->dim_fleches))/(dims.w-aff_data->dim_fleches*(2+barre_v_on)-barre_h_w);
    if (newpos != pos_affichage_l)
    {
        set_updated();
        pos_affichage_l = newpos;
    } else
    {
        affiche_barre();
    }
    return true_px;
}

int MF_BarManager::droitebarre(int px)
{
    assert (px >= 0);

    Rect dims (2, 2, this->dims.w-4, this->dims.h-4);

    int true_px = min(px, (int)dims.x+dims.w-(barre_v_on+1)*aff_data->dim_fleches-barre_h_w-barre_h_pos);
    if (true_px == 0) return 0;

    barre_h_pos += true_px;

    //maintenant il faut calculer à quelle ligne correspond la nouvelle pos
    int newpos = ((plus_grande_largeur-maxlargeur())*(barre_h_pos-dims.x-aff_data->dim_fleches))/(dims.w-aff_data->dim_fleches*(2+barre_v_on)-barre_h_w);
    if (newpos != pos_affichage_l)
    {
        set_updated();
        pos_affichage_l = newpos;
    } else
    {
        affiche_barre();
    }
    return true_px;
}

bool MF_BarManager::gauchebarre_crans(int crans)
{
    //1 crans = 1/10 de la largeur max
    return gauchebarre(crans*min(1, (int)maxlargeur()/10)) != 0;
}

bool MF_BarManager::droitebarre_crans(int crans)
{
    //1 crans = 1/10 de la largeur max
    return droitebarre(crans*min(1, (int)maxlargeur()/10)) != 0;
}

int MF_BarManager::get_barre_v_h()
{
    return max(((dims.h-4-(2+barre_h_on)*aff_data->dim_fleches)*maxlignes())/nbMF, (unsigned)8);
}

int MF_BarManager::get_barre_v_pos()
{
    return 2+aff_data->dim_fleches+(dims.h-aff_data->dim_fleches*(2+barre_h_on)-barre_v_h-4)*pos_affichage_v/(nbMF-maxlignes());
}

int MF_BarManager::get_barre_h_w()
{
    return ((dims.w-4-(2+barre_v_on)*aff_data->dim_fleches)*maxlargeur())/plus_grande_largeur;
}

int MF_BarManager::get_barre_h_pos()
{
    return dims.x+aff_data->dim_fleches+ (dims.w-dims.x-aff_data->dim_fleches*(2+barre_v_on)-barre_h_w)*pos_affichage_l/(plus_grande_largeur-maxlargeur());
}

void MF_BarManager::affiche(Surface &ecran)
{
    if (!updated) actualiser();
    surface.blitTo(ecran, 0, dims);
}

bool MF_BarManager::add_option(int id_of_the_bar, ...)
{
    va_list da_list;
    va_start(da_list, id_of_the_bar);

    MF_DataLine *my_point = new MF_DataLine(type_cont);
    allouer(my_point);
    my_point->bar_id = id_of_the_bar;

    MF_DataBar *it = dynamic_cast<MF_DataBar *>(my_point->pDebut);
    vector<MF_Base_Type*>::iterator it2 = type_cont.begin();

    while (it != NULL)
    {
        it = it->init((*it2)->w, (*it2)->h, va_arg(da_list, void*));
        ++it2;
    }

    va_end(da_list);

    set_updated();

    if (MF_pos == NULL)
    {
        MF_pos = my_point;
    }
    if (MF_pos_affichage == NULL)
    {
        MF_pos_affichage = my_point;
    }

    update_barre();

    return true;
}

int MF_BarManager::move_pos_x(int steps)
{
#ifdef DEBUG_ON
    assert(MF_pos != NULL);
#endif
    int i;
    MF_Base *item = MF_pos;
    if (steps >= 0)
    {
        for (i=0; i < steps; i ++)
        {
            if (item->pAvant == NULL) break;
            item = item->pAvant;
        }
        posx += i;
    } else
    {
        for (i=0; i > steps; i --)
        {
            if (item->pApres == NULL) break;
            item = item->pApres;
        }
        posx += i;
    }
    if (i != 0)
    {
        set_updated();
        MF_pos = dynamic_cast<MF_DataLine*>(item);
    }
    return i;
}

int MF_BarManager::move_pos_affichage(int steps)
{
#ifdef DEBUG_ON
    assert(MF_pos_affichage != NULL);
#endif
    int i;
    MF_Base *item = MF_pos_affichage;
    if (steps >= 0)
    {
        for (i=0; i < steps; i ++)
        {
            if (item->pAvant == NULL) break;
            item = item->pAvant;
        }
        pos_affichage_v += i;
    } else
    {
        for (i=0; i > steps; i --)
        {
            if (item->pApres == NULL) break;
            item = item->pApres;
        }
        pos_affichage_v += i;
    }
    if (i != 0)
    {
        MF_pos_affichage = dynamic_cast<MF_DataLine*>(item);
        set_updated();
    }

    return i;
}

void MF_BarManager::resize(Uint16 w, Uint16 h)
{
    MF_Surf::resize(w, h);
}

void MF_BarManager::setPos(bool prems)
{
    MF_Base::setPos(prems);
    set_updated();
}

void MF_BarManager::shareFont(Font &font)
{
    police = font;
}

void MF_BarManager::loadFont(const char* font_path, Uint8 ptsize)
{
    police.load(font_path, ptsize);
}

MF_DataLine::MF_DataLine(vector<MF_Base_Type *>&cont)
{
    //On crée les DataBar en fct des types
    for (int i = cont.size()-1; i >= 0; i--)
    {
        allouer( cont[i]->create() );
        pDebut->role = i;
    }
}

void MF_DataLine::affiche(MF_BarManager *boss, Rect &posbarre)
{
    //si on est highlighté
    if (boss->MF_pos == this)
    {
        Rect dest (posbarre.x-1, posbarre.y, posbarre.w, boss->option_h);
        //dépend si la fenêtre est première de son groupe
        if (boss->isFirst)
        {
            boss->surface.fill(dest, boss->VIP_HLColor);
        } else
        {
            boss->surface.fill(dest, boss->DEF_HLColor);
        }
    }

    Uint16 base_x = posbarre.x;
    MF_DataBar *prems = dynamic_cast<MF_DataBar *> (pDebut);
#ifdef DEBUG_ON
    assert(prems != NULL);
#endif
    prems->affiche(boss, posbarre);
    posbarre.x = base_x;
    posbarre.y += boss->option_h;

    if (pAvant == NULL || posbarre.y > posbarre.h) return;
    MF_DataLine *avant = dynamic_cast<MF_DataLine*>(pAvant);
#ifdef DEBUG_ON
    assert(avant != NULL);
#endif

    avant->affiche(boss, posbarre);
}

void MF_DataBar::affiche_next(MF_BarManager *boss, Rect &pos_barre)
{
    pos_barre.x += boss->type_cont[role]->w;
    if (pos_barre.x >= pos_barre.w || pApres == NULL)
        return;

    MF_DataBar *suivant = dynamic_cast<MF_DataBar*> (pApres);
    if (suivant == NULL)
    {
        throw MF_Exception ("MF_Databar :: affiche_next --> pApres pas databar!!");
    }
    suivant->affiche(boss, pos_barre);
}

void MF_DataBar::affiche(MF_BarManager *boss, Rect &pos_barre)
{
    throw(MF_Exception("MF_Databar::affiche -- classe non dérivée!"));
}

void MF_TextBar::affiche(MF_BarManager *boss, Rect &pos_barre)
{
    if (texte == NULL || texte[0] == 0)
        return affiche_next(boss, pos_barre);

    /* Voici l'affichage */
    //pos_barre est passé par réf, donc surtout pas le repasser à SDL_BlitSurface qui va le modifier
    Rect dest (pos_barre.x, pos_barre.y, boss->type_cont[role]->w, boss->type_cont[role]->h);
    Rect src (0, 0, boss->type_cont[role]->w, boss->type_cont[role]->h);

    Color textColor (0, 0, 0);

    Surface tmp = boss->police.render_blended(texte, textColor);

    tmp.blitTo(boss->surface, src, dest);

    affiche_next(boss, pos_barre);
}

void MF_ImageBar::affiche(MF_BarManager *boss, Rect &pos_barre)
{
    /* Voici l'affichage */
    //pos_barre est passé par réf, donc surtout pas le repasser à SDL_BlitSurface qui va le modifier
    Rect dest (pos_barre.x, pos_barre.y, boss->type_cont[role]->w, boss->type_cont[role]->h);
    Rect src (0, 0, boss->type_cont[role]->w, boss->type_cont[role]->h);

    image.blitTo(boss->surface, src, dest);

    affiche_next(boss, pos_barre);
}

MF_Alert::MF_Alert(MF_Boss *src, Font &police, const char *texte, const Color &color)
{
    //on initialise le bouton
    ok = new MF_TButton(70, 30, police, "Ok");

    //On calcule la taille du texte etc.
    int max_size = 0;
    Uint16 nb_ligne = 0;

    //on découpe le texte par les \n
    char copy[strlen(texte) + 1];
    strcpy(copy, texte);

    //on fait une boucle en prenant morceau par morceaux
    //on peut rien faire contre deux \n d'affilée(le vide compte pas)
    for (char *tmp = strtok(copy, "\n\r"); tmp != NULL; tmp = strtok(NULL, "\n\r"))
    {
        nb_ligne++;

        int w;
        police.text_size(tmp, &w, NULL);
        max_size = max(w, max_size);
    }

    //On peut maintenant calculer la taille de la fenêtre
    inside = new MF_Applet(max(max_size + 74, 300), police.line_skip() * nb_ligne + 80, color);

    //nos dims
    dims.w = inside->dims.w;
    dims.h = inside->dims.h;
    dims.x = (src->dims.w-dims.w)/2+src->dims.x;
    dims.y = (src->dims.h-dims.h)/2+src->dims.y;
    //On écrit la phrase
    inside->drawString(police, texte, (dims.w - max_size)/2, 16);
    //le bouton
    ok->dims.x = (dims.w-ok->dims.w)/2+dims.x;
    ok->dims.y = (dims.h-ok->dims.h)/2+dims.y+12;
    //le inside
    inside->dims.x = dims.x;
    inside->dims.y = dims.y;

    allouer(ok);
}

MF_Alert::~MF_Alert()
{
    delete inside;
}

void MF_Alert::affiche(Surface &surface)
{
    inside->affiche(surface);

    ok->affiche(surface);
}

bool MF_Alert::recoitMessage(const char *message, MF_Base* fenetre)
{
    if (strcmp(message, "release") == 0)
        pSup->detruireMF(this);

    return true;
}

MF_TrayBar :: MF_TrayBar(Sint16 x, Sint16 y, Uint16 w, Uint16 numvals, const Color &fcolor, Uint16 step, Sint16 min_val, bool clicAble)
        : pos_icon(min_val), num_vals(numvals), pas(step), min_val(min_val), void_color(0xFF,0xFF,0xFF), lastval(min_val), clicOn (false), clicAble(clicAble)
{
    MF_TrayBar::colorkey = Color(252, 3, 65);
    MF_Tray_Icon = Man.LoadRessource("pos_icon.bmp", MF_TrayBar::colorkey, true);

    bgColor = colorkey;
    setRect(x, y, w, height);
    surface.colorkey(colorkey);
    bgColor = fcolor;
    /*
        Maintenant qu'on a le fond transparent et la bonne filling color,
        on va pouvoir s'attaquer au dessin du contour qui restera tel quel après
     */
    Rect r (height/2+1, 0, dims.w-height-1, 1);
    surface.fill(r);
    r.y = dims.h - 1;
    surface.fill(r);
    r.x--;
    r.y = 1;
    r.h = dims.h-2;
    r.w += 2;
    surface.fill(r, void_color);
    //Les petits cercles ^^
    SDL_DrawFilledCircleQuarter(surface.s, height/2, height/2, height/2, /*noir*/0, bgColor.map(surface), true, false, true, false);
    SDL_DrawFilledCircleQuarter(surface.s, dims.w-(height/2+1), height/2, height/2, /*noir*/0, void_color.map(surface), false, true, false, true);
}


void MF_TrayBar::affiche(Surface &surface)
{
    //fond
    if (pos_icon != lastval)
        actualiser();
    MF_Surf::affiche(surface);
    //icone
    Rect r (get_posx() + dims.x, dims.y, 0, 0);
    MF_Tray_Icon.blitTo(surface, 0, r);
}

bool MF_TrayBar::gereEvenement(const SDL_Event &event)
{
    if (!clicAble)
    {
        if (event.type != SDL_KEYDOWN)
            return false;
        return gereTouche(event.key);
    }
    switch (event.type)
    {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                Sint16 posx = event.button.x - dims.x, posy = event.button.y - dims.y;
                if (SDL_IsInCircle(get_posx() + height/2+1, height/2+1, height/2+1, posx, posy))
                {
                    clicOn = true;
                    pos_clic_x = posx-get_posx();
                    return true;
                }
            }
            return false;
        case SDL_MOUSEBUTTONUP:
            if (clicOn && event.button.button == SDL_BUTTON_LEFT)
            {
                clicOn = false;
                return true;
            }
            return false;
        case SDL_MOUSEMOTION:
            if (clicOn)
            {
                {
                    Sint16 posx = event.motion.x - dims.x;
                    pos_icon = min_val + ((posx - pos_clic_x) * num_vals) / (dims.w-MF_Tray_Icon.w());
                }
            motion:
                if (pos_icon < min_val)
                {
                    pos_icon = min_val;
                } else
                    if (pos_icon >= min_val + num_vals)
                    {
                        pos_icon = min_val + num_vals - 1;
                    } else
                    {
                        pos_icon = pos_icon - ((pos_icon-min_val) % pas);
                    }
                envoieMessage("moved");
                return true;
            }
            return false;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_RIGHT)
            {
                pos_icon += pas;
                goto motion;
            } else if (event.key.keysym.sym == SDLK_LEFT)
            {
                pos_icon -= pas;
                goto motion;
            }
            return gereTouche(event.key);
        default:
            return false;
    }
}

void MF_TrayBar::actualiser()
{
    Rect r (0, 1, 0, dims.h-2);

    //Si diminution
    if (pos_icon < lastval)
    {
        r.x = get_posx() + height/2;
        r.w = ((lastval-min_val)*(dims.w-MF_Tray_Icon.w()+1))/num_vals + height/2 - r.x;
        surface.fill(r, void_color);
    } else //augmentation
    {
        r.x = ((lastval-min_val)*(dims.w-MF_Tray_Icon.w()+1))/num_vals + height/2;
        r.w = get_posx() + height/2 - r.x;
        surface.fill(r, bgColor);
    }

    lastval = pos_icon;
    return;
}

MF_TrayBar :: ~MF_TrayBar()
{
    ;
}

MF_ListeDeroulante::MF_ListeDeroulante(Uint16 w, Uint16 sh, Uint16 bh, Sint16 x, Sint16 y, Font &font)
        :MF_BarManager(w, sh, x, y, font), deroulee(false), small_height(sh), big_height(bh)
{

}

MF_ListeDeroulante::MF_ListeDeroulante(Uint16 w, Uint16 sh, Uint16 bh, Sint16 x, Sint16 y, const char *font_name, Uint8 ptsize)
        :MF_BarManager(w, sh, x, y, font_name, ptsize), deroulee(false), small_height(sh), big_height(bh)
{

}

bool MF_ListeDeroulante::dedans(int x, int y) const
{
    return (deroulee?MF_BarManager::dedans(x, y):dims.x <= x && dims.y <= y && dims.w + dims.x > x && dims.y + small_height > y);
}

bool MF_ListeDeroulante::gereClic(const SDL_MouseButtonEvent &bevent)
{
    if (bevent.type != SDL_MOUSEBUTTONDOWN || bevent.button != SDL_BUTTON_LEFT)
    {
        return MF_BarManager::gereClic(bevent);
    }

    if (!dedans(bevent.x, bevent.y))
        return change_deroulee(false);

    if (bevent.x < dims.x+dims.w-2-aff_data->dim_fleches)
    {
        int status = MF_BarManager::gereClic(bevent);
        return change_deroulee(false) ||status;
    }

    //Le clic est sur le côté flèches
    if (deroulee)
        return MF_BarManager::gereClic(bevent);
    else
        return change_deroulee(true);
}

bool MF_ListeDeroulante::gereTouche(const SDL_KeyboardEvent &kevent)
{
    int status = MF_BarManager::gereTouche(kevent);
    if (kevent.keysym.sym == SDLK_RETURN)
    {
        change_deroulee(false);
    }
    return status;
}

bool MF_ListeDeroulante::change_deroulee(bool newstate)
{
    if (deroulee == newstate)
        return false;

    deroulee = newstate;
    dims.h = (deroulee?big_height:small_height);
    resize(dims.w, dims.h);

    if (!deroulee)
    {
        MF_pos_affichage = MF_pos;
        pos_affichage_v = posx;
    } else
    {
        if (posx > nbMF-maxlignes())
        {
            move_pos_affichage(nbMF-maxlignes()-posx);
        }
        update_barre();
    }

    set_updated();

    return true;
}

const Color MF_Radio::drawing_color (60, 7, 67);

MF_Radio::MF_Radio(Font &police, const char *texte, bool selected, Sint16 x, Sint16 y, const Color &colorkey)
        :MF_Applet(15 + 20 + police.text_width(texte), max(police.line_skip(),(Uint16)19), colorkey, x, y), selected(selected)
{
    surface.colorkey(bgColor);

    drawRadio();
    drawString(police, texte, 34, 0);
}

void MF_Radio::affiche(Surface &ecran)
{
    if (selected != isFirst)
    {
        selected = isFirst;
        drawRadio();
    }
    return surface.blitTo(ecran, 0, dims);
}

int MF_Radio::drawRadio()
{
    if (selected)
        SDL_DrawFilledCircle(surface.s, 7, 7, 7, drawing_color.map(surface), drawing_color.map(surface));
    else
        SDL_DrawFilledCircle(surface.s, 7, 7, 7, drawing_color.map(surface), Color(0xFF, 0xFF, 0xFF).map(surface));

    return 0;
}

MF_Base * MF_BRadio::add_option(Font &police, const char *texte, Uint16 role)
{
    allouer(new MF_Radio(police, texte, true, dims.x, dims.h, bgColor));
    dims.h += pDebut->dims.h;
    dims.w = max(dims.w, pDebut->dims.w);
    pDebut->role = role;

    return pDebut;
}

bool MF_BRadio::recoitMessage(const char *message, MF_Base *fenetre)
{
    if (strcmp(message, "select") == 0)
    {
        return envoieMessage(("select: " + toString(fenetre->role)).c_str());
    }
    return false;
}

MF_CheckBox::MF_CheckBox(Font &police, const char *texte, bool checked, Sint16 x, Sint16 y, const Color &colorkey)
        :MF_Applet(15 + 20 + police.text_width(texte), max((Uint16)19, police.line_skip()), colorkey, x, y), checked(checked)
{
    surface.colorkey(bgColor);

    drawCross();
    drawString(police, texte, 34, 0);
}

void MF_CheckBox::affiche(Surface &ecran)
{
    if (!updated)
    {
        updated = true;
        drawCross();
    }
    surface.blitTo(ecran, 0, dims);
}

int MF_CheckBox::drawCross()
{
    //Contour
    Rect r (2, 1, 13, 13);
    DrawFilledRect(surface, r, Color(), Color(0xFF, 0xFF, 0xFF), 2);
    if (checked)
        BlitImage(4, 3, "cross.bmp");

    return 0;
}

bool MF_CheckBox::gereEvenement(const SDL_Event &event)
{
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT)
        return false;

    Rect r (dims.x + 1, dims.y + 1, 13, 13);
    if (!SDL_IsInRect(&r, event.button.x, event.button.y))
        return false;

    checked = !checked;

    if (checked)
        envoieMessage("checked");
    else
        envoieMessage("unchecked");

    set_updated();

    return true;
}

void MF_CheckBox::set_updated()
{
    updated = false;
    MF_Base::set_updated();
}

void MF_CheckBox::set_check(bool checked)
{
    if (checked == this->checked)
        return;

    this->checked = checked;
    set_updated();
}
