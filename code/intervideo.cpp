//!@file interface.cpp

#include "intervideo.hpp"
#include "exception.hpp"
#include <string>
#include <SDL/SDL_image.h>

using std::string;

namespace interface
{

Rect::Rect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    this->x = x, this->y = y, this->w = w, this->h = h;
}

bool Rect::isNull() const
{
    return x==0 && y==0 && w==0 && h==0;
}

Color::Color(Uint8 base)
{
    r=base, g=base, b=base;
}

Color::Color(Uint8 r, Uint8 g, Uint8 b)
{
    this->r = r, this->g = g, this->b = b;
}

Uint32 Color::map(const Surface &s) const
{
    return SDL_MapRGB(s.s->format, r,g,b);
}

void Surface::create (SDL_Surface *ext)
{
    free();
    s = ext;
}

Surface::Surface(SDL_Surface *ext)
{
    s = ext;
    if (s == NULL)
    {
        throw InterfaceException("Surface::Surface(SDL_Surface*) : surface NULL passée en paramètre!");
    }
}

Surface::Surface() : s(NULL)
{
    ;
}

Surface::Surface(const Surface &s) : s(NULL)
{
    shareFrom(s);
}

Surface::Surface(const char *filename, bool raw) : s(NULL)
{
    if (raw)
        raw_load(filename);
    else
        load(filename);
}

Surface::Surface(const char *filename, const ColorKey &colorkey, bool raw) : s(NULL)
{
    if (raw)
        raw_load(filename, colorkey);
    else
        load(filename, colorkey);
}

Surface::Surface(SDL_RWops *rw, bool close_the_RW) : s(NULL)
{
    load(rw, close_the_RW);
}

Surface::Surface(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags) : s(NULL)
{
    create(bpp,w,h,flags);
}

Surface::~Surface()
{
    free();
}

Surface& Surface::operator = (const Surface &s)
{
    shareFrom(s);
    return *this;
}

//charger
void Surface::raw_load(const char *filename)
{
    free();

    s = IMG_Load(filename);

    if (!s)
        throw InterfaceException(string() + "Surface::raw_load() -- loading failed: " + filename);
}

void Surface::raw_load(const char *filename, const Color &colorkeys)
{
    raw_load(filename);
    colorkey(colorkeys);
}

void Surface::load(const char * filename)
{
    raw_load(filename);
    adapt();
}

void Surface::load(SDL_RWops *ops, bool close_the_RW)
{
    free();

    SDL_Surface *temp = IMG_Load_RW(ops, close_the_RW);
    if (temp == NULL)
    {
        throw InterfaceException(string() + "Surface::load() with RWops -- loading failed");
    }
    s = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    if (s == NULL)
    {
        throw InterfaceException("Surface::load() -- SDL_DisplayFormat failed");
    }
}

void Surface::load(const char * filename, const ColorKey &colorkey)
{
    raw_load(filename, colorkey);
    adapt();
}

//créer
void Surface::create(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags)
{
    free();
    s = SDL_CreateRGBSurface(flags,w,h,bpp,0,0,0,0);
    if (s == NULL) throw InterfaceException("Surface::create(bpp,w,h,flags) -- error with SDL_CreateRGBSurface");
}

//s'adapter
void Surface::adapt()
{
    if (s == NULL)
        return;

    SDL_Surface *s2 = SDL_DisplayFormat(s);

    if (s2 == NULL)
        throw InterfaceException("Surface::adapt -- error with SDL_DisplayFormat");

    free();
    s = s2;
}

void Surface::fill(const Rect &r, const Color &color)
{
    Uint32 c = color.map(*this);

    if (r.isNull())
    {
        if (SDL_FillRect(s, 0, c) < 0)
            throw InterfaceException("Surface::fill -- error with SDL_FillRect");
    } else
    {
        Rect r2(r);
        if (SDL_FillRect(s, &r2, c) < 0)
            throw InterfaceException("Surface::fill -- error with SDL_FillRect");
    }
}

//partager à partir d'une autre surface
void Surface::shareFrom (const Surface &ext)
{
    if (s == ext.s)
        return;
    free();
    s = ext.s;
    if (s != NULL)
    {
        //On utilise une fonctionnalité SDL qui permet d'avoir un compteur de références
        s->refcount++;
    }
}

//Se libérer
void Surface::free()
{
    SDL_FreeSurface(s);
    s = NULL;
}

//L'alpha
void Surface::alpha(Uint8 alpha)
{
    if (SDL_SetAlpha(s,SDL_SRCALPHA|SDL_RLEACCEL,alpha) < 0)
        throw InterfaceException("Surface::alpha -- error");
}

/* Change le colorkey de la surface avec la nouvelle couleur */
void Surface::colorkey(const Color &color)
{
    if (SDL_SetColorKey(s,SDL_SRCCOLORKEY|SDL_RLEACCEL, color.map(*this)) < 0)
        throw InterfaceException("Surface::colorkey -- error");
}

//accesseurs
Uint16 Surface::w()
{
    return s->w;
}

Uint16 Surface::h()
{
    return s->h;
}

//dessiner
void Surface::blitTo(Surface &dest_surf, const Rect &clip_src, const Rect &clip_dest)
{
    /* Si le rectangle est de coordonnées nulles on fournit NULL, qui veut dire "toute la surface" */
    Rect * clip_s = clip_src.isNull() ? NULL : const_cast<Rect *>(&clip_src);
    Rect * clip_d = clip_dest.isNull() ? NULL : const_cast<Rect *>(&clip_dest);

    if (SDL_BlitSurface(s, clip_s, dest_surf.s, clip_d) < 0)
        throw InterfaceException("Surface::blitTo -- error with SDL_BlitSurface)");
}

//refcount
size_t Surface::refcount() const
{
    if (s)
        return s->refcount;
    else
        return 0;
}

Surface::operator bool ()
{
    return (s);
}

Window::Window()
{
}

Window::Window(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags)
{
    create(bpp,w,h,flags);
}

Window::~Window()
{
}

void Window::create(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags)
{
    free();
    s = SDL_SetVideoMode(w,h,bpp,flags);
    if (s == NULL)
        throw InterfaceException("Window::create() -- Error with SDL_SetVideoMode");
}

//Retourne l'écran ;)
void Window::flip()
{
    SDL_Flip(s);
}

//Diverses fonctions pour updater l'écran qu'en partie
void Window::update_rect(const Rect &r)
{
    SDL_UpdateRect(s,r.x,r.y,r.w,r.h);
}

template <class iterator>
void Window::update_rects(iterator debut, iterator fin)
{
    SDL_UpdateRects(s, debut, fin-debut);
}

} //namespace interface
