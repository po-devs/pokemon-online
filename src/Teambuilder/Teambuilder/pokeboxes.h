#ifndef POKEBOXES_H
#define POKEBOXES_H

#include <QtGui>
#include "Teambuilder/teambuilderwidget.h"

class PokeTeam;
class TeamHolder;

namespace Ui {
class PokeBoxes;
}

class PokeBoxes :  public TeamBuilderWidget
{
    Q_OBJECT

public:
    explicit PokeBoxes(QWidget *parent = 0, TeamHolder *nteam = NULL);
    ~PokeBoxes();
    void changePoke(PokeTeam *poke);
    void updatePoke();

public slots:
    void changeTeamPoke(int index);

private:
    PokeTeam *m_poke;
    TeamHolder *m_team;
    Ui::PokeBoxes *ui;
    const PokeTeam &poke() const {return *m_poke;}
    const TeamHolder &team() const {return *m_team;}
    PokeTeam &poke() {return *m_poke;}
    TeamHolder &team() {return *m_team;}
};

#endif // POKEBOXES_H
