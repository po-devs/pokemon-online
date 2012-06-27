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
};

#endif // POKEBOXES_H
