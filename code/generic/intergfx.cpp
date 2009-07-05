#include "intergfx.hpp"

namespace interface
{

    void DrawRect(Surface &surface, const Rect &working_rect, const Color &col, Uint8 bordersize)
    {
            Rect current_rect;

            //maintenant on fait les quatres bords du rectangles.
            ///BORD haut
            current_rect.x = working_rect.x, current_rect.w = working_rect.w, current_rect.y  = working_rect.y, current_rect.h = bordersize;
            surface.fill(current_rect, col);

            ///BORD bas
            current_rect.y += working_rect.h - bordersize;
            surface.fill(current_rect, col);

            //Revient à tester s'il s'agit d'un rectangle normal
            if (working_rect.h <= 2 * bordersize) return;

            ///BORD gauche
            current_rect.h = working_rect.h - 2 * bordersize, current_rect.w = bordersize, current_rect.y  = working_rect.y + bordersize;
            surface.fill(current_rect, col);

            ///BORD droit
            current_rect.x += working_rect.w - bordersize;
            surface.fill(current_rect, col);
    }

    void DrawFilledRect(Surface &surface, const Rect &r, const Color &borderColor, const Color &fillingColor, Uint8 bordersize)
    {
        DrawRect(surface, r, borderColor, bordersize);

        Sint16 wfond = r.w-2*bordersize;
        Sint16 hfond = r.h-2*bordersize;
        if (wfond > 0 && hfond > 0)
        {
            Rect rp(r.x + bordersize, r.y + bordersize, wfond, hfond);
            surface.fill(rp, fillingColor);
        }
    }
}
