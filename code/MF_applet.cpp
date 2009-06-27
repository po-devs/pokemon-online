#include "MF_applet.hh"

using namespace std;

MF_Applet::MF_Applet(Uint16 w, Uint16 h, const Color &rgb, Sint16 x, Sint16 y)
{
    bgColor = rgb;
    setRect(x, y, w, h);
}

//pour imprimer du texte
void MF_Applet::drawString(const Font &police, const char *texte, Sint16 x, Sint16 y, const Color &color)
{
    //on découpe le texte par les \n
    Uint16 FLS = police.line_skip();

    char copy[strlen(texte) + 1];
    strcpy(copy, texte);

    //on fait une boucle en prenant morceau par morceaux
    //on peut rien faire contre deux \n d'affilée(le vide compte pas)
    for (char *tmp = strtok(copy, "\n\r"); tmp != NULL; y+=FLS, tmp = strtok(NULL, "\n\r"))
    {
        Surface tmp_surf = police.render_shaded(tmp, color, bgColor);

        Rect r (x, y);
        tmp_surf.blitTo(surface, 0, r);
    }
    set_updated();
}

void MF_Applet::drawStyledString(const Font &police, const char *texte, Sint16 x, Sint16 y, const FontStyle &flags, const Color &rgb)
{
        FontStyle oldstyle = police.get_style();
        police.style(flags);
        drawString(police, texte, x, y, rgb);
        police.style(oldstyle);
}

void MF_Applet::display(Surface &surface)
{
    this->surface.blitTo(surface, 0, dims);

    MF_Box_DrawBorders(dims, surface, true);
}

//Pour copier une image sur la surface
void MF_Applet::BlitImage(Sint16 x, Sint16 y, const char *filename, bool colorkeyOn, Color *colorkey)
{
    Surface img;

    if (colorkeyOn)
    {
        img = ImageMan.LoadRessource(filename, *colorkey, true);
    } else
    {
        img = ImageMan.LoadRessource(filename, true);
    }

    Rect r (x, y);
    img.blitTo(surface, 0, r);

    set_updated();
}

void MF_Applet::BlitSurface(Surface &src, const Rect& src_part, Sint16 x_dest, Sint16 y_dest)
{
    Rect dest_rect (x_dest, y_dest);
    set_updated();
    src.blitTo(surface, src_part, dest_rect);
}

void MF_Applet::FillRect(const Rect &rect, const Color &col)
{
    set_updated();
    surface.fill(rect, col);
}

MF_HLApplet::MF_HLApplet(Uint16 w, Uint16 h, const Color &rgb, Sint16 x, Sint16 y, const Color &hl)
            :MF_Applet(w, h, rgb, x, y), HLColor(hl)
{
}

void MF_HLApplet::display(Surface &ecran)
{
    surface.blitTo(ecran, 0, dims);

    //et on rajoute un petit fond vert
    if (hovered == true)
    {
        Surface tmp(16, dims.w, dims.h);
        tmp.alpha(100);
        tmp.fill(0, HLColor);
        tmp.blitTo(ecran, 0, dims);
    }
    MF_Box_DrawBorders(dims, ecran, true);
}

int MF_HLApplet::set_hover_state(int enable)
{
    if (enable == -1) return hovered;
    else if (hovered != enable)
    {
        set_updated(), hovered = enable;
        return true;
    }
    return false;
}

MF_BApplet::MF_BApplet(Uint16 w, Uint16 h, const Color &rgb, Sint16 x, Sint16 y, const Color &hl)
           :MF_HLApplet(w, h, rgb, x, y, hl),
            clicOn(false)
{}

//pour les clics
bool MF_BApplet::deal_w_Event(const SDL_Event &event)
{
    switch(event.type)
    {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && hovered)
            {
                if (!clicOn)
                {
                    clicOn = true;
                    set_updated();
                }
                sendToBoss("clic");
                return 1;
            }
            return 0;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT && clicOn == true)
            {
                clicOn = false;
                set_updated();

                if (hovered)
                    sendToBoss("release");
                else
                	sendToBoss("false-release");
                return 1;
            }
            return 0;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
            {
                sendToBoss("clic");
                sendToBoss("release");
                return 1;
            }
            return MF_Directions::deal_w_key(event.key);
        default:
            return MF_Base::deal_w_Event(event);
    }
}

//encore une nouvelle fonction pour displayr!
void MF_BApplet::display(Surface &ecran)
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

    //et on rajoute un petit fond de highlight :)
    if (hovered == true)
    {
        Surface tmp (16, dims.w, dims.h);
        tmp.alpha(100);
        tmp.fill(0, HLColor);
        tmp.blitTo(ecran, 0, dims);
    }
    //Si cliqué, on inverse la bordure pour donner un effet
    MF_Box_DrawBorders(dims, ecran, !clicOn);
}

MF_HoverMenu::~MF_HoverMenu()
{
}

void MF_HoverMenu::display(Surface &surface)
{
    //C'est bien un '=', et non pas un '=='
    if (image_displayed = hovered)
    {
        image_1.blitTo(surface, 0, dims);
    } else
    {
        image_2.blitTo(surface, 0, dims);
    }
}

bool MF_HoverMenu::deal_w_Event(const SDL_Event &event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        sendToBoss("clic");
        return true;
    }
    if (event.type == SDL_KEYDOWN)
    {
        return deal_w_key(event.key);
    }
    return false;
}

MF_ImHLApplet::MF_ImHLApplet(const char *path, Sint16 x, Sint16 y, const Color &hl)
              :MF_Base(x, y), HLColor(hl), surface(ImageMan.LoadRessource(path))
{
    dims.w = surface.w();
    dims.h = surface.h();
}

int MF_ImHLApplet::set_hover_state(int enable)
{
    if (enable == -1) return hovered;
    else if (hovered != enable)
    {
        set_updated(), hovered = enable;
        return true;
    }
    return false;
}

void MF_ImHLApplet::display(Surface &ecran)
{
    surface.blitTo(ecran, 0, dims);

    if (!hovered)
    {
        return;
    }

    //et on rajoute un petit fond vert
    Surface tmp(16, dims.w, dims.h);
    tmp.alpha(100);
    tmp.fill(0, HLColor);
    tmp.blitTo(ecran, 0, dims);
}

bool MF_ImHLApplet::deal_w_Event(const SDL_Event &event)
{
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT || !isIn(event.button.x, event.button.y))
    {
        return false;
    }
    sendToBoss("clic");
    return true;
}

MF_Button::~MF_Button()
{
}

void MF_Button::display(Surface &surface)
{
    if (!clicOn)
    {
        image_1.blitTo(surface, 0, dims);
    } else
    {
        image_2.blitTo(surface, 0, dims);
    }
}

bool MF_Button::deal_w_Event(const SDL_Event &event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        if (hovered)
        {
            sendToBoss("clic");
            clicOn = true;
            set_updated();
            return true;
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && clicOn)
    {
        clicOn = false;
        set_updated();

        if (hovered)
            sendToBoss("release");
        else
            sendToBoss("false-release");
        return true;
    }
    if (event.type == SDL_KEYDOWN)
    {
        return deal_w_key(event.key);
    }
    return false;
}

MF_Button::MF_Button (const char* img_path_1, const char *img_path_2)
    : clicOn(false), image_1(ImageMan.LoadRessource(img_path_1,true)),
        image_2(ImageMan.LoadRessource(img_path_2,true))
{
    ;
}
