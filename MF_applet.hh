#ifndef MF_APPLET_HH
#define MF_APPLET_HH

#include <SDL/SDL_image.h>
#include "interfont.hpp"
#include <sstream>

#include "MF.hh"

//MF_applet.hh
//it's an applet, a surface where you can draw static things

class MF_Applet : virtual public MF_Surf
{
    public:
    //Besoin d'arguments pour faire le fond, pas de resize qui tienne!
    MF_Applet(Uint16 w, Uint16 h, const Color &rgb, Sint16 x = 0, Sint16 y = 0);
    ~MF_Applet(){}

    //Nouvelle fonction pour afficher
    virtual void affiche(Surface &surface);
    //pour imprimer du texte
    virtual void drawString(Font &police, const char *texte, Sint16 x, Sint16 y, const Color &rgb = Color());
    virtual void drawStyledString(Font &police, const char *texte, Sint16 x, Sint16 y, const FontStyle &flags, const Color &rgb = Color());
    //Pour copier une image sur la surface
    virtual void BlitImage(Sint16 x, Sint16 y, const char *filename, bool colorkeyon=false, Color *colorkey = NULL);
    virtual void BlitSurface(Surface &src, const Rect &src, Sint16 x_dest, Sint16 y_dest);
    virtual void FillRect(const Rect &r = 0, const Color &col = Color(0xFF,0xFF, 0xFF));
};

//applet qui s'illumine quand y'a la souris dessus
class MF_HLApplet : public MF_Applet
{
    public:
    //couleur pour le hover
    Color HLColor;

    //on ajoute la couleur du highlight
    MF_HLApplet(Uint16 w, Uint16 h, const Color &rgb, Sint16 x = 0, Sint16 y = 0, const Color &hl = Color(0x22, 0xFF, 0xAA));
    ~MF_HLApplet(){;}

    //une autre fonction pour afficher
    virtual void affiche(Surface &ecran);
    virtual int set_hover_state(int enable);
    //Mettre la couleur de highlight
    Color Set_HLColor(Uint8 r, Uint8 g, Uint8 b){
        HLColor.r = r, HLColor.g = g, HLColor.b = b;
        if (hovered) set_updated();
        return HLColor;
    }
};

//Même chose, sauf qu'on utilise une image seulement
class MF_ImHLApplet : virtual public MF_Base
{
    public:
    //couleur pour le hover
    Color HLColor;
    //comme MF_Surf
    Surface surface;

    MF_ImHLApplet(const char *path, Sint16 x = 0, Sint16 y = 0, const Color &hl = Color(0x22, 0xFF, 0xAA));
    ~MF_ImHLApplet(){}

    virtual int set_hover_state(int enable);
    //Mettre la couleur de highlight
    Color Set_HLColor(const Color &ncol){
        HLColor = ncol;
        if (hovered) set_updated();
        return HLColor;
    }
    virtual void affiche(Surface &surf);
    virtual bool gereEvenement(const SDL_Event &event);
};

//bouton - applet
class MF_BApplet : public MF_HLApplet, virtual public MF_Directions
{
    public:
    bool clicOn;

    MF_BApplet(Uint16 w, Uint16 h, const Color &rgb, Sint16 x = 0, Sint16 y = 0, const Color &hl = Color(0x22, 0xFF, 0xAA));
    ~MF_BApplet(){;}

    //pour les clics
    virtual bool gereEvenement(const SDL_Event &event);
    //encore une nouvelle fonction pour afficher!
    virtual void affiche(Surface &ecran);
};

//truc avec deux images
class MF_HoverMenu : virtual public MF_Base, virtual public MF_Directions
{
    public:
        //0 si c'est l'image de base, 1 si c'est la seconde
        bool image_displayed;
        //les deux images
        Surface image_1, image_2;

        MF_HoverMenu (const char* img_path_1, const char *img_path_2) : image_displayed(0), image_1(img_path_1), image_2(img_path_2){}
        ~MF_HoverMenu();

        virtual bool check_updated() { return image_displayed == hovered; }
        virtual void affiche(Surface &ecran);
        virtual bool gereEvenement(const SDL_Event &event);
};

//Même chose mais en bouton
class MF_Button : virtual public MF_Base, virtual public MF_Directions
{
    public:
        bool clicOn;
        //les deux images
        Surface image_1, image_2;

        MF_Button (const char* img_path_1, const char *img_path_2);
        ~MF_Button();

        virtual void affiche(Surface &ecran);
        virtual bool gereEvenement(const SDL_Event &event);
};
#include "MF_form.hh"

#endif
