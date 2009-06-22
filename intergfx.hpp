#ifndef INTERGFX_HPP_INCLUDED
#define INTERGFX_HPP_INCLUDED

#include "SDL_gfx.h"
#include "intervideo.hpp"

namespace interface
{
    void DrawRect(Surface &surface, const Rect &r, const Color &col, Uint8 bordersize);
    void DrawFilledRect(Surface &surface, const Rect &r, const Color &borderColor, const Color &fillingColor, Uint8 bordersize);
} // namespace interface

#endif // INTERGFX_HPP_INCLUDED
