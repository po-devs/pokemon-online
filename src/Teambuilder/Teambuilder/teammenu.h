#ifndef TEAMMENU_H
#define TEAMMENU_H

#include "../PokemonInfo/pokemonstructs.h"
#include "Teambuilder/teambuilderwidget.h"
#include <QHash>

class QStackedWidget;
class QTabBar;
class TeamHolder;
class QStringListModel;
class QAbstractItemModel;
class PokeEdit;

class TeamMenu : public TeamBuilderWidget
{
    Q_OBJECT

public:
    explicit TeamMenu(QMainWindow *window, QAbstractItemModel *pokeModel, TeamHolder *team, int index=0);
    ~TeamMenu();

    void updateTeam();
    void addMenus(QMenuBar *);
signals:
    void switchToTrainer();
public slots:
    void switchToTab(int index);
    void choosePokemon();
    /* Close the advanced tab */
    void toggleAdvanced();
    void closeAdvanced();
private slots:
    void tabIconChanged();
private:
    void setupUi();
    void updateItemModel();
    void updateTabs();

    struct _ui {
        QStackedWidget *stack;
        QTabBar *pokemonTabs;
        QHash<int, PokeEdit*> pokemons;
        QStringListModel *itemsModel, *natureModel;
        QAbstractItemModel *pokemonModel;
    };

    _ui *ui;

    TeamHolder *m_team;
    QPointer<QAction> advancedMenu;
    Pokemon::gen lastGen;
    TeamHolder &team() { return *m_team;}
    const TeamHolder &team() const { return *m_team;}
};

#endif // TEAMMENU_H
