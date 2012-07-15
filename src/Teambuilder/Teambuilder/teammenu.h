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

signals:
    void switchToTrainer();
public slots:
    void switchToTab(int index);
    void choosePokemon();
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
    Pokemon::gen lastGen;
    TeamHolder &team() { return *m_team;}
    const TeamHolder &team() const { return *m_team;}
};

#endif // TEAMMENU_H
