#include "MF_text.hh"
#include "intergfx.hpp"

using namespace std;

MF_Line::~MF_Line()
{
}

/* Met une couleur de texte*/
void MF_Line::setTextColor(const Color &c)
{
    set_updated();
    textColor = c;
}

/* Met une police */
void MF_Line::setFont(const char *fichier, Uint16 ptsize)
{
    police = FontMan.LoadRessource(fichier, ptsize);
}

void MF_Line::shareFont(Font &font)
{
    police = font;
}

/* Pour balancer du texte */
void MF_Line::write(const string &texte, int pos)
{
    /* si on tombe sur un \n ou \r on prend que la suite */
    unsigned int n;

    n = texte.find_last_of("\r\n");

    /* On en a trouvé! */
    if (n != string::npos)
    {
        this->texte = texte.substr(n+1) + this->texte.substr(pos);
    } else
    {
        /* Sinon on se contente de rajouter le texte */
        this->texte.insert(pos, texte);
    }

    /* et on actualise! */
    set_updated();;
}

/* Pour refresh la surface en mémoire */
void MF_Line::refresh()
{
    updated = true;

    /* Voici l'affichage */
    surface.fill(0, bgColor);

    Surface tmp = police.render_shaded(texte.c_str(), textColor, bgColor);

    tmp.blitTo(surface);
}

int MF_Line::toLen(int maxlen) const
{
#ifndef MF_TEXT_ACCENTUE
    int curlen (0);
    int w (0);

    int i (1);
    const char *explore = texte.c_str();

    if (*explore == '\r' || *explore == '\n' || *explore == '\0') return 0;

    TTF_GlyphMetrics(police, *explore, &w, NULL, NULL, NULL, &curlen);

    curlen -= w;

    if (curlen > maxlen) return 0;
    for (++explore; ; ++explore, curlen += w, i++)
    {
        if (*explore == '\r' || *explore == '\n' || *explore == '\0')
        {
            return i;
        }
        TTF_GlyphMetrics(police, *explore, NULL, NULL, NULL, NULL, &w);
        if (curlen + w > maxlen)
        {
            return i;
        }
    }
#else
    int curlen (0);
    char ch;

    int i (1);
    const char *explore = texte.c_str();

    if (*explore == '\r' || *explore == '\n' || *explore == '\0') return 0;

    ch = explore[1];
    *(char*)(void*)(&explore[1]) = 0;

    curlen = police.text_width(explore);
    *(char*)(void*)(&explore[1]) = ch;

    if (curlen > maxlen) return 0;
    const char *ptr = explore;
    for (++explore; ; ++explore, i++)
    {
        if (*explore == '\r' || *explore == '\n' || *explore == '\0')
        {
            return i;
        }
        ch = explore[1];
        *(char*)(void*)(&explore[1]) = 0;

        curlen = police.text_width(ptr);
        *(char*)(void*)(&explore[1]) = ch;

        if (curlen > maxlen)
            return i;
    }
#endif
}

/* Gere clic, motion et touche */
bool MF_WLine::deal_w_Event(const SDL_Event &event)
{
    switch (event.type)
    {
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            return deal_w_key(event.key);
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            return deal_w_click(event.button);
        case SDL_MOUSEMOTION:
            return deal_w_motion(event.motion);
        default:
            return 0;
    }
}

void MF_WLine::setPos(bool prems)
{
    MF_Base::setPos(prems);

    if (prems == true)
    {
        if (ecrivable)
        {
            curseurOn = true;
            curseurDisp = true;
            second_update = false;
            lastCursor = SDL_GetTicks();
        }
    }
    else //if prems == false
    {
        curseurOn = false;
        if (curseurDisp)
        {
            second_update = false;
        }
        if (surligneOn == true)
        {
            surligneOn = false;
            updated = false;
        }
    }
}

bool MF_WLine::deal_w_key(const SDL_KeyboardEvent &keyevent)
{
    if (keyevent.type == SDL_KEYUP)
        return 0;

    if (!ecrivable)
    {
        switch (keyevent.keysym.sym)
        {
            case SDLK_BACKSPACE:
            case SDLK_RETURN:
            case SDLK_TAB:
            case SDLK_DELETE:
                return 0;
            default:
                break;
        }
    }

    int refresh = 0;

    switch (keyevent.keysym.sym)
    {
        case SDLK_BACKSPACE:
            if (surligneOn) {
                clearHighlight();
            }
            else if (curseury > 0)
            {
                MF_Line::erase(curseury-1, 1);
                --curseury;
            }
            refresh = 1;
            break;
        case SDLK_RETURN:
            texte.insert(0, "t");
            sendToBoss (texte.c_str());
            erase(0);
            curseury = 0;
            refresh = 1;
            break;
        case SDLK_TAB:
            if (surligneOn)
                clearHighlight();
            write("    ");
            refresh = 1;
            break;
        case SDLK_DELETE:
            if (surligneOn) {
                clearHighlight();
            }
            else if (texte[curseury] != 0) {
                erase(curseury, 1);
            }
            refresh = 1;
            break;
        case SDLK_PAGEUP:
        case SDLK_HOME:
            if (updateHighlight() || surligneOn) {
                refresh = 1;
            }
            curseury = 0;
            if (refresh)
            {
                set_updated();
            }
            refresh = 1;
            break;
        case SDLK_END:
        case SDLK_PAGEDOWN:
            if (updateHighlight() || surligneOn) {
                refresh = 1;
            }
            curseury = texte.length();
            if (refresh)
            {
                set_updated();
            }
            refresh = 1;
            break;
        case SDLK_UP:
        case SDLK_DOWN:
            return 0;
        case SDLK_LEFT:
            if (updateHighlight() || surligneOn) {
                refresh = 1;
            }
            if (curseury > 0) {
                --curseury;
            }
            if (refresh)
            {
                set_updated();
            }
            refresh = 1;
            break;
        case SDLK_RIGHT:
            if (updateHighlight() || surligneOn) {
                refresh = 1;
            }
            if (texte[curseury] != 0) {
                ++curseury;
            }
            if (refresh)
            {
                set_updated();
            }
            refresh = 1;
            break;
        default:
            break;
    }

    if (refresh)
    {
        /* Si il y a une touche pressée active
           on actualise et on remet le timer du
           curseur à 0
           --
           Ca veut dire que le curseur s'display
           obligatoirement, et pendant 500 ms au
           moins */
        second_update = false;
        lastCursor = SDL_GetTicks();
        curseurDisp = true;
        return refresh;
    }

    if (keyevent.keysym.unicode == 0)
    {
        /* c'est une touche spéciale */
        /* Qu'on n'a pas traité avant (ex: alt, shift...) */
        return 1;
    }
    /* si alt ou ctrl */
    if (ALT_ON or CTRL_ON)
    {
        return 0;
    }

    if (!ecrivable)
        return 0;

    /* C'est une touche ASCII */
    clearHighlight();
    char ch = keyevent.keysym.unicode;
    write(ch);
    refresh = 1;

    if (refresh)
    {
        lastCursor = SDL_GetTicks();
        curseurDisp = true;
        return refresh;
    }

    return 0;
}

bool MF_WLine::deal_w_click(const SDL_MouseButtonEvent &buttonevent)
{
    /* Que les clics gauche */
    if (buttonevent.button != SDL_BUTTON_LEFT)
        return 0;
    if (buttonevent.type == SDL_MOUSEBUTTONUP)
    {
        clicOn = false;
        return 1;
    }
    /* On regarde d'abord si le clic est dans la ligne en hauteur*/
    if (buttonevent.y >= dims.y && buttonevent.y <= dims.y+this->police.line_skip())
    {
        /* puis en largeur */
        curseury = toLen(buttonevent.x-dims.x);
        updateHighlight();
        startHighlight();
        this->clicOn = true;
        set_updated();;
    }

    return 1;
}

bool MF_WLine::deal_w_motion(const SDL_MouseMotionEvent &motionevent)
{
    /* Le souci c'est la gestion du highlight */

    if (this->clicOn == true)
    {
        /* On regarde d'abord si le motion est dans la ligne en hauteur*/
        if (motionevent.y >= dims.y && motionevent.y <= dims.y+this->police.line_skip())
        {
            /* puis en largeur */
            curseury = toLen(MIN(motionevent.x-dims.x, dims.w));
        }
        set_updated();;
    }
    return 1;
}

/* Pour write du texte */
void MF_WLine::write(const std::string &texte, int pos)
{
    this->texte.insert(pos, texte);
    curseury = pos + texte.length();

    /* et on actualise! */
    if (police)
        set_updated();;
}

/* erase là ou c'est surligné */
bool MF_WLine::clearHighlight()
{
    if (surligneOn == false)
        return false;
    surligneOn = false;

    int min, max; /* debut et fin du highlight */

    min = MIN(curseury, curseuryDeb);
    max = MAX(curseury, curseuryDeb);

    if (min == max) {
        return false;
    }

    /* On efface là ou c'est highlighted */
    erase (min, max - min);

    /* et on remet le curseur au début du highlight */
    curseury = min;
    return true;
}

/* commencer si possible le higlight */
void MF_WLine::startHighlight()
{
    if (surligneOn == true)
        return;

    surligneOn = true;
    curseuryDeb = curseury;
}

/* voir si on doit enlever le highlight, ... */
bool MF_WLine::updateHighlight()
{
    //si on doit le mettre
    if (surligneOn == false && (clicOn == true or SHIFT_ON == true)) {
        startHighlight();
        return true;
    }
    //si on doit le virer
    if (surligneOn == true && clicOn ==false && SHIFT_ON == false) {
        surligneOn = false;
        return true;
    }
    return false;
}

/* displayr et refresh */
void MF_WLine::display(Surface &ecran)
{
    second_update = true;

    MF_Line::display(ecran);

    /* la seule diff c'est le curseur , le reste est géré par actualise */
    if (!curseurOn || !isFirst)
        return;

    /* Si on est à plus de 500ms sec de lastCursor on change d'état */
    Uint32 curTime = SDL_GetTicks();
    if (curTime - lastCursor > 500)
    {
        curseurDisp = !curseurDisp;
        lastCursor = SDL_GetTicks();
    }

    /* Si besoin d' displayr le curseur */
    if (curseurDisp)
    {
        Rect cursRect;
        int w;
        string tmp;

        /* Pos du curseur */
        tmp = texte.substr(0, curseury);
        w = police.text_width(tmp.c_str());

        cursRect.x = w + dims.x;
        cursRect.y = dims.y;
        cursRect.w = 1;
        cursRect.h = MIN(police.line_skip(), dims.y+dims.h-cursRect.y);

        /* affichage du curseur de la même couleur que celle du texte */
        ecran.fill (cursRect, textColor);
    }
}

void MF_WLine::refresh()
{
    if (!surligneOn)
    {
        MF_Line::refresh();
        return;
    }

    updated = true;

    /* On doit gérer le highlight v_v */

    //fond
    surface.fill(0, bgColor);
    {
        int min, max, x, w; /* debut et fin du highlight, rect.x et rect.w */

        max = (curseury > (min = curseuryDeb > curseury ? curseury : curseuryDeb) ? curseury : curseuryDeb);

        /* On remplit d'abord avec un beau bleu xD*/
        Rect lightRect;

        /**
          On pourrait faire w = substr(min, max-min) mais TTF_SizeText rajoute en plus
          l'advance du premier caractère, et donc l'highlight ne tomberait pas juste avec
          certaines polices qui possèdent un advance (Pulse.ttf par exemple)
         **/
        string tmp = texte.substr(0, max);
        w = police.text_width(tmp.c_str());

        tmp = tmp.substr(0, min);
        x = police.text_width(tmp.c_str());

        lightRect.x = (Sint16) x;
        lightRect.w = (Uint16) (w-x);
        lightRect.h = police.line_skip();
        lightRect.y = 0;

        //highlight
        surface.fill(lightRect, Color(153, 202, 242));
    }
    //et puis texte
    //le blended est obligé pour ne pas occulter le highlight
    Surface tmp = police.render_blended(texte.c_str(), textColor);

    tmp.blitTo(surface);
}

MF_MLine::MF_MLine()
        :capacity(1), ncurrentligne(0), barreOn(false), barre_clicOn(false), timer_wasinit(false)
{
    if (!flechebas)
        flechebas = ImageMan.LoadRessource("Arrow.png", true);
    if (!flechehaut)
        flechehaut = ImageMan.LoadRessource("Arrow2.png", true);

    //on fait la premiere ligne, ses pos (tout simplement 0,0).
    xy pos(0,0);
    poslignes.resize(1);
    poslignes[0] = pos;
}

MF_MLine::~MF_MLine()
{
    //on libère le timer
    if (timer_wasinit == true)
    {
        SDL_RemoveTimer(timer_ID);
        timer_wasinit = false;
    }
}

void MF_MLine::update_capacity()
{
    assert(capacity > 0);
    /* on regarde le nombre de lignes en trop */
    int difference = get_nblignes() - capacity;

    if (difference <= 0)
    {
        return;
    }

    /* et on coupe les connections, effacement des dif premiers entiers */
    poslignes.erase(poslignes.begin(), poslignes.begin() + difference);

    int len_to_erase = poslignes[0].x;
    texte.erase(0, len_to_erase);

    for (std::vector<xy>::iterator it = poslignes.begin(); it < poslignes.end(); ++it) //decalage du pointeur
    {
        (*it).x -= len_to_erase;
    }

    //on n'échappe pas à son destin
    update_barre();
}

//voir les SDL_Timer :) Ca a pas été sans erreurs
Uint32 MF_MLine_fonction_timer(Uint32 interval, void *param)
{
    MF_MLine *item = (MF_MLine*)param;

    //si la classe n'est plus la première de son MF_Boss, on retourne
    //ou si la souris n'est plus enfoncée
    if ((item->isFirst == false) || (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1) == 0)) {
        //on enlève le timer
        SDL_RemoveTimer(item->timer_ID);
        item->timer_wasinit = false;
        return 0;
    }

    //Et on simule un clic
    SDL_Event event;
    SDL_MouseButtonEvent bevent;
    bevent.type = SDL_MOUSEBUTTONDOWN;
    bevent.button = SDL_BUTTON_LEFT;
    bevent.state = SDL_PRESSED;
    bevent.x = (item->posx)+item->dims.x;
    bevent.y = (item->posy)+item->dims.y;
    event.button = bevent;

    SDL_PushEvent(&event);
    return SDL_DEFAULT_REPEAT_INTERVAL;
}

void MF_MLine::write (const std::string &texte, unsigned int posx, int posy)
{
    assert(posx < get_nblignes());
    assert(poslignes[posx].y <= posy);

    std::string tmp = texte;

    //on remplace les tab par des espaces
    unsigned int tab(0);
    while ( (tab = tmp.find('\t', tab)) != std::string::npos)
    {
        tmp.erase(tab, 1);
        tmp.insert(tab, "    ");
    }
    //on insere le texte.
    this->texte.insert(get_offset(posx, posy), tmp);
    poslignes[posx].y += tmp.length();
    update_largeur(posx, false);

    if (barreOn)
    {
        //descend la barre
        ncurrentligne = get_nblignes()-(dims.h/police.line_skip());
    }
    set_updated();;
}

void MF_MLine::refresh()
{
    updated = true;

    /* remplissage */
    surface.fill(0, bgColor);

    /* on doit maintenant write le texte
       je vous jure que c'est simple */
    int hauteur = police.line_skip();
    Rect postext(0,0,0,0);

    for (std::vector<xy>::iterator it = poslignes.begin() + ncurrentligne; it != poslignes.end() && postext.y < dims.h; ++it, postext.y += hauteur)
    {
        if ((*it).y == 0) continue;
        Surface tmp = police.render_shaded((texte.substr((*it).x, (*it).y)).c_str(), textColor, bgColor);
        tmp.blitTo(surface, 0, postext);
    }

    /* et cette **** de barre!! */
    if (barreOn)
    {
        int barreh = getbarreh();
        int barrey = getbarrey(barreh);
        displaybarre(barrey, barreh);
    }
}

void MF_MLine::update_barre()
{
    /* pour savoir si on doit mettre ou enlever la barre? */
    unsigned int maxlines = maxlignes();

    if (maxlines < get_nblignes())
    {
        if (barreOn) return;
        /* on doit mettre la barre... */
        barreOn = true;
        /* on refait tout */
        update_largeur(0, true);

        /* on revoit le nombre de ligne pour le curseur */
        /* en fait c'est pour bien positionner la barre */
        ncurrentligne = get_nblignes()-maxlines;
        return;
    }
    /* sinon */
    if (!barreOn) return;
    /* et on update tout encore */
    barreOn = false;
    barre_clicOn = false;
    update_largeur(0, true);
    /* et on met bien le curseur au début */
    ncurrentligne = 0;
}

bool MF_MLine::deal_w_Event(const SDL_Event &event)
{
    /* juste une redirection .*/
    switch (event.type)
    {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return deal_w_key(event.key);
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return deal_w_click(event.button);
        case SDL_MOUSEMOTION:
            return deal_w_motion(event.motion);
        default:
            return 0;
    }
}

//Pour faire défiler le texte
bool MF_MLine::deal_w_key(const SDL_KeyboardEvent &event)
{
    if (event.type == SDL_KEYUP || barreOn == false) return 0;

    /*
      on gère juste touche bas et haut
     */

    if (event.keysym.sym == SDLK_UP)
    {
        /* on monte ncurrentbond d'un cran */
        if (montebarre(1)) set_updated();;
        return 1;
    }
    if (event.keysym.sym == SDLK_DOWN)
    {
        /* on descend ncurrentbond d'un cran */
        if (descendbarre(1)) set_updated();;
        return 1;
    }
    return 0;
}

bool MF_MLine::deal_w_click(const SDL_MouseButtonEvent &buttonevent)
{
    if (!barreOn) return 0;

    /* roue de la souris */
    if (buttonevent.button == SDL_BUTTON_WHEELDOWN)
    {
        if (descendbarre(1)) set_updated();
        else displaybarre(getbarrey(getbarreh()), getbarreh());
        return 1;
    }
    if (buttonevent.button == SDL_BUTTON_WHEELUP)
    {
        if (montebarre(1)) set_updated();
        else displaybarre(getbarrey(getbarreh()), getbarreh());
        return 1;
    }

    if (buttonevent.button != SDL_BUTTON_LEFT) return 0;

    //pos relatives
    posx = buttonevent.x - dims.x;
    posy = buttonevent.y - dims.y;

    if (buttonevent.type == SDL_MOUSEBUTTONDOWN)
    {
        //la seule partie qui nous intéresse est la partie de la barre déroulante
        if (posx < dims.w-16 || posx > dims.w) return 0;
        int maxlines = maxlignes();
        //clique-t-on sur une fleche?
        //flechehaut
        if (posy <= 15)
        {
            if (montebarre(1))
            {
                set_updated();;
                //on ajoute le timer
                if (timer_wasinit == false)
                {
                    timer_ID = SDL_AddTimer(SDL_DEFAULT_REPEAT_DELAY, MF_MLine_fonction_timer, (void*)(this));
                    timer_wasinit = true;
                }
            }
            //sortie
            return 1;
        }
        //flechebas
        if (posy >= dims.h-16)
        {
            if (descendbarre(1))
            {
                set_updated();;
                //on ajoute le timer
                if (timer_wasinit == false)
                {
                    timer_ID = SDL_AddTimer(SDL_DEFAULT_REPEAT_DELAY, MF_MLine_fonction_timer, (void*)(this));
                    timer_wasinit = true;
                }
            }
            //sortie
            return 1;
        }
        //dans la barre?
        int barreh = getbarreh();
        int barrey = getbarrey(barreh);

        if (posy >= barrey && posy < barrey + barreh)
        {
            //si c'est dans la barre
            barre_clicOn = true;
            barreclic_y  = posy;
            //si il y a un timer, on l'enlève
            if (timer_wasinit == true)
            {
                SDL_RemoveTimer(timer_ID);
                timer_wasinit = false;
            }
            return 1;
        }
        //au dessus
        if (posy < barrey)
        {
            // on décale de maxlignes vers le haut
            montebarre(maxlines);
            set_updated();;

            //on ajoute le timer
            if (timer_wasinit == false)
            {
                timer_ID = SDL_AddTimer(SDL_DEFAULT_REPEAT_DELAY, MF_MLine_fonction_timer, (void*)(this));
                timer_wasinit = true;
            }

            //et sortie
            return 1;
        }
        //au dessous
        //c'est l'inverse
        descendbarre(maxlines);
        set_updated();;

        //on ajoute le timer
        if (timer_wasinit == false)
        {
            timer_ID = SDL_AddTimer(SDL_DEFAULT_REPEAT_DELAY, MF_MLine_fonction_timer, (void*)(this));
            timer_wasinit = true;
        }

        //et sortie
        return 1;
    }
    /* c'est SDL_MOUSEBUTTONUP */
    /* on lache le clic */
    //si il y a un timer, on l'enlève
    if (timer_wasinit == true)
    {
        SDL_RemoveTimer(timer_ID);
        timer_wasinit = false;
    }
    if (barre_clicOn == false)
    {
        return 0;
    }
    barre_clicOn = false;
    return 1;
}

bool MF_MLine::deal_w_motion(const SDL_MouseMotionEvent &motion)
{
    /* là le deal est de savoir si on a cliqué la barre */
    if (!barre_clicOn) return 0;

    // pos relative
    int posy = motion.y - dims.y;

    /* donc on doit redisplayr la barre avec les changements. on calcule
      d'abord la pos normale de la barre */
    int maxlines = maxlignes();
    int difference = get_nblignes()-maxlines;
    int barreh = getbarreh();
    int barrey = getbarrey(barreh);

    // on met la difference entre le clic et la pos de la souris
    int difference_y = posy - barreclic_y;

    /* on vérifie que ca va pas trop bas ou trop haut */
    int decalage = barreclic_y - barrey;
    barrey += difference_y;

    if (barrey < 16)
    {
        barrey = 16;
    } else if (barrey > dims.h - 16 - barreh)
    {
        barrey = dims.h - 16 - barreh;
    }

    //on regarde si la nouvelle pos a un n° de ligne different.
    unsigned int noligne = ((barrey - 16) * difference) / (dims.h - barreh - 32);

    barreOn = false;
    if (noligne != ncurrentligne)
    {
        /* on calcule le nombre de lignes ainsi sautées */
        int diff_lignes = noligne-ncurrentligne;

        //maitenant, vers le haut ou bas
        if (diff_lignes > 0)
        {
            //vers le bas. on vérifie que ca dépasse pas le nb de lignes
            diff_lignes = (diff_lignes + ncurrentligne + maxlines >= get_nblignes()) ? get_nblignes() - ncurrentligne - maxlines : diff_lignes;
            if (descendbarre(diff_lignes)) set_updated();;
        } else
        {
            //vers le haut. même chose
            diff_lignes = (ncurrentligne + diff_lignes > 0) ? diff_lignes : -ncurrentligne;
            if (montebarre(-diff_lignes)) set_updated();;
        }
        // si une ligne a bougé on ajuste aussi la valeur en mémoire
        barreclic_y = getbarrey(barreh) + decalage;
    }
    barreOn = true;

    /* réaffichage de la barre */
    displaybarre(barrey, barreh);
    return 1;
}

int MF_MLine::toLen(int noligne, int maxlen) const
{
#ifndef MF_TEXT_ACCENTUE
    int curlen (0);
    int w (0);

    int i (1);
    const char *explore = texte.c_str() + poslignes[noligne].x;

    if (*explore == '\r' || *explore == '\n' || *explore == '\0') return 0;

    TTF_GlyphMetrics(police, *explore, &w, NULL, NULL, NULL, &curlen);

    curlen -= w;

    if (curlen > maxlen) return 0;

    for (++explore; ; ++explore, curlen += w, i++)
    {
        if (*explore == '\r' || *explore == '\n' || *explore == '\0')
        {
            return i;
        }

        TTF_GlyphMetrics(police, *explore, NULL, NULL, NULL, NULL, &w);
        if (curlen + w > maxlen)
        {
            return i;
        }
    }
#else
 #ifdef MF_RETOUR_LIGNE
    int lastpos(0);
 #endif
    int curlen (0);
    char ch;

    int i (1);
    const char *explore = texte.c_str() + poslignes[noligne].x;

    if (*explore == '\r' || *explore == '\n' || *explore == '\0') return 0;

    ch = explore[1];
    *(char*)(void*)(&explore[1]) = 0;

    curlen = police.text_width(explore);
    *(char*)(void*)(&explore[1]) = ch;

    if (curlen > maxlen) return 0;

    const char *ptr = explore;
    for (++explore; ; ++explore, i++)
    {
 #ifdef MF_RETOUR_LIGNE
        if (*explore == '\r' || *explore == '\n' || *explore == '\0')
        {
            return i;
        }
        if (*explore < 'A' || (*explore > 'Z' && *explore < 'a') || *explore > 'z')
        {
            lastpos = i;
        }
        ch = explore[1];
        *(char*)(void*)(&explore[1]) = 0;

        curlen = police.text_width(ptr);
        *(char*)(void*)(&explore[1]) = ch;

        if (curlen > maxlen)
            return lastpos == 0 ? i : lastpos+1;
 #else
        if (*explore == '\r' || *explore == '\n' || *explore == '\0')
        {
            return i;
        }
        ch = explore[1];
        *(char*)(void*)(&explore[1]) = 0;

        curlen = police.text_width(ptr);
        *(char*)(void*)(&explore[1]) = ch;

        if (curlen > maxlen)
            return i;
 #endif
    }
#endif
}

bool MF_MLine::montebarre(int crans)
{
    assert(crans >= 0);
    if (crans == 0 || ncurrentligne == 0) return 0;
    ncurrentligne = (unsigned)crans > ncurrentligne? 0 : ncurrentligne - crans;
    return 1;
}

bool MF_MLine::descendbarre(int crans)
{
    assert(crans >= 0);
    int maxlines = maxlignes();
    int i;
    for (i = crans; ncurrentligne < (get_nblignes() - maxlines) && i > 0; i--, ncurrentligne++)
    {
    }
    return (i!=crans);
}

void MF_MLine::displaybarre(int barrey, int barreh)
{
    /* fleche haut et bas */
    Rect r (dims.w-16, 0, 16,16);
    flechehaut.blitTo(surface, 0, r);

    r.y = dims.h-16;
    flechebas.blitTo(surface, 0, r);

    //fond
    r.y = 16, r.h = dims.h-32;
    surface.fill(r, Color(240, 208, 130));

    /* et pam! trois affichage en série! */
    r.y = barrey, r.h = barreh;
    DrawRect(surface, r, Color(1, 58, 205), 1);
    r.y += 1, r.h -= 2, r.x += 1, r.w = 14;
    DrawRect(surface, r, Color(0xFF, 0xFF, 0xFF), 1);
    r.y += 1, r.h -= 2, r.x += 1, r.w = 12;
    surface.fill(r, Color(65, 172, 205));
}

void MF_MLine::update_largeur(unsigned int noligne, bool suivantes)
{
    int maxlen = maxwidth();
    bool capacity_change (false);

    for (bool next = true; noligne < get_nblignes() && next == true; noligne++)
    {
        //on vérifie la longueur
        int len = toLen(noligne, maxlen);
        int normlen = poslignes[noligne].y;
        //Si le texte déjà écrit est trop long ou trop court, on doit ajuster.
        if (len != normlen)
        {
            poslignes[noligne].y = len;
            int pos = len + poslignes[noligne].x;

            //détruire tout ce qu'il y a après
            if (texte[pos] == '\0')
            {
                if (noligne + 1 < get_nblignes())
                {
                    poslignes.erase(poslignes.begin()+noligne+1, poslignes.end());
                    capacity_change = true;
                }
                break;
            }
            bool aie (false);
            //retour à la ligne, la ligne suivante commence au caractère d'après
            if (texte[pos] == '\n' || texte[pos] == '\r')
                aie = true;

            if (poslignes.size() <= noligne+1)
            {
                poslignes.push_back(xy(pos+aie, 0));
                capacity_change = true;
            } else
            {
                int diff = poslignes[noligne+1].x - pos - aie;
                poslignes[noligne+1].x = pos+aie;
                poslignes[noligne+1].y += diff;
            }
            continue;
        }

        if (texte[len + poslignes[noligne].x] == '\0')
        {
            if (noligne + 1 < get_nblignes())
            {
                poslignes.erase(poslignes.begin()+noligne+1, poslignes.end());
                capacity_change = true;
            }
            break;
        }
        //on ajuste next pour savoir si continuer la boucle
        next = suivantes;
    }
    if (capacity_change)
    {
        update_capacity();
        update_barre();
    }
}

/* récupérer la pos d'un caractère à partir de son offset */
xy MF_MLine::get_abspos(int offset)
{
    assert (offset >= 0);

    std::vector<xy>::iterator it;
    int i;

    for (it = poslignes.begin(), i = 0; it+1 < poslignes.end() && (*it).x+(*it).y < offset; ++it, ++i)
        {}

    return xy(i, (MIN((unsigned)offset, texte.length()))-((*it).x) );
}

void MF_MLine::erase(int depart, int nombre)
{
    if (nombre == -1)
    {
        texte.erase(depart);
    } else
    {
        texte.erase(depart, nombre);
    }
    update_largeur(get_abspos(depart).x, true);
    set_updated();;
}

bool operator > (xy &a, xy &b)
{
    if (a.x > b.x) return true;
    if (a.x < b.x) return false;
    return (a.y > b.y);
}

bool operator < (xy &a, xy &b)
{
    if (a.x < b.x) return true;
    if (a.x > b.x) return false;
    return (a.y < b.y);
}

//affichage. On se contente de rajouter le curseur si besoin
void MF_MWLine::display(Surface &ecran)
{
    second_update = true;
    MF_Line::display(ecran);

    /* la seule diff c'est le curseur , le reste est géré par actualise */
    if (!curseurOn || !isFirst)
        return;

    /* Si on est à plus de 500 ms de lastCursor on le change d'état */
    Uint32 curTime = SDL_GetTicks();
    if (curTime - lastCursor > 500)
    {
        lastCursor = curTime;
        curseurDisp = !curseurDisp;
    }
    /* Si besoin d'displayr le curseur */
    if (curseurDisp)
    {
        Rect cursRect;
        int w;
        string tmp;

        /* Pos du curseur */
        tmp = texte.substr(poslignes[curseurx].x, curseury);
        w = police.text_width(tmp.c_str());

        cursRect.x = w + dims.x;
        cursRect.w = 1;
        cursRect.y = dims.y + (curseurx-ncurrentligne) * police.line_skip();
        cursRect.h = MIN(police.line_skip(), dims.y+dims.h-cursRect.y);


        /* affichage du curseur de la même couleur que celle du texte */
        if (cursRect.y >= dims.y && cursRect.y < dims.y + dims.h)
            ecran.fill(cursRect, textColor);
    }
}

/* actualise la surface en mémoire */
void MF_MWLine::refresh()
{
    /* si pas de highlight on fait juste comme MF_MLine */
    if (!surligneOn)
    {
        MF_MLine::refresh();
        return;
    }

    updated = true;

    /* sinon on doit tracer tout le highlight */
    //fond
    surface.fill(0, bgColor);

    //highlight
    unsigned int maxlines = maxlignes();
    {
        unsigned int min, max; /* min et max du highlight, rect.x et rect.w */
        int w, x;
        xy deb(curseurxDeb, curseuryDeb), fin(curseurx, curseury); /* debut et fin du highlight */

        if (deb > fin)
        {
            xy tmp = fin;
            fin = deb;
            deb = tmp;
        }

        /* On remplit d'abord avec un beau bleu xD*/
        Rect lightRect;

        /* on fait toutes les lignes */
        unsigned int i = MAX(ncurrentligne, (unsigned)deb.x);

        for ( ; i <= (unsigned)fin.x && i <= ncurrentligne+maxlines; i++)
        {
            min = (i == (unsigned)deb.x) ? deb.y : 0;
            max = (i == (unsigned)fin.x) ? fin.y : poslignes[i].y;
            /**
              On pourrait faire w = substr(min, max-min) mais TTF_SizeText rajoute en plus
              l'advance du premier caractère, et donc l'highlight ne tomberait pas juste avec
              certaines polices qui possèdent un advance (Pulse.ttf par exemple)
             **/
            std::string tmp = texte.substr(poslignes[i].x, max);
            w = police.text_width(tmp.c_str());

            tmp = tmp.substr(0, min);
            x = police.text_width(tmp.c_str());

            lightRect.x = (Sint16) x;
            lightRect.w = (Uint16) (w-x);
            lightRect.h = police.line_skip();
            lightRect.y = (i - ncurrentligne) * police.line_skip();

            //highlight
            surface.fill(lightRect, Color(153, 202, 242));
        }
    }
    /* on doit maintenant write le texte
       je vous jure que c'est simple */
    int hauteur = police.line_skip();
    Rect postext(0,0,0,0);

    for (std::vector<xy>::iterator it = poslignes.begin()+ncurrentligne; it != poslignes.end() && postext.y < dims.h;
            ++it, postext.y += hauteur)
    {
        if (it->x != it->y)
        {
            Surface tmp = police.render_blended((texte.substr(it->x, it->y)).c_str(), textColor);
            tmp.blitTo(surface, 0, postext);
        }
    }

    /* et cette barre chérie!! */
    if (barreOn)
    {
        int barreh = getbarreh();
        int barrey = getbarrey(barreh);
        displaybarre(barrey, barreh);
    }
}

/* nouvelles fonctions pour écrire */
/* elles updatent aussi le curseur :) */
void MF_MWLine::write (const std::string &texte, unsigned int posx, unsigned int posy)
{
    assert(posx < get_nblignes());
    assert((unsigned)poslignes[posx].y >= posy);

    std::string tmp = texte;

    //on remplace les tab par des espaces
    unsigned int tab(0);
    while ( (tab = tmp.find('\t', tab)) != std::string::npos)
    {
        tmp.erase(tab, 1);
        tmp.insert(tab, "    ");
    }

    curseurx = posx;
    curseury = posy;

    int pos = get_offset(posx, posy) + tmp.length();

    this->texte.insert(get_offset(posx , posy), tmp);

    poslignes[posx].y += tmp.length();
    for (unsigned int i = posx+1; i < poslignes.size(); i++)
    {
        poslignes[i].x += tmp.length();
    }
    update_largeur(posx, true);
    set_abspos_curseur(pos);

    rebarre();
    set_updated();;
}

/* gestion évènements pour faire defiler texte et écrire */
bool MF_MWLine::deal_w_motion(const SDL_MouseMotionEvent &motionevent)
{
    // si c'est la barre
    if (barre_clicOn)
    {
        return MF_MLine::deal_w_motion(motionevent);
    }
    //si c'est pas cliqué on quitte
    if (!clicOn) return 0;

    //sinon on se charge du surlignage
    /* On regarde d'abord si le motion est dans les lignes en hauteur*/
    if (motionevent.y >= dims.y && motionevent.y <= dims.y+dims.h)
    {
        /* puis en largeur */
        int ligne = (motionevent.y-dims.y) / police.line_skip();
        if (ligne < (int)get_nblignes() - (int)ncurrentligne*barreOn)
        {
            curseurx = ncurrentligne + ligne;
            curseury = toLen(curseurx, MIN(motionevent.x-dims.x, dims.w-barreOn*16));
        }
    }
    set_updated();;
    return 1;
}

bool MF_MWLine::deal_w_key(const SDL_KeyboardEvent &keyevent)
{
    if (keyevent.type == SDL_KEYUP)
        return 0;

    if (!ecrivable)
    {
        switch (keyevent.keysym.sym)
        {
            case SDLK_BACKSPACE:
            case SDLK_RETURN:
            case SDLK_TAB:
            case SDLK_DELETE:
                return 0;
            default:
                break;
        }
    }

    int refresh = 0;

    switch (keyevent.keysym.sym)
    {
        case SDLK_BACKSPACE:
            if (surligneOn)
                clearHighlight();
            else if (get_offset_curseur() > 0)
            {
                erase(get_offset_curseur()-1, 1);
            }
            refresh = 1;
            break;
        case SDLK_RETURN:
            if (surligneOn)
                clearHighlight();
            write("\n");
            set_updated();
            refresh = 1;
            break;
        case SDLK_TAB:
            if (surligneOn)
                clearHighlight();
            write("    ");
            set_updated();
            refresh = 1;
            break;
        case SDLK_DELETE:
            if (surligneOn)
                clearHighlight();
            else if (get_offset_curseur() != get_offset())
                erase(get_offset_curseur(), 1);
            refresh = 1;
            break;
        case SDLK_PAGEUP:
            montecurseur(dims.h/police.line_skip());
            set_updated();
            refresh = 1;
            break;
        case SDLK_HOME:
            if (updateHighlight() || surligneOn)
            {
                refresh = 1;
            }
            curseury = 0;
            if (refresh)
            {
                set_updated();
            }
            refresh = 1;
            break;
        case SDLK_PAGEDOWN:
            descendcurseur(dims.h/police.line_skip());
            set_updated();
            refresh = 1;
            break;
        case SDLK_END:
            if (updateHighlight() || surligneOn)
            {
                refresh = 1;
            }
            curseury = poslignes[curseurx].y;
            if (refresh)
            {
                set_updated();
            }
            refresh = 1;
            break;
        case SDLK_UP:
            updateHighlight();
            montecurseur(1);
            set_updated();
            refresh = 1;
            break;
        case SDLK_DOWN:
            updateHighlight();
            descendcurseur(1);
            set_updated();
            refresh = 1;
            break;
        case SDLK_LEFT:
            if (updateHighlight() || surligneOn)
            {
                refresh = 1;
            }
            if (curseury > 0)
                --curseury;
            else if (curseurx>0)
            {
                montecurseur(1);
                curseury = poslignes[curseurx].y;
                refresh = 1;
            }
            if (refresh == 1)
            {
                set_updated();
            }
            refresh = 1;
            break;
        case SDLK_RIGHT:
            if (updateHighlight() || surligneOn)
            {
                refresh = 1;
            }
            if (curseury < poslignes[curseurx].y)
                ++curseury;
            else if (curseurx < get_nblignes()-1 )
            {
                descendcurseur(1);
                curseury = 0;
                refresh = 1;
            }
            if (refresh == 1)
            {
                set_updated();
            }
            refresh = 1;
            break;
        default:
            break;
    }


    if (refresh)
    {
        /* Si il y a une touche pressée active
           on actualise et on remet le timer du
           curseur à 0
           --
           Ca veut dire que le curseur s'display
           obligatoirement, et pendant 500 ms au
           moins */
        second_update = false;
        lastCursor = SDL_GetTicks();
        curseurDisp = true;
        return refresh;
    }

    if (keyevent.keysym.unicode == 0)
    {
        /* c'est une touche spéciale */
        /* Qu'on n'a pas traité avant (ex: alt, shift...) */
        return 1;
    }
    /* si alt ou ctrl */
    if (ALT_ON or CTRL_ON)
    {
        return 0;
    }

    if (!ecrivable)
        return 0;

    /* C'est une touche ASCII */
    clearHighlight();
    char ch = keyevent.keysym.unicode;
    write(ch);
    set_updated();
    lastCursor = SDL_GetTicks();
    curseurDisp = true;
    return 1;
}

bool MF_MWLine::deal_w_click(const SDL_MouseButtonEvent &buttonevent)
{
    if (MF_MLine::deal_w_click(buttonevent)) return 1;

    if (buttonevent.button != SDL_BUTTON_LEFT)
    {
        return 0;
    }

    if (buttonevent.type == SDL_MOUSEBUTTONUP)
    {
        if (clicOn == true)
        {
            clicOn = false;
            return 1;
        }
        return 0;
    }
    //on cherche la ligne en hauteur
    int posx = buttonevent.x - dims.x;
    int posy = buttonevent.y - dims.y;

    if (posx < 0 || posy < 0 || posx > dims.w || posy > dims.h) return 0;

    unsigned int ligne = (posy/police.line_skip());

    updateHighlight();

    if (ligne >= get_nblignes() - ncurrentligne*barreOn)
    {
        return 0;
    }

    curseurx = (ncurrentligne*(barreOn==true)) + ligne;
    curseury = toLen(curseurx, posx);
    clicOn = true;
    if (updateHighlight())
    {
        set_updated();;
    }
    return 1;
}

void MF_MWLine::montecurseur(int crans)
{
    assert (crans >= 0);
    curseurx = MAX((int)curseurx - (int)crans, 0);
    int len = poslignes[curseurx].y;
    curseury = MIN(curseury, len);
    //on doit chercher avec la barre
    rebarre();
}

void MF_MWLine::descendcurseur(int crans)
{
    assert (crans >= 0);
    curseurx = MIN(curseurx + crans, get_nblignes()-1);
    int len = poslignes[curseurx].y;
    curseury = MIN(curseury, len);
    //on doit chercher avec la barre
    rebarre();
}

void MF_MWLine::rebarre()
{
    if (barreOn == false) return;
    //SI BARRE TROP BASSE
    if (ncurrentligne > curseurx)
    {
        montebarre(ncurrentligne-curseurx);
        return;
    }
    //SI BARRE TROP HAUTE
    int maxlines = maxlignes();
    if (ncurrentligne + maxlines <= curseurx)
    {
        descendbarre(curseurx+1-maxlines-ncurrentligne);
        return;
    }
    //SI CURSEUR TROP BAS
    if (ncurrentligne + maxlines > get_nblignes())
    {
        montebarre(maxlines+ncurrentligne-get_nblignes());
        return;
    }
}

void MF_MWLine::update_capacity()
{
    assert(capacity > 0);
    /* on regarde le nombre de lignes en trop */
    int difference = get_nblignes() - capacity;

    if (difference <= 0)
    {
        return;
    }

    /* et on coupe les connections, effacement des dif derniers entiers */
    poslignes.erase(poslignes.end()-difference, poslignes.end());

    texte.erase(poslignes[capacity-1].x + poslignes[capacity-1].y);

    //on n'échappe pas à son destin
    update_barre();
}

void MF_MWLine::printstatut()
{
    cout << "Statut:";
    cout << "\n\tnblignes: " << get_nblignes();
    cout << "\n\tcurseurx: " << curseurx;
    cout << "\n\tcurseury: " << curseury;
    cout << "\n\tligne1 : " << poslignes[0].y;
    cout << endl;
}


void MF_MWLine::updatebarre()
{
    int pos = get_offset_curseur();
    MF_MLine::update_barre();
    set_abspos_curseur(pos);
}

void MF_MWLine::startHighlight()
{
    if (surligneOn == true)
        return;

    surligneOn = true;
    curseuryDeb = curseury;
    curseurxDeb = curseurx;
}

bool MF_MWLine::clearHighlight()
{
    if (surligneOn == false)
        return false;
    surligneOn = false;

    int off1, off2, min, max; /* debut et fin du highlight */

    off1 = get_offset(curseurx, curseury);
    off2 = get_offset(curseurxDeb, curseuryDeb);

    min = MIN(off1, off2);
    max = MAX(off1, off2);

    if (min == max) {
        return false;
    }

    /* On efface là ou c'est highlighted */
    erase (min, max - min);

    /* et on remet le curseur au début du highlight */
    set_abspos_curseur(min);
    return true;
}

void MF_MWLine::erase(int depart, int nombre)
{
    int pos = get_offset_curseur();
    if (nombre == -1)
    {
        if (pos > depart) pos = depart;
        texte.erase(depart);
    } else
    {
        if (pos > depart+nombre)
        {
            pos -= nombre;
        } else if (pos > depart)
        {
            pos = depart;
        }
        texte.erase(depart, nombre);
    }
    update_largeur(get_abspos(depart).x, true);
    set_abspos_curseur(pos);
    rebarre();
    set_updated();;
}
