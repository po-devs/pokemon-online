#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "engineinterface.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "centralwidget.h"

class TB_Menu;
class TeamBuilder;
class Client;
class ServerChoice;
class PluginManager;

/* The main window!

   All the general initialization is done here, the persistent data (i.e the team) too.
   The main window manages the sub-modules. */

class MainEngine : public QObject, public MainEngineInterface {
    Q_OBJECT
public:
    MainEngine();
    ~MainEngine();

    void loadTeam(const QString &path);

    void addStyleMenu(QMenuBar *m);
    void addThemeMenu(QMenuBar *m);
    void changeTheme(const QString &theme);
public slots:
    void launchMenu();
    void launchCredits();
    void launchTeamBuilder();
    void goOnline(const QString &url, const quint16 port, const QString &name);
    void launchServerChoice();
    void changeLanguage();
    void updateMenuBar();
    void openPluginManager();
    void quit();

    /* slots called by subwindows when they need it */
    void loadTeamDialog();
    void loadStyleSheet();
private slots:
    /* Relies on ((QAction*)(sender()))->text() */
    void openPluginConfiguration();
    void changeStyle();
    void changeTheme();
private:
    QMainWindow *displayer;
    PluginManager *pluginManager;

    QMenuBar* transformMenuBar(QMenuBar *param);

    TrainerTeam m_team;
public:
    TrainerTeam *trainerTeam() {
        return &m_team;
    }
};

#endif // MAINWINDOW_H
