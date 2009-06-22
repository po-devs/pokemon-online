#ifndef CREDITS_HH_INCLUDED
#define CREDITS_HH_INCLUDED

#include "MF.hh"

class Credits : public MF_Boss, public MF_Prio
{
    public:
        Credits();

        bool recoitMessage(const char *message, MF_Base *fenetre);
        void affiche(Surface &ecran);
};

#endif // CREDITS_HH_INCLUDED
