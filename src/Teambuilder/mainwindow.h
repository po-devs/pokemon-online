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

    void addThemeMenu(QMenuBar *m);
    void addStyleMenu(QMenuBar *m);
    void changeTheme(const QString &theme);
public slots:
    void launchMenu(bool first = false);
    void launchCredits();
    void launchTeamBuilder();
    void reloadPokemonDatabase();
    void goOnline(const QString &url, const quint16 port, const QString &name);
    void launchServerChoice();
    void changeLanguage();
    void updateMenuBar();
    void openPluginManager();
    void quit();

    /* slots called by subwindows when they need it */
    void loadTeamDialog();
    void loadReplayDialog();
    void loadStyleSheet();
    void changeStyle();
    void showReplay(QString);
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

    TeamHolder *m_team;

    void routine(CentralWidgetInterface *w);
public:
    TeamHolder *trainerTeam() {
        return m_team;
    }
    ThemeAccessor *theme();
};

#endif // MAINWINDOW_H
