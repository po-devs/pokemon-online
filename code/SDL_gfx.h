#ifndef SDL_GFX_HH
#define SDL_GFX_HH

//SDL_gfx.h
//Pour les manips de dessin

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>

//Largeur d'un texte
Uint16 TTF_TextWidth(TTF_Font *police, const char *texte);
//Pour creer une SDL_Color
SDL_Color SDL_CreateColor(Uint8 r, Uint8 g, Uint8 b);
//Pour un SDL_Rect
SDL_Rect SDL_CreateRect(Sint16 x, Sint16 y, Uint16 w, Uint16 h);
//pour faire un map d'une couleur
Uint32 SDL_MapColor(SDL_PixelFormat *fmt, const SDL_Color &color);
//pour charger une image avec du colorkey
SDL_Surface* IMG_LoadColorkey(const char *path, SDL_Color colorkey);
//Pour dessiner un rectangle avec un contour, et une certaine taille de bordure
int SDL_DrawRect(SDL_Surface *surface, SDL_Rect *dest, Uint32 bordercolor, Uint8 bordersize);
//même chose, mais avec un fond
int SDL_DrawFilledRect(SDL_Surface *surface, SDL_Rect *dest, Uint32 bordercolor, Uint32 bgcolor, Uint8 bordersize);
//pour tester si certaines coordonnées sont dans un rectangle
int SDL_IsInRect(SDL_Rect *r, Sint16 x, Sint16 y);
//cercle
int SDL_IsInCircle(Sint16 c_x, Sint16 c_y, Uint16 rayon, Sint16 x, Sint16 y);
//tracer une ligne
void SDL_DrawLine(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 couleur);
//et un cercle
void SDL_DrawCircle(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul);
//un cercle rempli
void SDL_DrawFilledCircle(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul, Uint32 fillcoul);
//et des quarts de cercle
void SDL_DrawCircleQuarter(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul, bool topleft, bool topright, bool bottomleft, bool bottomright);
//et remplis!
void SDL_DrawFilledCircleQuarter(SDL_Surface *surface, Sint16 cx, Sint16 cy, Uint16 rayon, Uint32 coul, Uint32 fillcoul, bool topleft, bool topright, bool bottomleft, bool bottomright);
//un zoom lent mais normalement de qualité
//SDL_Surface *SDL_QualityZoom32Surface(SDL_Surface *source, SDL_Rect *src_rect, SDL_Rect *final_dims);
//zoom beaucoup plus rapide avec même qualité :( :( :(
//SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth);

#endif
