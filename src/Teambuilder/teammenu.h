#ifndef TEAMMENU_H
#define TEAMMENU_H

#include "teambuilderwidget.h"
#include <QHash>

class QStackedWidget;
class QTabBar;
class TeamHolder;

class TeamMenu : public TeamBuilderWidget
{
    Q_OBJECT

public:
    explicit TeamMenu(TeamHolder *team, int index=0);
    ~TeamMenu();
signals:
    void switchToTrainer();
private slots:
    void switchToTab(int index);
private:
    void setupUi();

    struct _ui {
        QStackedWidget *stack;
        QTabBar *pokemonTabs;
        QHash<int, QWidget*> pokemons;
    };

    _ui *ui;

    TeamHolder *m_team;
    TeamHolder &team() { return *m_team;}
    const TeamHolder &team() const { return *m_team;}
};

#endif // TEAMMENU_H
