#ifndef INTERFONT_HPP_INCLUDED
#define INTERFONT_HPP_INCLUDED

#include <SDL/SDL_ttf.h>
#include "intervideo.hpp"

namespace interface
{
    typedef Uint32 FontStyle;

    class Font
    {
        public:
            mutable TTF_Font *f;
            size_t *ref_count;
            explicit Font(TTF_Font *font);
            void load(TTF_Font *font);
            void init();
        public:
            Font();
            Font(const Font &font);
            Font(const char *filename, Uint16 ptsize);
            ~Font();
            Font & operator = (const Font &other);

            //charge une font
            void load(const char *filename, Uint16 ptsize);

            //height of a regular text line of this font, size of a string
            Uint16 line_skip() const;
            void text_size(const char *text, int *w, int *h) const;
            Uint16 text_width(const char *text) const;
            Uint16 text_height(const char *text) const;

            //get / set font style
            FontStyle get_style() const;
            void style(const FontStyle &stylist) const;

            //render text
            Surface render_shaded(const char *text, const Color &textcolor, const Color &bgColor) const;
            Surface render_blended(const char *text, const Color &textcolor) const;

            //libère la police
            void free();

            //en etat d'écrire
            operator bool () const;

            //renvoie le nombre de références
            size_t refcount() const;
    };
} // namespace interface

#endif // INTERFONT_HPP_INCLUDED
