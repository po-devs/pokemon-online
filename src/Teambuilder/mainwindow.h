#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "../PokemonInfo/pokemoninfo.h"

class TB_Menu;
class TeamBuilder;
class Client;
class ServerChoice;

/* The main window!

   All the general initialization is done here, the persistent data (i.e the team) too.
   The main window manages the sub-modules. */

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    TrainerTeam m_team;

    TB_Menu *m_menu;
    TeamBuilder *m_TB;
    Client *m_client;
    ServerChoice *m_choice;

    TrainerTeam *trainerTeam();

public:
    MainWindow();

    void loadTeam(const QString &path);
public slots:
    void launchMenu();
    void launchCredits();
    void launchTeamBuilder();
    void goOnline(const QString &url);
    void launchServerChoice();
    void changeStyle();
    void changeLanguage();

    /* slots called by subwindows when they need it */
    void loadTeamDialog();

    void setDock(Qt::DockWidgetArea area,QDockWidget * dock,Qt::Orientation orient);
    void removeDock(QDockWidget * dock);
};

#endif // MAINWINDOW_H
