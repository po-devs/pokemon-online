#include "channel.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"

Channel::Channel(const QString &name, int id, Client *parent)
    : QObject(parent), client(parent), myname(name), myid(id)
{
    /* Those will actually be gotten back by the client itself, when
       he adds the channel */
    mymainchat = new QScrollDownTextEdit();
    myplayers = new QTreeWidget();
    battleList = new QTreeWidget();

    mymainchat->setObjectName("MainChat");

    myplayers->setColumnCount(1);
    myplayers->header()->hide();
    myplayers->setIconSize(QSize(24,24));
    myplayers->setIndentation(13);
    myplayers->setObjectName("PlayerList");
    myplayers->setContextMenuPolicy(Qt::CustomContextMenu);

    battleList->setColumnCount(2);
    battleList->setHeaderLabels(QStringList() << tr("Player 1") << tr("Player 2"));
    battleList->setSortingEnabled(true);
    battleList->resizeColumnToContents(0);
    battleList->setIndentation(0);

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myplayers, SIGNAL(itemActivated(QTreeWidgetItem*, int)), client, SLOT(seeInfo(QTreeWidgetItem*)));
    connect(battleList, SIGNAL(itemActivated(QTreeWidgetItem*,int)), client, SLOT(battleListActivated(QTreeWidgetItem*)));
}

int Channel::ownId() const {
    return client->ownId();
}

void Channel::showContextMenu(const QPoint &requested)
{
    QIdTreeWidgetItem *item = dynamic_cast<QIdTreeWidgetItem*>(myplayers->itemAt(requested));

    if (item && item->id() != 0)
    {
        QMenu *menu = new QMenu(client);

        createIntMapper(menu->addAction(tr("&Challenge")), SIGNAL(triggered()), client, SLOT(seeInfo(int)), item->id());

        createIntMapper(menu->addAction(tr("&View Ranking")), SIGNAL(triggered()), client, SLOT(seeRanking(int)), item->id());
        if (item->id() == ownId()) {
            if (client->away()) {
                createIntMapper(menu->addAction(tr("Go &Back")), SIGNAL(triggered()), client, SLOT(goAway(int)), false);
            } else {
                createIntMapper(menu->addAction(tr("Go &Away")), SIGNAL(triggered()), client, SLOT(goAway(int)), true);
            }
        } else {
            createIntMapper(menu->addAction(tr("&Send Message")), SIGNAL(triggered()), client, SLOT(startPM(int)), item->id());
            if (client->player(item->id()).battling())
                createIntMapper(menu->addAction(tr("&Watch Battle")), SIGNAL(triggered()), client, SLOT(watchBattleOf(int)), item->id());
            if (client->isIgnored(item->id()))
                createIntMapper(menu->addAction(tr("&Remove Ignore")), SIGNAL(triggered()), client, SLOT(removeIgnore(int)), item->id());
            else
                createIntMapper(menu->addAction(tr("&Ignore")), SIGNAL(triggered()), client, SLOT(ignore(int)), item->id());
        }

        int myauth = client->ownAuth();

        if (myauth > 0) {
            createIntMapper(menu->addAction(tr("&Control Panel")), SIGNAL(triggered()), client, SLOT(controlPanel(int)), item->id());

            int otherauth = client->player(item->id()).auth;

            if (otherauth < myauth) {
                menu->addSeparator();
                createIntMapper(menu->addAction(tr("&Kick")), SIGNAL(triggered()), this, SLOT(kick(int)), item->id());

                /* If you're an admin, you can ban */
                if (myauth >= 2) {
                    menu->addSeparator();
                    createIntMapper(menu->addAction(tr("&Ban")), SIGNAL(triggered()), this, SLOT(ban(int)), item->id());
                }
            }
        }

        menu->exec(client->mapToGlobal(requested));
    }
}

void Channel::sortAllPlayersByTier()
{
    foreach(QIdTreeWidgetItem *it, myplayersitems)
    {
        if (it->parent())
            it->parent()->takeChild(it->parent()->indexOfChild(it));
        else
            myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(it));
    }

    foreach(QIdTreeWidgetItem *it, mytiersitems)
    {
        delete myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(it));
    }

    mytiersitems.clear();

    foreach(QString tier, client->getTierList()) {
        QIdTreeWidgetItem *it = new QIdTreeWidgetItem(0, QStringList() << tier);
        //it->setBackgroundColor("#0CA0DD");
        //it->setColor("white");
        QFont f = it->font(0);
        f.setPixelSize(14);
        it->setFont(0,f);
        it->setText(0,tier);
        myplayers->addTopLevelItem(it);
        mytiersitems.insert(tier, it);

    }
    myplayers->expandAll();

    QHash<int, QIdTreeWidgetItem *>::iterator iter;

    for (iter = myplayersitems.begin(); iter != myplayersitems.end(); ++iter) {
        QString tier = client->tier(iter.key());

        if (mytiersitems.contains(tier)) {
            placeItem(iter.value(), mytiersitems.value(tier));
        } else {
            placeItem(iter.value());
        }
    }

    myplayers->headerItem()->sortChildren(0, Qt::AscendingOrder);
    foreach(QIdTreeWidgetItem *it, mytiersitems) {
        it->sortChildren(0, Qt::AscendingOrder);
    }
}

void Channel::sortAllPlayersNormally()
{
    foreach(QIdTreeWidgetItem *it, myplayersitems)
    {
        if (it->parent())
            it->parent()->takeChild(it->parent()->indexOfChild(it));
        else
            myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(it));
    }

    foreach(QIdTreeWidgetItem *it, mytiersitems)
    {
        delete myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(it));
    }

    mytiersitems.clear();

    QHash<int, QIdTreeWidgetItem *>::iterator iter;

    for (iter = myplayersitems.begin(); iter != myplayersitems.end(); ++iter) {
        placeItem(iter.value(), NULL);
    }
}

void Channel::placeItem(QIdTreeWidgetItem *item, QTreeWidgetItem *parent)
{
    if(item->id() >= 0) {
        if(parent == NULL) {
            myplayers->addTopLevelItem(item);
            myplayers->sortItems(0,Qt::AscendingOrder);
        } else {
            parent->addChild(item);
            parent->sortChildren(0,Qt::AscendingOrder);
        }
    }
}

void Channel::battleStarted(int bid, int id1, int id2)
{
    if (!hasPlayer(id1) && !hasPlayer(id2))
        return;

    battles.insert(bid, Battle(id1, id2));
    QIdTreeWidgetItem *it = new QIdTreeWidgetItem(bid, QStringList() << name(id1) << name(id2));
    battleItems.insert(bid, it);
    battleList->addTopLevelItem(it);

    if (id1 != 0) {
        item(id1)->setToolTip(0,tr("Battling against %1").arg(name(id2)));
        updateState(id1);
    }
    if (id2 != 0) {
        item(id2)->setToolTip(0,tr("Battling against %1").arg(name(id1)));
        updateState(id2);
    }
}

QIdTreeWidgetItem *Channel::item(int id) {
    return myplayersitems.value(id);
}

QString Channel::name(int player)
{
    return client->name(player);
}

void Channel::battleEnded(int battleid, int res, int winner, int loser)
{
    if (battles.contains(battleid)) {
        battles.remove(battleid);

        battleList->takeTopLevelItem(battleList->indexOfTopLevelItem(battleItems[battleid]));
        delete battleItems.take(battleid);
    }

    if (client->showPEvents || winner == ownId() || loser == ownId()) {
        if (res == Forfeit) {
            printLine(tr("%1 forfeited against %2.").arg(name(loser), name(winner)));
        } else if (res == Tie) {
            printLine(tr("%1 and %2 tied.").arg(name(loser), name(winner)));
        } else if (res == Win) {
            printLine(tr("%1 won against %2.").arg(name(winner), name(loser)));
        }
    }
}

/*

void Client::battleListReceived(const QHash<int, Battle> &battles)
{
    this->battles = battles;

    QHashIterator<int, Battle> h(battles);

    while (h.hasNext()) {
        h.next();
        QIdTreeWidgetItem *it = new QIdTreeWidgetItem(h.key(), QStringList() << name(h.value().id1) << name(h.value().id2));
        battleItems.insert(h.key(), it);
        battleList->addTopLevelItem(it);
    }
}
*/

void Channel::playerReceived(int playerid) {
    if (!ownPlayers.contains(playerid)) {
        insertNewPlayer(playerid);
        return;
    }

    QIdTreeWidgetItem *item = myplayersitems.value(playerid);

    if (item->parent())
        item->parent()->takeChild(item->parent()->indexOfChild(item));
    else
        myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(item));

    item->setText(0, client->info(playerid).name);
    item->setColor(client->color(playerid));

    QString tier = client->tier(playerid);
    if (client->sortBT && mytiersitems.contains(tier)) {
        placeItem(item, mytiersitems.value(tier));
    } else {
        placeItem(item,NULL);
    }

    updateState(playerid);
}

void Channel::insertNewPlayer(int playerid)
{
    ownPlayers.insert(playerid);
    QIdTreeWidgetItem *item = new QIdTreeWidgetItem(playerid, QStringList());
    QFont f = item->font(0);
    f.setBold(true);
    item->setFont(0,f);
    item->setText(0,name(playerid));
    item->setColor(client->color(playerid));
    myplayersitems.insert(playerid, item);

    QString tier = client->tier(playerid);
    if (client->sortBT && mytiersitems.contains(tier)) {
        placeItem(item, mytiersitems.value(tier));
    } else {
        placeItem(item,NULL);
    }

    updateState(playerid);
}

void Channel::dealWithCommand(int command, QDataStream *stream)
{
    QDataStream &in = *stream;

    if (command == NetworkCli::JoinChannel) {
        qint32 id;
        in >> id;

        if (hasPlayer(id))
            return;

        playerReceived(id);
        if (client->showPEvents) {
            printLine(tr("%1 has joined the channel.").arg(name(command)));
        }
    } else if (command == NetworkCli::ChannelMessage) {
        QString message;

        in >> message;
        printLine(message);
    } else {
        printLine(QString::number(command));
    }
}

void Channel::updateState(int id)
{
    int auth = client->auth(id);
    if (item(id)) {
        if (client->isIgnored(id)) {
            item(id)->setIcon(0, client->statusIcon(auth,Client::Ignored));
            return;
        }
        if (client->player(id).battling()) {
            item(id)->setIcon(0, client->statusIcon(auth,Client::Battling));
        } else if (client->player(id).away()) {
            item(id)->setIcon(0, client->statusIcon(auth,Client::Away));
            item(id)->setToolTip(0, "");
        } else {
            item(id)->setIcon(0, client->statusIcon(auth,Client::Available));
            item(id)->setToolTip(0, "");
        }
    }
}

void Channel::playerLogOut(int id) {
    QString name = this->name(id);

    if (client->showPEvents)
        printLine(tr("%1 logged out.").arg(name));
}

void Channel::removePlayer(int id) {
    QIdTreeWidgetItem *item = myplayersitems.take(id);

    if (item->parent())
        item->parent()->takeChild(item->parent()->indexOfChild(item));
    else
        myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(item));

    delete item;

    ownPlayers.remove(id);
}

void Channel::printLine(const QString &line)
{
    QString timeStr = "";
    if(client->showTS)
        timeStr = "(" + QTime::currentTime().toString() + ") ";
    if (line.length() == 0) {
        mainChat()->insertPlainText("\n");
        return;
    }
    /* Only activates if no window has focus */
    if (!QApplication::activeWindow()) {
        if (line.contains(QRegExp(QString("\\b%1\\b").arg(name(ownId())),Qt::CaseInsensitive))) {
            client->raise();
            client->activateWindow();
        }
    }
    if (line.leftRef(3) == "***") {
        mainChat()->insertHtml("<span style='color:magenta'>" + timeStr + escapeHtml(line) + "</span><br />");
        return;
    }
    /* Let's add colors */
    int pos = line.indexOf(':');
    if ( pos != -1 ) {
        QString beg = line.left(pos);
        QString end = line.right(line.length()-pos-1);

        int id = client->id(beg);

        if (beg == "~~Server~~") {
            mainChat()->insertHtml("<span style='color:orange'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else if (beg == "Welcome Message") {
            mainChat()->insertHtml("<span style='color:blue'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else if (id == -1) {
            mainChat()->insertHtml("<span style='color:#3daa68'>" + timeStr + "<b>" + escapeHtml(beg)  + "</b>:</span>" + escapeHtml(end) + "<br />");
        } else {
            if (client->isIgnored(id))
                return;
            QColor color = client->color(id);

            if (client->auth(id) > 0 && client->auth(id) <= 3) {
                mainChat()->insertHtml("<span style='color:" + color.name() + "'>" + timeStr + "+<i><b>" + escapeHtml(beg) + ":</i></b></span>" + escapeHtml(end) + "<br />");
            }
            else if (id == ownId()) {
                mainChat()->insertHtml("<span style='color:" + color.name() + "'>" + timeStr + "<b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
            } else {
                mainChat()->insertHtml("<span style='color:" + color.name() + "'>" + timeStr + "<b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
            }
        }
    } else {
        mainChat()->insertPlainText( timeStr + line + "\n");
    }
}

void Channel::printHtml(const QString &str)
{
    mainChat()->insertHtml(str);
}
