#ifndef INTERFACE_HPP_INCLUDED
#define INTERFACE_HPP_INCLUDED

#include <SDL/SDL.h>

namespace interface
{

class Surface;

struct Rect : public SDL_Rect
{
    Rect(Sint16 x=0,Sint16 y=0, Uint16 w=0, Uint16 h=0);
    bool isNull() const;
};

struct Color : public SDL_Color
{
    //Set everything to base
    Color (Uint8 base=0);
    //Normal
    Color(Uint8 r, Uint8 g, Uint8 b=0);
    /* interne */
    Uint32 map(const Surface &s) const;
};

typedef Color ColorKey;

class Surface
{
public: /* Privé! ou risqué */
    void create (SDL_Surface *ext);
    explicit Surface (SDL_Surface *ext);
public: /* safe */
    mutable SDL_Surface *s;

    /** METHODES **/
    Surface();
    Surface(const Surface &s);
    Surface(const char *filename, bool raw = false);
    Surface(const char *filename, const Color & colorkey, bool raw = false);
    Surface(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags=0);
    Surface(SDL_RWops *rw_data, bool close_the_RW);
    ~Surface();
    Surface &operator = (const Surface &s);
    operator bool ();

    //charger -- avec ou sans optimisations par rapport au format de l'écran
    //le sans optimisation peut être parfois nécessaire pour certains formats d'images
    void raw_load(const char * filename);
    void raw_load(const char * filename, const Color & colorkey);
    void load(const char * filename);
    void load(const char * filename, const Color & colorkey);
    void load(SDL_RWops *rw_data, bool close_the_RW);

    //créer -- et bien sûr détruire l'ancienne surface
    void create(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags=0);

    //s'adapter au format d'affichage
    void adapt();

    //copier à partir d'une autre surface
    void shareFrom (const Surface &s);

    //avoir le ref count
    size_t refcount() const;

    //Et se libérer
    void free();

    //accesseurs
    Uint16 w();
    Uint16 h();

    //dessiner
    void blitTo(Surface &dest_surf, const Rect &clip_src=Rect(), const Rect &clip_dest=Rect());
    void fill(const Rect &dest = Rect(), const Color &col = Color());

    //transparence et colorkey
    void alpha(Uint8 alpha);
    void colorkey(const Color &colorkey);
};

class Window : public Surface
{
public:
    Window();
    Window(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags);
    ~Window();

    void create(Uint16 bpp, Uint16 w, Uint16 h, Sint32 flags=0);

    //Retourne l'écran ;)
    void flip();

    //Diverses fonctions pour updater l'écran qu'en partie
    void update_rect(const Rect &r);
    template <class iterator>
    void update_rects(iterator debut, iterator fin);
};

}//namespace interface

#endif // INTERFACE_HPP_INCLUDED
