#ifndef POKEBOXES_H
#define POKEBOXES_H

#include <QtGui>
#include "Teambuilder/teambuilderwidget.h"

class PokeTeam;
class TeamHolder;
class PokeBoxItem;
class PokeBox;

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

    void updateTeam();
public slots:
    void changeTeamPoke(int index);

    void showPoke(PokeTeam *poke);
    void switchBoxTeam(int,int,int);

    void storePokemon();
    void switchPokemon();
    void deletePokemon();
    void withdrawPokemon();
private:
    PokeTeam *m_poke;
    TeamHolder *m_team;
    Ui::PokeBoxes *ui;
    QList<PokeBox*> boxes;
    const PokeTeam &poke() const {return *m_poke;}
    const TeamHolder &team() const {return *m_team;}
    PokeTeam &poke() {return *m_poke;}
    TeamHolder &team() {return *m_team;}

    void loadBoxes();
    void addBox(const QString &name);

    PokeBox *currentBox();

    void updateSpot(int i);
    int currentPoke() const;

    void setCurrentTeamPoke(PokeTeam *p);
    PokeTeam *currentPokeTeam();
};

#endif // POKEBOXES_H
