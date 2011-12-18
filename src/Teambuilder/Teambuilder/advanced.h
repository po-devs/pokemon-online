#ifndef TB_ADVANCED_H
#define TB_ADVANCED_H

#include <QtGui>
#include "../PokemonInfo/pokemonstructs.h"

class AvatarBox;

/* This file contains the code for the "Advanced" window you
   can access in the Teambuilder on a pokemon */

class PokeTeam;
class QCompactTable;

class TB_Advanced : public QWidget
{
    Q_OBJECT
public:
    TB_Advanced(PokeTeam *poke);
public slots:
    //called by update DV and when the stat in the teambuilder is modified.
    void updateStats();
    void updateStat(int stat);
signals:
    void imageChanged();
    void genderChanged();
    void levelChanged();
    void statChanged();
    void abilityChanged();
    void pokeFormeChanged(Pokemon::uniqueId newform);
private slots:
    void changeAbility(bool);
    void changeShininess(bool);
    void changeShininess2(bool);
    void changeGender(bool);
    void updatePokeImage();
    void changeDVsAccordingToHP(int row);
    void changeLevel(int);
    void changeHiddenPower(int);
    void changeHappiness(int);
    /* Do not use directly */
    void changeDV(int newval);
    void changeForme();
private:
    PokeTeam *m_poke;

    QLabel *pokeImage;
    /* hp means hidden power */
    QComboBox *hpchoice;
    QSpinBox *dvchoice[6];
    QLabel *stats[6];
    QLabel *hpower;
    QCompactTable *hpanddvchoice;
    QRadioButton *ability[3];
    QRadioButton *gender1, *gender2;
    QCheckBox *shiny;
    QSpinBox *level, *happiness;


    PokeTeam *poke();
    const PokeTeam *poke() const;

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
    int gen() const;
};

#endif // TB_ADVANCED_H
