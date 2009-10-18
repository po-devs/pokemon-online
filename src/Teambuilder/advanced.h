#ifndef TB_ADVANCED_H
#define TB_ADVANCED_H

#include <QtGui>
#include "teambuilder.h"

class TB_Advanced : public QMainWindow
{
    Q_OBJECT
private:
    PokeTeam *m_poke;

    QLabel *pokeImage;
    /* hp means hidden power */
    QComboBox *hpchoice;
    QComboBox *dvchoice[6];
    QCompactTable *hpanddvchoice;
    QRadioButton *ability1, *ability2;
    QRadioButton *shiny, *notshiny;

    PokeTeam *poke();
private slots:
    //void changeAbility();
    //void changeShininess();
public:
    TB_Advanced(PokeTeam *poke);
};

#endif // TB_ADVANCED_H
