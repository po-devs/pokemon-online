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

    int id() const {
        return myid;
    }
    int name() const {
        return myname;
    }

    QTreeWidget *playersWidget() {
        return myplayers;
    }
    QTreeWidget *battlesWidget() {
        return mybattles;
    }
    QScrollDownTextEdit *mainChat() {
        return mymainchat;
    }
public slots:
    void showContextMenu(const QPoint &point);
private:
    QTreeWidget *myplayers;
    QHash<int, QIdTreeWidgetItem *> myplayersitems;
    QHash<QString, QIdTreeWidgetItem *> mytiersitems;
    QTreeWidget *battleList;
    QHash<qint32, Battle> battles;
    QHash<int, QIdTreeWidgetItem *> battleItems;
    QScrollDownTextEdit *mymainchat;
    Client *client;

    QString myname;
    QString myid;

    QIdTreeWidgetItem *item(int  id);
};

#endif // CHANNEL_H
