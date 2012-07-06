#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "engineinterface.h"
#include "centralwidget.h"
#include "Teambuilder/teamholder.h"

class PluginManager;
class MainWidget;

/* The main window!

   All the general initialization is done here, the persistent data (i.e the team) too.
   The main window manages the sub-modules. */

class MainEngine : public QObject, public MainEngineInterface {
    Q_OBJECT
public:
    MainEngine();
    ~MainEngine();

    int numberOfTabs() const;

    void addThemeMenu(QMenuBar *m);
    void addStyleMenu(QMenuBar *m);
    void changeTheme(const QString &theme);
public slots:
    void launchMenu(bool first = false);
    void launchCredits();
    void launchTeamBuilder();
    void reloadPokemonDatabase();
    void goOnline(const QString &url, const quint16 port, const QString &name);
    void launchServerChoice(bool newTab = false);
    void changeLanguage();
    void updateMenuBar();
    void openPluginManager();
    void quit();

    /* slots called by subwindows when they need it */
    void loadTeamDialog();
    void loadReplayDialog();
    void openNewTab();
    void loadStyleSheet();
    void changeStyle();
    void showReplay(QString);
    void closeTab();
private slots:
    /* Relies on ((QAction*)(sender()))->text() */
    void openPluginConfiguration();
    void changeTheme();

    void changeUserThemeFolder();
private:
    void rebuildThemeMenu();

    QMainWindow *displayer;
    PluginManager *pluginManager;

    QMenuBar* transformMenuBar(QMenuBar *param);
    QMenu* themeMenu;
    MainWidget *main;

    QHash<int, TeamHolder *> m_teams;

    int freespot;

    void routine(CentralWidgetInterface *w);

    TeamHolder *trainerTeam(int spot);
    int currentSpot() const;
    void addTeam(int spot);

    QVector <TeamHolder*> trash;

    void clearTrash();
public:
    TeamHolder *trainerTeam();
    ThemeAccessor *theme();
};

#endif // MAINWINDOW_H
