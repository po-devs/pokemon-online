/* 
 * File:   serverwidget.h
 * Author: remco
 *
 * Created on July 9, 2010, 5:45 PM
 */

#ifndef SERVERWIDGET_H
#define	SERVERWIDGET_H

#include <QWidget>

#include "server.h"

class QScrollDownTextBrowser;
class ScriptWindow;
class QIdListWidgetItem;
class QMenuBar;
class QListWidget;
class QLineEdit;

class ServerWidget: public QWidget
{
    Q_OBJECT

public:
    ServerWidget(Server *myserver);
    QMenuBar *createMenuBar();
    bool isServerTrayPopupAllowed() const { return showTrayPopup; }
    bool isMinimizeToTrayAllowed() const { return minimizeToTray; }
    bool isDoubleClickIcon() const { return doubleClick; }

    void atShutDown();
    
public slots:
    void showContextMenu(const QPoint &p);
    void openTempBanDialog(int pId);
    void openConfig();
    void openPlayers();
    void openAntiDos();
    void openScriptWindow();
    void openTiersWindow();
    void openBattleConfigWindow();
    void openSqlConfigWindow();
    void openPluginManager();
    void openModsWindow();

    void sendServerMessage();
    void addChatline(const QString &line);

    void playerConnects(int id);
    void playerChangedName(int id, const QString &name);
    void playerDisconnects(int id);
    void clearChat();
signals:
    void menuBarChanged();
private slots:
    /* Relies on sender() */
    void openPluginConfig();
    void loadGuiSettings();
private:
    QListWidget *mylist;
    QScrollDownTextBrowser *mymainchat;
    QLineEdit *myline;
    
    QScrollDownTextBrowser *mainchat();
    QListWidget *list();

    QHash<int, QIdListWidgetItem *> myplayersitems;

    QPointer<ScriptWindow> myscriptswindow;

    Server *server;
    bool minimizeToTray, showTrayPopup, doubleClick;
};

#endif	/* SERVERWIDGET_H */

