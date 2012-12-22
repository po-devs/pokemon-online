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
    void changePoke(PokeTeam *poke, int slot = -1, int box=-1);
    void updatePoke();

    void updateTeam();
//signals:
//    void done();
public slots:
    void changeTeamPoke(int index);
    void currentBoxChanged(int b);

    void showPoke(PokeTeam *poke);
    void switchBoxTeam(int box, int boxslot, int poke);

    void storePokemon();
    void switchPokemon();
    void deletePokemon();
    void withdrawPokemon();

    void newBox();
    void editBoxName();
    void deleteBox(int num=-1);
    /* deals with dropping something on a pokemon box button */
    void dealWithButtonDrop(int index, QDropEvent *event);
private:
    PokeTeam *m_poke;
    int displayedBox, displayedSlot;
    TeamHolder *m_team;
    Ui::PokeBoxes *ui;
    QList<PokeBox*> boxes;
    const PokeTeam &poke() const {return *m_poke;}
    const TeamHolder &team() const {return *m_team;}
    PokeTeam &poke() {return *m_poke;}
    TeamHolder &team() {return *m_team;}

    void loadBoxes();
    void addBox(const QString &name);

    bool existBox(const QString &name) const;
    void doDeleteBox(int num);

    PokeBox *currentBox();

    void updateSpot(int i);
    int currentPoke() const;

    void setCurrentTeamPoke(PokeTeam *p);
    PokeTeam *currentPokeTeam();
};

#endif // POKEBOXES_H
