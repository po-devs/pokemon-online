#ifndef MENU_H
#define MENU_H

#include <QFrame>
#include "centralwidget.h"

namespace Ui {
class Menu;
}

class MainEngine;
class TeamHolder;

class Menu : public QFrame, public CentralWidgetInterface
{
    Q_OBJECT
    
public:
    explicit Menu(TeamHolder *t, QWidget *parent = 0);
    ~Menu();

    /* Creates a menu bar to give to the main window */
    QMenuBar *createMenuBar(MainEngine *w);

signals:
    void goToTeambuilder();
    void goToOnline();
    void goToCredits();
    void goToExit();

public slots:
    void loadTeam();
    void loadAll(const TeamHolder&);

protected:
    bool event(QEvent *e);
private:
    Ui::Menu *ui;

    TeamHolder *team;
};

#endif // MENU_H
