#ifndef CHANNEL_H
#define CHANNEL_H

#include <QtGui>
#include "../PokemonInfo/networkstructs.h"

class QIdTreeWidgetItem;
class QScrollDownTextEdit;
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

    QTreeWidget *playersWidget() {
        return myplayers;
    }
    QTreeWidget *battlesWidget() {
        return battleList;
    }
    QScrollDownTextEdit *mainChat() {
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
    void battleStarted(int battleid, int id1, int id2);
    void battleReceived(int battleid, int id1, int id2);
    void battleEnded(int battleid, int res, int winner, int loser);
    void playerLogOut(int id);
    void updateState(int id);
    void removePlayer(int id);
    void insertNewPlayer(int id);

    void changeName(int id, const QString &name);

    void printLine(const QString &str);
    void printHtml(const QString &str);

    void dealWithCommand(int command, QDataStream *stream);
    QHash<qint32, Battle> &getBattles();
signals:
    void quitChannel(int chanid);
public slots:
    void showContextMenu(const QPoint &point);
private:
    QTreeWidget *myplayers;
    QHash<int, QIdTreeWidgetItem *> myplayersitems;
    QHash<QString, QIdTreeWidgetItem *> mytiersitems;
    QTreeWidget *battleList;
    QHash<int, QIdTreeWidgetItem *> battleItems;
    QHash<qint32, Battle> battles;
    QScrollDownTextEdit *mymainchat;

    Client *client;

    QSet<int> ownPlayers;

    QString myname;
    int myid;

    QIdTreeWidgetItem *item(int  id);
};

#endif // CHANNEL_H
