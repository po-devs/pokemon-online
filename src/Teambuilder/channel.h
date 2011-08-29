#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtGui>
#include "../PokemonInfo/networkstructs.h"

class QIdTreeWidgetItem;
class QScrollDownTextBrowser;
class Client;

class Channel : public QObject {
    Q_OBJECT
public:
    Channel(const QString &name, int id, Client *parent);
    ~Channel();

    int id() const {
        return myid;
    }
    int ownId() const;
    QString name() const {
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
    bool hasPlayer(int player) const {
        return ownPlayers.contains(player);
    }
    bool hasRemoteKnowledgeOf(int player) const;
    QString name(int player);

    void sortAllPlayersByTier();
    void sortAllPlayersNormally();
    void playerReceived(int playerid);
    void placeItem(QIdTreeWidgetItem *item, QTreeWidgetItem *parent=NULL);
    void placeTier(const QString &tier);
    void battleStarted(int battleid, int id1, int id2);
    void battleReceived(int battleid, int id1, int id2);
    void battleEnded(int battleid, int res, int winner, int loser);
    void playerLogOut(int id);
    void updateState(int id);
    void removePlayer(int id);
    void insertNewPlayer(int id);
    void receivePlayerList(const QVector<int> &ids);
    void addEvent(int event);
    void removeEvent(int event);
    bool eventEnabled(int event);
    void resetEvents();
    void restoreEventSettings();

    void makeReadyToQuit() {
        readyToQuit = true;
    }
    bool isReadyToQuit() const {
        return readyToQuit;
    }

    void signalDisconnection();

    void changeName(int id, const QString &name);

    QString addChannelLinks(const QString &line);
    void checkFlash(const QString &haystack, const QString &needle);
    void printLine(const QString &str, bool flashing = true);
    void printHtml(const QString &str);

    void dealWithCommand(int command, QDataStream *stream);
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
    QHash<int, QIdTreeWidgetItem *> myplayersitems;
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

    QIdTreeWidgetItem *item(int  id);
    void getBackAllPlayerItems();
};

#endif // CHANNEL_H
