#ifndef TEAMBUILDER_H
#define TEAMBUILDER_H

#include "../PokemonInfo/pokemon.h"
#include "../PokemonInfo/geninfo.h"
#include <QWidget>
#include "centralwidget.h"
#include "Teambuilder/teambuilderwidget.h"

class TeamHolder;
class TrainerMenu;
class TeamMenu;
class PokeBoxes;

class TeamBuilder : public QMainWindow, public CentralWidgetInterface
{
    Q_OBJECT
public:
    TeamBuilder(TeamHolder *team, bool loadSettings=true);
    ~TeamBuilder();

    virtual QSize defaultSize() const;
    virtual QMenuBar *createMenuBar(MainEngine *);

    TeamBuilderWidget *currentWidget();
    TeamBuilderWidget *widget(int i);
public slots:
    void saveAll();
    void openLoadWindow();
    void loadAll(const TeamHolder &t);
    void newTeam();
    void openBoxes();
    void editPoke(int);
    void switchToTrainer();
    void setTierList(const QStringList &tiers); //tells which tiers are available

    void setNoMod();
    void changeMod();
    void genChanged();
    void installMod();

    void importTeam();
    void exportTeam();
    void addTeam();
    void openTeam();
    void saveTeam();
private slots:
    void markTeamUpdated();
    void updateCurrentTeamAndNotify();
    void onSaveTeam();
signals:
    void done();
    void reloadMenuBar();
    void reloadDb();
private:
    TeamHolder &team() {return *m_team;}
    TeamHolder *m_team;
    TrainerMenu *trainer;
    TeamMenu *teamMenu;
    PokeBoxes *boxesMenu;

    QAbstractItemModel *pokemonModel;

    struct _ui {
        QHash<Pokemon::gen, QAction*> gens;
        QStackedWidget *stack;
    };

    Pokemon::gen lastGen;

    _ui * ui;

    void markAllUpdated();
    void switchTo(TeamBuilderWidget *w);
};

#endif // TEAMBUILDER_H
