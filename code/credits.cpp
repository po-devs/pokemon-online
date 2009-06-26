#include "credits.h"
#include "MF_applet.hh"
#include "MF_text.hh"
#include "utilities.hh"
#include "SDL_gfx.h"
#include <fstream>

using namespace std;

Credits::Credits()
{
    ifstream f("db/credits.txt", ios::binary);
    //Le nombre de personnes différentes
    int MagicNumber;

    f >> MagicNumber;

    //On cherche le nom correspondant
    smart_ptr<string> nom = find_line(f, (rand() % MagicNumber) + 2);

    /** TITRE **/
    MF_Ligne *title = new MF_Ligne();
    allouer(title);

    title->setColor(0xFF);
    title->setTextColor(Color(0xFF, 0X44, 0x66));

    title->setFont("verdana.ttf", 40);

    int w, h;
    title->police.text_size(nom->c_str(), &w, &h);

    SDL_Surface *ecran = SDL_GetVideoSurface();

    title->setRect((ecran->w-w)/2, 40, w, h);
    title->ecrire(*nom);

    /** DESCRIPTION **/
    MF_MLigne *desc = new MF_MLigne();
    allouer(desc);

    desc->setColor(0xFF);
    desc->setTextColor(0);
    desc->setFont("verdana.ttf", 16);
    desc->setRect(80, 150, ecran->w - 160, 300);
    desc->setcapacity(11);

    smart_ptr< fast_array<char> > it = get_file_content(("db/" + *nom + ".txt").c_str(), true);
    desc->ecrire(*it);

    /** BOUTON "BACK" **/
    allouer (new MF_ImHLApplet("back.png", 250, 460));
}

bool Credits::recoitMessage(const char *message, MF_Base *fenetre)
{
    if (strcmp(message, "clic") == 0)
    {
        envoieMessage("menu");
        return true;
    }
    return false;
}

void Credits::affiche(Surface &s)
{
    updated = true;
    s.fill(0, Color(0xFF, 0xFF, 0xFF));
    afficheMF(s);
}
