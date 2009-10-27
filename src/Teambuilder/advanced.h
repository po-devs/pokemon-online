#ifndef TB_ADVANCED_H
#define TB_ADVANCED_H

#include <QtGui>

/* This file contains the code for the "Advanced" window you
   can access in the Teambuilder on a pokemon */

class PokeTeam;
class QCompactTable;

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
    const PokeTeam *poke() const;

    //Called by updateDVs
    void updateStat(int stat);
    void updateStats();

    void updatePokeImage();
    void updateGender();
    void updateAbility();
    int currentHiddenPower() const;
    int calculateHiddenPowerPower() const;
    int calculateHiddenPowerType() const;
    void updateDV(int stat);
    void updateDVs();
    void updateHiddenPower();
    void updateHpAndDvChoice();
    void changeDV(int stat, int newval);
    // Gives the num of the stat corresponding to that ptr
    int stat(QObject *dvchoiceptr);
private slots:
    void changeAbility(bool);
    void changeShininess(bool);
    void changeGender(bool);
    void changeDVsAccordingToHP(int row);
    void changeLevel(int);
    void changeHiddenPower(int);
    /* Do not use directly */
    void changeDV(int newval);
public:
    TB_Advanced(PokeTeam *poke);
};

#endif // TB_ADVANCED_H
