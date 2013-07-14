#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtGui>
#ifdef QT5
#include <QtWidgets>
#endif
#include "../PokemonInfo/networkstructs.h"

class QTreeWidget;
class QTreeWidgetItem;
class QIdTreeWidgetItem;
class QScrollDownTextBrowser;
class Client;
class DataStream;

class Channel : public QObject {
    Q_OBJECT
public:
    Channel(const QString &name, int id, Client *parent);
    ~Channel();

    Q_INVOKABLE int id() const {
        return myid;
    }
    void setId(int id);

    int ownId() const;
    Q_INVOKABLE QString name() const {
        return myname;
    }
    void setName(const QString &name);

    QTreeWidget *playersWidget() {
        return myplayers;
    }
    QTreeWidget *battlesWidget() {
        return battleList;
    }
    QScrollDownTextBrowser *mainChat() {
        return mymainchat;
    }
    Q_INVOKABLE bool hasPlayer(int player) const {
        return ownPlayers.contains(player);
    }
    Q_INVOKABLE bool hasRemoteKnowledgeOf(int player) const;
    QString name(int player);

    Q_INVOKABLE void sortAllPlayersByTier();
    Q_INVOKABLE void sortAllPlayersNormally();
    void playerReceived(int playerid);
    void placeItem(QIdTreeWidgetItem *item, QTreeWidgetItem *parent=NULL);
    void placeTier(const QString &tier);
    void battleStarted(int battleid, int id1, int id2, QString tier);
    void battleReceived(int battleid, int id1, int id2, QString tier);
    void battleEnded(int battleid, int res, int winner, int loser);
    void playerLogOut(int id);
    void updateState(int id);
    void removePlayer(int id);
    void insertNewPlayer(int id);
    void receivePlayerList(const QVector<int> &ids);
    Q_INVOKABLE QStringList players();
    Q_INVOKABLE void addEvent(int event);
    Q_INVOKABLE void removeEvent(int event);
    Q_INVOKABLE bool eventEnabled(int event);
    Q_INVOKABLE void resetEvents();
    Q_INVOKABLE void restoreEventSettings();

    void cleanData();

    Q_INVOKABLE void makeReadyToQuit() {
        readyToQuit = true;
    }
    Q_INVOKABLE bool isReadyToQuit() const {
        return readyToQuit;
    }

    void signalDisconnection();

    void changeName(int id, const QString &name);

    Q_INVOKABLE QString addChannelLinks(const QString &line);
    Q_INVOKABLE void checkFlash(const QString &haystack, const QString &needle);
    Q_INVOKABLE void printLine(const QString &str, bool flashing = true, bool act=true, bool global = false);
    Q_INVOKABLE void printHtml(const QString &str, bool act = true, bool global = false);

    void dealWithCommand(int command, DataStream *stream);
    QHash<qint32, Battle> &getBattles();

    /* removes if necessary (i.e. empty) a tier */
    void cleanTier(QTreeWidgetItem *tier);

    enum State {
        Inactive,
        Active,
        Flashed
    };
    int state;
signals:
    void quitChannel(int chanid);
    void battleReceived2(int battleid, int id1, int id2);
    void activated(Channel *c);
    void pactivated(Channel *c);
public slots:
    void showContextMenu(const QPoint &point);
    void anchorClicked(const QUrl &url);
private:
    QTreeWidget *myplayers;
    QMultiHash<int, QIdTreeWidgetItem *> myplayersitems;
    QHash<QString, QTreeWidgetItem *> mytiersitems;
    QTreeWidget *battleList;
    QHash<int, QIdTreeWidgetItem *> battleItems;
    QHash<qint32, Battle> battles;
    QScrollDownTextBrowser *mymainchat;

    Client *client;

    QSet<int> ownPlayers;

    QString myname;
    int myid;
    int events;
    bool readyToQuit;

    bool stillLoading;

    QIdTreeWidgetItem *item(int  id);
    QList<QIdTreeWidgetItem *> items(int  id);
    void insertPlayerItems(int playerid);
};

#endif // CHANNEL_H
