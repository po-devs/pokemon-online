/* 
 * File:   serverwidget.h
 * Author: remco
 *
 * Created on July 9, 2010, 5:45 PM
 */

#ifndef SERVERWIDGET_H
#define	SERVERWIDGET_H

#include <QtGui>
#include "server.h"

class QScrollDownTextBrowser;
class ScriptWindow;
class QIdListWidgetItem;


class ServerWidget: public QWidget
{
    Q_OBJECT

public:
    ServerWidget(Server *myserver);
    QMenuBar *createMenuBar();
    bool isServerTrayPopupAllowed() const { return server->isTrayPopupAllowed(); }
    bool isMinimizeToTrayAllowed() const { return server->isMinimizeToTrayAllowed(); }

    void atShutDown();
    
public slots:
    void showContextMenu(const QPoint &p);
    void openConfig();
    void openPlayers();
    void openAntiDos();
    void openScriptWindow();
    void openTiersWindow();
    void openBattleConfigWindow();
    void openSqlConfigWindow();
    void openPluginManager();

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
private:
    QListWidget *mylist;
    QScrollDownTextBrowser *mymainchat;
    QLineEdit *myline;
    
    QScrollDownTextBrowser *mainchat();
    QListWidget *list();

    QHash<int, QIdListWidgetItem *> myplayersitems;

    QPointer<ScriptWindow> myscriptswindow;

    Server *server;
};

#endif	/* SERVERWIDGET_H */

