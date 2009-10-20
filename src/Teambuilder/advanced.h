#ifndef TB_ADVANCED_H
#define TB_ADVANCED_H

#include <QtGui>
#include "teambuilder.h"

/* This file contains the code for the "Advanced" window you
   can access in the Teambuilder on a pokemon */

class TB_Advanced : public QWidget
{
    Q_OBJECT
private:
    PokeTeam *m_poke;

    QLabel *pokeImage;
    /* hp means hidden power */
    QComboBox *hpchoice;
    QSpinBox *dvchoice[6];
    QLabel *stats[6];
    QLabel *hpower;
    QCompactTable *hpanddvchoice;
    QRadioButton *ability1, *ability2;
    QRadioButton *gender1, *gender2;
    QCheckBox *shiny;
    QSpinBox *level;

    PokeTeam *poke();

    //Called by updateDVs
    void updateStat(int stat);
    void updateStats();

    void updatePokeImage();
    void updateGender();
    void updateAbility();
    void updateDV(int stats);
    void updateDVs();
    void updateHiddenPower();
    void changeDV(int stat, int newval);
    // Gives the num of the stat corresponding to that ptr
    int stat(QObject *dvchoiceptr);
private slots:
    void changeAbility(bool);
    void changeShininess(bool);
    void changeGender(bool);
    void changeLevel(int);
    /* Do not use directly */
    void changeDV(int newval);
public:
    TB_Advanced(PokeTeam *poke);
};

#endif // TB_ADVANCED_H
