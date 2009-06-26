#ifndef BIG_BOSS_H_INCLUDED
#define BIG_BOSS_H_INCLUDED

#include "MF.hh"
#include "Team_Struct.h"

/* La seule classe toujours présente!

   Il peut y avoir moyen de faire un union ou je ne sais quoi, mais pourquoi se compliquer la vie? */

class BigBoss : public MF_Boss
{
    public:
        Team equipe;

        BigBoss();

        bool recoitMessage(const char *message, MF_Base *fenetre);
        void affiche(Window &ecran);
};

#endif // BIG_BOSS_H_INCLUDED
