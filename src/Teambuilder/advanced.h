#ifndef TB_ADVANCED_H
#define TB_ADVANCED_H

#include <QtGui>
#include "teambuilder.h"

/* This file contains the code for the "Advanced" window you
   can access in the Teambuilder on a pokemon */

class TB_Advanced : public QMainWindow
{
    Q_OBJECT
private:
    PokeTeam *m_poke;

    QLabel *pokeImage;
    /* hp means hidden power */
    QComboBox *hpchoice;
    QComboBox *dvchoice[6];
    QLabel *stats[6];
    QLabel *hpower;
    QCompactTable *hpanddvchoice;
    QRadioButton *ability1, *ability2;
    QRadioButton *gender1, *gender2;
    QCheckBox *shiny;
    QComboBox *level;

    PokeTeam *poke();

    void updateStat(int stat);
    void updateStats();
private slots:
    void changeAbility();
    void changeShininess();
public:
    TB_Advanced(PokeTeam *poke);
};

#endif // TB_ADVANCED_H
