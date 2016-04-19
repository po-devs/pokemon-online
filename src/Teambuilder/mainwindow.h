#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "engineinterface.h"
#include "centralwidget.h"
#include <PokemonInfo/teamholder.h>
#include "downloadmanager.h"
#include <Utilities/otherwidgets.h>

class QScrollDownTextBrowser;
class ClientPluginManager;
class MainWidget;

/* The main window!

   All the general initialization is done here, the persistent data (i.e the team) too.
   The main window manages the sub-modules. */

class MainEngine : public QObject, public MainEngineInterface {
    Q_OBJECT
public:
    MainEngine(bool updated);
    ~MainEngine();

    int numberOfTabs() const;

    void addThemeMenu(QMenuBar *m);
    void addStyleMenu(QMenuBar *m);
    void addLanguageMenu(QMenuBar *m);
    void changeTheme(const QString &theme);
public slots:
    void launchMenu();
    void launchAbout();
    void openWebsite();
    void openForum();
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
    void openThemesForum();
    void changeStyle();
    void showReplay(QString);
    void closeTab();

    /* Shows a notification with the tray icon */
    void showMessage(const QString &title, const QString &msg);
    void raiseLastNotificationSender();
    void exitWarningChanged(bool warn);
private slots:
    /* Relies on ((QAction*)(sender()))->text() */
    void openPluginConfiguration();
    void changeTheme();
    void changeUserThemeFolder();
    void reloadThemes();

    void updateDataReady(const QString &data, bool error);
    void changeLogReady(const QString &data, bool error);

    void updateRunningTime();

    void pmNotificationsChanged(bool notify);
private:
    void rebuildThemeMenu();

    QMainWindowPO *displayer;
    ClientPluginManager *pluginManager;

    QMenuBar* transformMenuBar(QMenuBar *param);
    QMenu* themeMenu;
    MainWidget *main;

    bool pmNotify;
    QSystemTrayIcon *trayIcon;
    QPointer<QWidget> lastNotificationSender;

    DownloadManager downloader;

    QHash<int, TeamHolder *> m_teams;

    int freespot;

    void routine(CentralWidgetInterface *w);

    TeamHolder *trainerTeam(int spot);
    int currentSpot() const;
    void addTeam(int spot);

    QVector <TeamHolder*> trash;

    void clearTrash();

    //***************//
    //* Update Data *//
    //***************//
    QString updateData;
    QString changeLog;

public:
    TeamHolder *trainerTeam();
    ThemeAccessor *theme();

    static MainEngine *inst;
};

#endif // MAINWINDOW_H
