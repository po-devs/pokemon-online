#include "interfont.hpp"
#include "exception.hpp"

#include <iostream>
using namespace std;

namespace interface
{

Font::Font() :ref_count(NULL)
{
    ;
}

Font::Font(TTF_Font *font) : ref_count(NULL)
{
    load(font);
}

Font::Font(const Font &font) : ref_count(NULL)
{
    *this = font;
}

Font::Font(const char *filename, Uint16 ptsize)
    : ref_count(NULL)
{
    load(filename, ptsize);
}

Font::~Font()
{
    free();
}

Font & Font::operator = (const Font &font)
{
    if (ref_count == font.ref_count)
        return *this;
    free();
    if (!font.ref_count)
        return *this;
    ref_count = font.ref_count;
    ++(*ref_count);
    f = font.f;

    return *this;
}

void Font::free()
{
    if (ref_count)
    {
        if (--(*ref_count) == 0)
        {
            TTF_CloseFont(f);
            delete ref_count;
        }
        ref_count = NULL;
    }
}

void Font::init()
{
    ref_count = new size_t(1);
}

void Font::load(TTF_Font *font)
{
    free();
    if (font = NULL)
        throw InterfaceException("Font::load(font): font NULL passée en paramètre!");
    init();
    f = font;
}

void Font::load(const char *path, Uint16 ptsize)
{
    free();

    f = TTF_OpenFont(path, ptsize);
    if (f == NULL)
        throw InterfaceException(std::string() + "Font::load(path, ptsize) -- Chargement de la police raté! -- " + path);

    init();
}

Font::operator bool () const
{
    return (ref_count);
}

Uint16 Font::line_skip() const
{
    return TTF_FontLineSkip(f);
}

void Font::text_size(const char *text, int *w, int *h) const
{
    TTF_SizeText(f, text, w, h);
}

Uint16 Font::text_width(const char *text) const
{
    int w;
    TTF_SizeText(f, text, &w, NULL);
    return w;
}

Uint16 Font::text_height(const char *text) const
{
    int h;
    TTF_SizeText(f, text, &h, NULL);
    return h;
}

FontStyle Font::get_style() const
{
    return TTF_GetFontStyle(f);
}

void Font::style(const FontStyle &stylist) const
{
    TTF_SetFontStyle(f, stylist);
}

Surface Font::render_shaded(const char *text, const Color &textcolor, const Color &bgColor) const
{
    SDL_Surface *s = TTF_RenderText_Shaded(f,text,textcolor,bgColor);

    if (!s)
        throw InterfaceException("Font::render_shaded");

    return Surface(s);
}

Surface Font::render_blended(const char *text, const Color &textcolor) const
{
    SDL_Surface *s = TTF_RenderText_Blended(f,text,textcolor);
    if (s == NULL)
        throw InterfaceException("Font::render_blended");

    return Surface(s);
}

size_t Font::refcount() const
{
    if (!ref_count)
        return 0;
    else
        return *ref_count;
}

} //namespace interface
