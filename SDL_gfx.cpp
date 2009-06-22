//SDL_gfx.c
#include <algorithm>
#include "SDL_gfx.h"

Uint16 TTF_TextWidth(TTF_Font *police, const char *texte)
{
    int w;
    TTF_SizeText(police, texte, &w, NULL);

    return w;
}

SDL_Surface *IMG_LoadColorkey(const char *path, SDL_Color colorkey)
{
    SDL_Surface *surf = IMG_Load(path);
    SDL_SetColorKey(surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(surf->format, colorkey.r, colorkey.g, colorkey.b));
    return surf;
}

SDL_Color SDL_CreateColor(Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Color c = {r, g, b};
    return c;
}

SDL_Rect SDL_CreateRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    SDL_Rect r = {x, y, w, h};
    return r;
}

Uint32 SDL_MapColor(SDL_PixelFormat *fmt, const SDL_Color &color)
{
    return SDL_MapRGB(fmt, color.r, color.g, color.b);
}

int SDL_IsInCircle(Sint16 c_x, Sint16 c_y, Uint16 rayon, Sint16 x, Sint16 y)
{
    return (x-c_x)*(x-c_x) + (y-c_y)*(y-c_y) <= rayon * rayon;
}
int SDL_DrawRect(SDL_Surface *surface, SDL_Rect *dest, Uint32 bordercolor, Uint8 bordersize)
{
    if (surface == NULL)
    {
        SDL_SetError("Surface nulle passée en argument à SDL_DrawRect");
        return -1;
    }

    {
        SDL_Rect working_rect, current_rect;
        int statut;

        if (dest == NULL)
        {
            //si dest est NULL, on fait toute la taille de la surface
            working_rect.x = 0, working_rect.y = 0, working_rect.w = surface->w, working_rect.h = surface->h;
        } else
        {
            working_rect = *dest;
        }

        //maintenant on fait les quatres bords du rectangles.
        ///BORD haut
        current_rect.x = working_rect.x, current_rect.w = working_rect.w, current_rect.y  = working_rect.y, current_rect.h = bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        ///BORD bas
        current_rect.y += working_rect.h - bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        //Revient à tester s'il s'agit d'un rectangle normal
        if (working_rect.h <= 2 * bordersize) return 0;

        ///BORD gauche
        current_rect.h = working_rect.h - 2 * bordersize, current_rect.w = bordersize, current_rect.y  = working_rect.y + bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        ///BORD droit
        current_rect.x += working_rect.w - bordersize;
        return SDL_FillRect(surface, &current_rect, bordercolor);
    }
}

void setPixel(SDL_Surface *surface, int x, int y, Uint32 coul)
{
    *((Uint32*)(surface->pixels) + x + y * surface->w) = coul;
}

void setPixelVerif(SDL_Surface *surface, int x, int y, Uint32 coul)
{
    if (x >= 0 && x < surface->w &&
            y >= 0 && y < surface->h)
        setPixel(surface, x, y, coul);
}

void switch_int(Sint16 *x, Sint16 *y)
{
    *x += *y;
    *y = *x-*y;
    *x = *x-*y;
}

void SDL_DrawLine(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 couleur)
{
    int coeff_x = abs(x2-x1);
    int coeff_y = abs(y2-y1);

    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    //cas où la droite est plus horizontale
    if (coeff_x >= coeff_y)
    {
        if (x1 > x2)
        {
            switch_int(&x1, &x2);
            switch_int(&y1, &y2);
        }

        setPixel(surface, x1, y1, couleur);

        int dy = -coeff_x;
        int sens_y = (y1 < y2) ? 1 : -1;

        int y = y1;
        int x;

        for ( x=x1+1; x <=x2; x++ )
        {
            dy += coeff_y;
            if (dy >= 0)
            {
                y+=sens_y;
                dy -= coeff_x;
            }
            setPixel(surface, x, y, couleur);
        }
    } else
    {
        //cas où la droite est plus verticale
        if (y1 > y2)
        {
            switch_int(&x1, &x2);
            switch_int(&y1, &y2);
        }

        setPixel(surface, x1, y1, couleur);

        int dx = -coeff_y;
        int sens_x = (x1 < x2) ? 1 : -1;

        int x = x1;
        int y;

        for ( y=y1+1; y <=y2; y++ )
        {
            dx += coeff_x;
            if (dx >= 0)
            {
                x+=sens_x;
                dx -= coeff_y;
            }
            setPixel(surface, x, y, couleur);
        }
    }
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}

void SDL_DrawCircle(SDL_Surface *aff, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul)
{
    if (SDL_MUSTLOCK(aff))
        SDL_LockSurface(aff);
    int d, y, x;

    d = 3 - (2 * rayon);
    x = 0;
    y = rayon;

    while (y >= x) {
        setPixelVerif(aff, cx + x, cy + y, coul);
        setPixelVerif(aff, cx + y, cy + x, coul);
        setPixelVerif(aff, cx - x, cy + y, coul);
        setPixelVerif(aff, cx - y, cy + x, coul);
        setPixelVerif(aff, cx + x, cy - y, coul);
        setPixelVerif(aff, cx + y, cy - x, coul);
        setPixelVerif(aff, cx - x, cy - y, coul);
        setPixelVerif(aff, cx - y, cy - x, coul);

        if (d < 0)
            d = d + (4 * x) + 6;
        else {
            d = d + 4 * (x - y) + 10;
            y--;
        }

        x++;
    }
    if (SDL_MUSTLOCK(aff))
        SDL_UnlockSurface(aff);
}

void SDL_DrawCircleQuarter(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul, bool topleft, bool topright, bool bottomleft, bool bottomright)
{
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    int d, y, x;

    d = 3 - (2 * rayon);
    x = 0;
    y = rayon;

    while (y >= x)
    {
        if (topright)
        {
            setPixel(surface, cx + x, cy + y, coul);
            setPixel(surface, cx + y, cy + x, coul);
        }
        if (topleft)
        {
            setPixel(surface, cx - x, cy + y, coul);
            setPixel(surface, cx - y, cy + x, coul);
        }
        if (bottomright)
        {
            setPixel(surface, cx + x, cy - y, coul);
            setPixel(surface, cx + y, cy - x, coul);
        }
        if (bottomleft)
        {
            setPixel(surface, cx - x, cy - y, coul);
            setPixel(surface, cx - y, cy - x, coul);
        }

        if (d < 0)
            d = d + (4 * x) + 6;
        else {
            d = d + 4 * (x - y) + 10;
            y--;
        }

        x++;
    }
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}

void SDL_DrawFilledCircleQuarter(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul, Uint32 fillcoul, bool topleft, bool topright, bool bottomleft, bool bottomright)
{
    int d, y, x;

    d = 3 - (2 * rayon);
    x = 0;
    y = rayon;

    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

    if (topleft | topright)
        setPixel(surface, cx, cy - rayon +1, fillcoul);
    if (bottomleft | bottomright)
        setPixel(surface, cx, cy + rayon -1, fillcoul);

    while (y >= x)
    {
        if (bottomright)
        {
            setPixelVerif(surface, cx + x, cy + y, coul);
            setPixelVerif(surface, cx + y, cy + x, coul);
            for (int y2 = y-1; y2 >= 0; y2--)
            {
                setPixel(surface, cx + y2, cy + x, fillcoul);
                //if (x != 0)
                    setPixel(surface, cx + x, cy + y2, fillcoul);
            }
        }
        if (bottomleft)
        {
            setPixelVerif(surface, cx - x, cy + y, coul);
            setPixelVerif(surface, cx - y, cy + x, coul);
            for (int y2 = y-1; y2 >= 0; y2--)
            {
                setPixel(surface, cx - y2, cy + x, fillcoul);
                if (x != 0)
                    setPixel(surface, cx - x, cy + y2, fillcoul);
            }
        }
        if (topright)
        {

            setPixelVerif(surface, cx + x, cy - y, coul);
            setPixelVerif(surface, cx + y, cy - x, coul);
            for (int y2 = y-1; y2 >= 0; y2--)
            {
                setPixel(surface, cx + y2, cy - x, fillcoul);
                if (x != 0)
                    setPixel(surface, cx + x, cy - y2, fillcoul);
            }
        }
        if (topleft)
        {
            setPixelVerif(surface, cx - x, cy - y, coul);
            setPixelVerif(surface, cx - y, cy - x, coul);
            for (int y2 = y-1; y2 >= 0; y2--)
            {
                setPixel(surface, cx - y2, cy - x, fillcoul);
                if (x != 0)
                    setPixel(surface, cx - x, cy - y2, fillcoul);
            }
        }

        if (d < 0)
            d = d + (4 * x) + 6;
        else {
            d = d + 4 * (x - y) + 10;
            y--;
        }

        x++;
    }
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}

void SDL_DrawFilledCircle(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul, Uint32 fillcoul)
{
    int d, y, x;

    d = 3 - (2 * rayon);
    x = 0;
    y = rayon;
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    setPixel(surface, cx, cy - rayon +1, fillcoul);
    setPixel(surface, cx, cy + rayon -1, fillcoul);

    while (y >= x)
    {
        setPixelVerif(surface, cx + x, cy + y, coul);
        setPixelVerif(surface, cx + y, cy + x, coul);
        for (int y2 = y-1; y2 >= 0; y2--)
        {
            setPixel(surface, cx + y2, cy + x, fillcoul);
            if (x != 0)
                setPixel(surface, cx + x, cy + y2, fillcoul);
        }

        setPixelVerif(surface, cx - x, cy + y, coul);
        setPixelVerif(surface, cx - y, cy + x, coul);
        for (int y2 = y-1; y2 >= 0; y2--)
        {
            setPixel(surface, cx - y2, cy + x, fillcoul);
            if (x != 0)
                setPixel(surface, cx - x, cy + y2, fillcoul);
        }

        setPixelVerif(surface, cx + x, cy - y, coul);
        setPixelVerif(surface, cx + y, cy - x, coul);
        for (int y2 = y-1; y2 >= 0; y2--)
        {
            setPixel(surface, cx + y2, cy - x, fillcoul);
            if (x != 0)
                setPixel(surface, cx + x, cy - y2, fillcoul);
        }

        setPixelVerif(surface, cx - x, cy - y, coul);
        setPixelVerif(surface, cx - y, cy - x, coul);
        for (int y2 = y-1; y2 >= 0; y2--)
        {
            setPixel(surface, cx - y2, cy - x, fillcoul);
            if (x != 0)
                setPixel(surface, cx - x, cy - y2, fillcoul);
        }

        if (d < 0)
            d = d + (4 * x) + 6;
        else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
}

int SDL_DrawFilledRect(SDL_Surface *surface, SDL_Rect *dest, Uint32 bordercolor, Uint32 bgcolor, Uint8 bordersize)
{
    if (surface == NULL)
    {
        SDL_SetError("Surface nulle passée en argument à SDL_DrawRect");
        return -1;
    }

    {
        SDL_Rect working_rect, current_rect;
        int statut;

        if (dest == NULL)
        {
            //si dest est NULL, on fait toute la taille de la surface
            working_rect.x = 0, working_rect.y = 0, working_rect.w = surface->w, working_rect.h = surface->h;
        } else
        {
            working_rect = *dest;
        }

        //maintenant on fait les quatres bords du rectangles.
        ///BORD haut
        current_rect.x = working_rect.x, current_rect.w = working_rect.w, current_rect.y  = working_rect.y, current_rect.h = bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        ///BORD bas
        current_rect.y += working_rect.h - bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        //Revient à tester s'il s'agit d'un rectangle normal
        if (working_rect.h <= 2 * bordersize) return 0;

        ///BORD gauche
        current_rect.h = working_rect.h - 2 * bordersize, current_rect.w = bordersize, current_rect.y  = working_rect.y + bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        ///BORD droit
        current_rect.x += working_rect.w - bordersize;
        statut = SDL_FillRect(surface, &current_rect, bordercolor);
        if (statut < 0) return statut;

        ///FOND
        working_rect.x += bordersize, working_rect.y += bordersize, working_rect.w -= 2*bordersize, working_rect.h -= 2*bordersize;
        return SDL_FillRect(surface, &working_rect, bgcolor);
    }
}

int SDL_IsInRect(SDL_Rect *r, Sint16 x, Sint16 y)
{
    return (x >= r->x && y >= r->y && x < r->x + r->w && y < r->y + r->h);
}

#define RGB_FROM_PIXEL(Pixel, fmt, r, g, b)				\
{									\
	r = (((Pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss); 		\
	g = (((Pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss); 		\
	b = (((Pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss); 		\
}

//Zoom sans alpha (ni dans le départ, ni dans la fin)
//donc little aliasing
//quand il n'y a pas que des colorkeys, les colorkeys sont pas comptés
/*
  Fonctionnement:
  Une fonction de zoom basse qualité parcoure la surface destination,
  et à chaque pixel associe le pixel correspondant de la surface de départ.
  ----
  Cette fonction-ci parcourt aussi la surface destination. Mais elle fait
  la moyenne pondérée des pixels de la surface de départ correspondants. Ainsi
  le zoom est plus soft.
  A noter que la qualité est meilleure (enfin selon mon impression) avec des
  facteurs entiers (zoom *2, ...)
*/

/*
   Voilà la fonctoin non optimisée, donc elle est commentée
   La fonction optimisée prend en moyenne 7% moins de temps,
   mais elle est beaucoup plus simple à comprendre
*/

//SDL_Surface *SDL_QualityZoom32Surface(SDL_Surface *source, SDL_Rect *src_rect, SDL_Rect *final_dims)
//{
//	SDL_Rect r_source;
//	if (src_rect == NULL) r_source.x = 0, r_source.y = 0, r_source.w = source->w, r_source.h = source->h;
//	else r_source = *src_rect;
//
//	bool colorkey_on = source->flags & SDL_SRCCOLORKEY;
//	Uint32 colorkey = source->format->colorkey;
//
//	double rx_zoom = (double)r_source.w/final_dims->w;
//	double ry_zoom = (double)r_source.h/final_dims->h;
//
//	SDL_Surface *final = SDL_CreateRGBSurface(source->flags, final_dims->w, final_dims->h, 32, 0, 0, 0, 0);
//	if (final == NULL) return final;
//
//	if (colorkey_on)
//		SDL_SetColorKey(final, SDL_SRCCOLORKEY | (final->flags & SDL_RLEACCEL), colorkey);
//
//    Uint32* dst_pixel = (Uint32*)final->pixels;
//    Uint32* src_pixel = (Uint32*)source->pixels;
//	for (int y = 0; y < final->h; y++)
//		for (int x = 0; x < final->w; x++, ++dst_pixel)
//		{
//		    double r_d(0), g_d(0), b_d(0), coeff_sum(0);
//
//            bool only_colorkey(true);
//
//		    Uint16 y_pos = (Uint16) (ry_zoom * y);
//		    Uint16 x_pos = (Uint16) (rx_zoom * x);
//
//		    double y_cond = ry_zoom;
//		    double y_count = std::min((double)((int)(ry_zoom * y + 1) - y_pos), y_cond);
//
//            Uint32 *temp_pixel = src_pixel + y_pos * source->w + x_pos;
//
//		    for(int y_src = 0; y_cond > 0.0001 && y_src + y_pos < source->h; y_src++, temp_pixel += source->w)
//		    {
//		        double x_cond = rx_zoom;
//                double x_count = std::min((double)((int)(rx_zoom * x + 1) - x_pos), x_cond);
//
//                Uint32 *temp_pixel2 = temp_pixel;
//
//                for(int x_src = 0; x_cond > 0.0001 && x_src + x_pos < source->w; x_src++, ++temp_pixel2)
//                {
//                    Uint32 pixel = *temp_pixel2;
//                    if (pixel != colorkey) only_colorkey = false;
//
//                    Uint8 r, g, b;
//                    RGB_FROM_PIXEL(pixel, source->format, r, g, b);
//
//                    double coeff = x_count * y_count;
//
//                    coeff_sum+=coeff;
//                    r_d += r*coeff;
//                    g_d += g*coeff;
//                    b_d += b*coeff;
//
//                    x_cond -= x_count;
//                    x_count = std::min(x_cond, 1.0);
//                }
//                y_cond -= y_count;
//                y_count = std::min(y_cond, 1.0);
//		    }
//		    if (only_colorkey)
//		    {
//		        *dst_pixel = colorkey;
//		        continue;
//		    }
//
//		    r_d/= coeff_sum;
//		    g_d/= coeff_sum;
//		    b_d/= coeff_sum;
//
//			*dst_pixel = SDL_MapRGB(final->format, (Uint8)r_d, (Uint8) g_d, (Uint8) b_d);
//		}
//
//	return final;
//}
//
///*
//   Voilà la fonction optimisée,
//   Elle prend en moyenne 7% de moins de temps que l'autre.
//   Mais pour ça, j'ai du rajouté plein de multiplication pour
//   éviter les doubles.
//
//   Si jamais il y a un problème avec cette fonction (c'est possible),
//   ou que l'autre est plus jolie, n'hésitez pas à changer.
//   Si c'est juste qu'avec des trops grandes surfaces et des zooms trop important
//   ca bugge, alors essayez de changer quelques Uint16 en Uint32.
//*/
//
//SDL_Surface *SDL_QualityZoom32Surface(SDL_Surface *source, SDL_Rect *src_rect, SDL_Rect *final_dims)
//{
//	SDL_Rect r_source;
//	if (src_rect == NULL) r_source.x = 0, r_source.y = 0, r_source.w = source->w, r_source.h = source->h;
//	else r_source = *src_rect;
//
//	bool colorkey_on = source->flags & SDL_SRCCOLORKEY;
//	Uint32 colorkey = source->format->colorkey;
//
//	SDL_Surface *final = SDL_CreateRGBSurface(source->flags, final_dims->w, final_dims->h, 32, 0, 0, 0, 0);
//	if (final == NULL) return final;
//
//	if (colorkey_on)
//		SDL_SetColorKey(final, SDL_SRCCOLORKEY | (final->flags & SDL_RLEACCEL), colorkey);
//
//    Uint32* dst_pixel = (Uint32*)final->pixels;
//    Uint32* src_pixel = (Uint32*)source->pixels;
//	for (int y = 0; y < final->h; y++)
//		for (int x = 0; x < final->w; x++, ++dst_pixel)
//		{
//		    Uint32 r_d(0), g_d(0), b_d(0), coeff_sum(0);
//
//            bool only_colorkey(true);
//
//		    Uint32 y_pos = source->h * y;
//		    Uint32 x_pos = source->w * x;
//
//		    Sint16 y_cond = source->h;
//		    Sint16 y_count = std::min(final->h - (y_pos%final->h), (Uint32)y_cond);
//
//            Uint32 *temp_pixel = src_pixel + y_pos * source->w /final->h + x_pos/final->w;
//
//		    for(Uint16 y_src = 0; y_cond > 0 && y_src * final->h + y_pos < (unsigned)source->h*final->h; y_src++, temp_pixel += source->w)
//		    {
//		        Sint16 x_cond = source->w;
//                Sint16 x_count = std::min(final->w - (x_pos%final->w), (Uint32)x_cond);
//
//                Uint32 *temp_pixel2 = temp_pixel;
//
//                for(Uint16 x_src = 0; x_cond > 0 && x_src * final->w + x_pos < (unsigned)source->w*final->w; x_src++, ++temp_pixel2)
//                {
//                    Uint32 pixel = *temp_pixel2;
//                    if (pixel != colorkey) only_colorkey = false;
//
//                    Uint8 r, g, b;
//                    RGB_FROM_PIXEL(pixel, source->format, r, g, b);
//
//                    Uint32 coeff = x_count * y_count;
//
//                    coeff_sum+=coeff;
//                    r_d += r*coeff;
//                    g_d += g*coeff;
//                    b_d += b*coeff;
//
//                    x_cond -= x_count;
//                    x_count = std::min(x_cond, (Sint16)final->w);
//                }
//                y_cond -= y_count;
//                y_count = std::min(y_cond, (Sint16)final->h);
//		    }
//		    if (only_colorkey)
//		    {
//		        *dst_pixel = colorkey;
//		        continue;
//		    }
//
//		    r_d/= coeff_sum;
//		    g_d/= coeff_sum;
//		    b_d/= coeff_sum;
//
//			*dst_pixel = SDL_MapRGB(final->format, r_d, g_d, b_d);
//		}
//
//	return final;
//}
