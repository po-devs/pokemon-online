#include "channel.h"
#include "client.h"
#include "poketextedit.h"
#include "remove_direction_override.h"
#include "theme.h"

Channel::Channel(const QString &name, int id, Client *parent)
    : QObject(parent), state(Inactive), client(parent), myname(name), myid(id), readyToQuit(false), stillLoading(true)
{
    /* Those will actually be gotten back by the client itself, when
       he adds the channel */
    mymainchat = new PokeTextEdit();
    myplayers = new QTreeWidget();
    battleList = new QTreeWidget();

    mymainchat->setObjectName("MainChat");
    mymainchat->setOpenExternalLinks(false);
    
    QFile stylesheet(Theme::path("mainchat.css"));
    stylesheet.open(QIODevice::ReadOnly);
    mymainchat->document()->setDefaultStyleSheet(stylesheet.readAll());
    connect(mymainchat, SIGNAL(anchorClicked(QUrl)), SLOT(anchorClicked(QUrl)));

    myplayers->setColumnCount(2);
    myplayers->setColumnHidden(1, true);
    myplayers->header()->hide();
    myplayers->setIconSize(QSize(18,18));
    myplayers->setIndentation(13);
    myplayers->setObjectName("PlayerList");
    myplayers->setContextMenuPolicy(Qt::CustomContextMenu);

    battleList->setColumnCount(2);
    battleList->setHeaderLabels(QStringList() << tr("Player 1") << tr("Player 2"));
    battleList->setSortingEnabled(true);
    battleList->resizeColumnToContents(0);
    battleList->setIndentation(0);

    events = -1;
    restoreEventSettings();

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myplayers, SIGNAL(itemActivated(QTreeWidgetItem*, int)), client, SLOT(seeInfo(QTreeWidgetItem*)));
    connect(battleList, SIGNAL(itemActivated(QTreeWidgetItem*,int)), client, SLOT(battleListActivated(QTreeWidgetItem*)));
}

Channel::~Channel()
{
}

int Channel::ownId() const {
    return client->ownId();
}

void Channel::setName(const QString &name) {
    myname = name;
    restoreEventSettings();
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
                createIntMapper(menu->addAction(tr("&Kick")), SIGNAL(triggered()), client, SLOT(kick(int)), item->id());

                /* If you're an admin, you can ban */
                if (myauth >= 2) {
                    menu->addSeparator();
                    createIntMapper(menu->addAction(tr("&Ban")), SIGNAL(triggered()), client, SLOT(ban(int)), item->id());
                }
            }
        }

        menu->exec(myplayers->mapToGlobal(requested));
    }

    item = dynamic_cast<QIdTreeWidgetItem*>(myplayers->itemAt(requested));
}

void Channel::anchorClicked(const QUrl &url)
{
    // Hack against Qt's bug?
    // http://www.qtcentre.org/archive/index.php/t-2858.html
    int hv = mymainchat->horizontalScrollBar()->value();
    int vv = mymainchat->verticalScrollBar()->value();
    mymainchat->setSource(QUrl());
    mymainchat->scrollToAnchor(url.toString());
    mymainchat->horizontalScrollBar()->setValue(hv);
    mymainchat->verticalScrollBar()->setValue(vv);
    // study the URL scheme
    if (url.scheme()=="po") {
        if(url.path().leftRef(5) == "join/") {
            QString cname = url.path().mid(5);
            client->join(cname);
            client->activateChannel(cname);
        }
    } else {
        QDesktopServices::openUrl(url);
    }
}

void Channel::getBackAllPlayerItems()
{
    foreach(QIdTreeWidgetItem *it, myplayersitems) {
        if (it->parent())
            it->parent()->takeChild(it->parent()->indexOfChild(it));
        else {
            myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(it));
        }
    }
}

void Channel::sortAllPlayersByTier()
{
    getBackAllPlayerItems();
    myplayers->clear();
    mytiersitems.clear();

    QHash<int, QIdTreeWidgetItem *>::iterator iter;

    for (iter = myplayersitems.begin(); iter != myplayersitems.end(); ++iter) {
        QString tier = client->tier(iter.key());

        if (!mytiersitems.contains(tier))
            placeTier(tier);

        placeItem(iter.value(), mytiersitems.value(tier));
    }

    myplayers->expandAll();
}

void Channel::placeTier(const QString &tier)
{
    if (mytiersitems.contains(tier))
        return;

    QTreeWidgetItem *tierItem = client->tierRoot.addTier(myplayers->invisibleRootItem(), tier);

    mytiersitems.insert(tier, tierItem);
}

void Channel::sortAllPlayersNormally()
{
    getBackAllPlayerItems();
    myplayers->clear();
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

            if (!stillLoading) {
                myplayers->sortItems(0,Qt::AscendingOrder);
                if(client->sortBA) {
                    myplayers->sortItems(1,Qt::DescendingOrder);
                }
            }
        } else {
            parent->addChild(item);

            if (!stillLoading) {
                parent->sortChildren(0,Qt::AscendingOrder);
                if(client->sortBA) {
                    parent->sortChildren(1,Qt::DescendingOrder);
                }
            }
        }
    }
}

QHash<qint32, Battle> & Channel::getBattles()
{
    return battles;
}

void Channel::battleStarted(int bid, int id1, int id2)
{
    if (!hasPlayer(id1) && !hasPlayer(id2))
        return;

    if (eventEnabled(Client::BattleEvent) || id1 == ownId() || id2 == ownId())
        printLine(tr("Battle between %1 and %2 started.").arg(name(id1), name(id2)), false);

    battleReceived(bid, id1, id2);

    if (id1 != 0 && item(id1) != NULL) {
        item(id1)->setToolTip(0,tr("Battling against %1").arg(name(id2)));
        updateState(id1);
    }
    if (id2 != 0 && item(id2) != NULL) {
        item(id2)->setToolTip(0,tr("Battling against %1").arg(name(id1)));
        updateState(id2);
    }
}

void Channel::battleReceived(int bid, int id1, int id2)
{
    if (battles.contains(bid))
        return;

    battles.insert(bid, Battle(id1, id2));
    QIdTreeWidgetItem *it = new QIdTreeWidgetItem(bid, QStringList() << name(id1) << name(id2));
    battleItems.insert(bid, it);
    battleList->addTopLevelItem(it);
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
    } else {
        return;
    }

    if (eventEnabled(Client::BattleEvent) || winner == ownId() || loser == ownId() || client->mySpectatingBattles.contains(battleid)) {
        if (res == Forfeit) {
            printLine(tr("%1 forfeited against %2.").arg(name(loser), name(winner)), false);
        } else if (res == Tie) {
            printLine(tr("%1 and %2 tied.").arg(name(loser), name(winner)), false);
        } else if (res == Win) {
            printLine(tr("%1 won against %2.").arg(name(winner), name(loser)), false);
        }
    }
}

void Channel::playerReceived(int playerid) {
    if (!ownPlayers.contains(playerid)) {
        insertNewPlayer(playerid);
        return;
    }

    QIdTreeWidgetItem *item = myplayersitems.value(playerid);
    QTreeWidgetItem *parent = item->parent();

    if (parent)
        parent->takeChild(parent->indexOfChild(item));
    else
        myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(item));

    changeName(playerid, client->name(playerid));

    item->setColor(client->color(playerid));
    item->setText(1,QString::number(client->auth(playerid)));

    QString tier = client->tier(playerid);
    if (client->sortBT) {
        if (!mytiersitems.contains(tier))
            placeTier(tier);

        placeItem(item, mytiersitems.value(tier));

    } else {
        placeItem(item, NULL);
    }

    updateState(playerid);

    cleanTier(parent);
}

/* When a player has a name updated, change all possible places of that name */
void Channel::changeName(int id, const QString &name)
{
    /* Playerslist */
    if (myplayersitems.contains(id))
        myplayersitems.value(id)->setText(0, name);

    /* Battleslist */
    QHashIterator<qint32, Battle> bit(battles);
    while (bit.hasNext()) {
        bit.next();

        QIdTreeWidgetItem *tit = battleItems.value(bit.key());
        if (bit.value().id1 == id) {
            tit->setText(0, name);
        } else if (bit.value().id2 == id) {
            tit->setText(1, name);
        }
    }
}

void Channel::insertNewPlayer(int playerid)
{
    ownPlayers.insert(playerid);
    QIdTreeWidgetItem *item = new QIdTreeWidgetItem(playerid, QStringList());
    QFont f = item->font(0);
    f.setBold(true);
    item->setFont(0,f);
    item->setText(0,name(playerid));
    item->setText(1,QString::number(client->auth(playerid)));
    item->setColor(client->color(playerid));
    myplayersitems.insert(playerid, item);

    QString tier = client->tier(playerid);
    if (client->sortBT && client->tierList.contains(tier)) {
        if (!mytiersitems.contains(tier))
            placeTier(tier);

        placeItem(item, mytiersitems.value(tier));
    } else {
        placeItem(item, NULL);
    }

    updateState(playerid);
}

void Channel::receivePlayerList(const QVector<int> &ids)
{
    foreach(int id, ids) {
        playerReceived(id);
    }
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

        if (eventEnabled(Client::ChannelEvent)) {
            printLine(tr("%1 joined the channel.").arg(name(id)), false);
        }
    } else if (command == NetworkCli::ChannelMessage) {
        QString message;

        in >> message;
        printLine(message);
    } else if (command == NetworkCli::HtmlChannel) {
        QString message;

        in >> message;
        printHtml(message);
    } else if (command == NetworkCli::BattleList) {
        QHash<qint32, Battle> battles;
        in >> battles;
        this->battles = battles;

        QHashIterator<int, Battle> h(battles);

        while (h.hasNext()) {
            h.next();
            QIdTreeWidgetItem *it = new QIdTreeWidgetItem(h.key(), QStringList() << name(h.value().id1) << name(h.value().id2));
            battleItems.insert(h.key(), it);
            battleList->addTopLevelItem(it);
            emit battleReceived2(h.key(), h.value().id1, h.value().id2);
        }

        // Battle list finished the channel loading, now it's a good time to sort the players
        stillLoading = false;
        if(client->sortBT) {
            sortAllPlayersByTier();
        } else {
            sortAllPlayersNormally();
        }
    } else if (command == NetworkCli::LeaveChannel) {
        qint32 id;
        in >> id;
        if (eventEnabled(Client::ChannelEvent)) {
            printLine(tr("%1 left the channel.").arg(name(id)), false);
        }
        /* Remove everything... */
        removePlayer(id);

        if (id == ownId()) {
            printHtml(tr("<i>You are not in the channel anymore</i>"));
            emit quitChannel(this->id());
        }
    } else if (command == NetworkCli::ChannelBattle) {
        qint32 id, id1, id2;
        in >> id >> id1 >> id2;
        emit battleReceived2(id, id1, id2);
        battleReceived(id, id1, id2);
    } else{
        printHtml(tr("<i>Unkown command received: %1. Maybe the client should be updated?</i>").arg(command));
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

    removePlayer(id);

    if (eventEnabled(Client::ChannelEvent))
        printLine(tr("%1 logged out.").arg(name), false);
}

void Channel::removePlayer(int id) {
    if (!ownPlayers.contains(id))
        return;
    /* Data */
    ownPlayers.remove(id);

    /* Players List */


    QIdTreeWidgetItem *item = myplayersitems.take(id);
    QTreeWidgetItem *parent = item->parent();

    if (parent)
        parent->takeChild(parent->indexOfChild(item));
    else
        myplayers->takeTopLevelItem(myplayers->indexOfTopLevelItem(item));
    delete item;

    /* Battleslist */
    QSet<int> dlt;
    QHashIterator<qint32, Battle> bit(battles);
    while (bit.hasNext()) {
        bit.next();

        QIdTreeWidgetItem *tit = battleItems.value(bit.key());
        if (!hasPlayer(bit.value().id1) && !hasPlayer(bit.value().id2)) {
            battlesWidget()->takeTopLevelItem(battlesWidget()->indexOfTopLevelItem(tit));
            delete tit;
            battleItems.remove(bit.key());
            dlt.insert(bit.key());
        }
    }

    foreach(int id, dlt) {
        battles.remove(id);
    }

    cleanTier(parent);
}

void Channel::cleanTier(QTreeWidgetItem *tier)
{
    while (tier && tier->childCount() == 0 && tier->parent()) {
        QTreeWidgetItem *next = tier->parent();
        next = tier->parent();
        tier->parent()->takeChild(tier->parent()->indexOfChild(tier));
        mytiersitems.remove(tier->text(0));
        delete tier;
        tier = next;
    }
}

bool Channel::hasRemoteKnowledgeOf(int player) const
{
    if (hasPlayer(player))
        return true;

    foreach(Battle b, battles) {
        if (b.id1 == player || b.id2 == player)
            return true;
    }

    return false;
}

QString Channel::addChannelLinks(const QString &line2)
{
    // make a mutable copy
    QString line = line2;
    /* scan for channel links */
    int pos = 0;
    pos = line.indexOf('#', pos);
    QString longest_match;
    while(pos != -1)
    {
        ++pos;
        QString longestName;
        QString longestChannelName;
        foreach(QString name, client->channelNames)
        {
            QString channelName = line.midRef(pos, name.length()).toString();
            bool res=channelName.toLower() == name.toLower();
            if(res && longestName.size() < channelName.size()) {
                longestName = name;
                longestChannelName = channelName;
            }
        }
        if(!longestName.isNull())
        {
            QString html = QString("<a href=\"po:join/%1\">#%2</a>").arg(longestName, longestChannelName);
            line.replace(pos-1, longestName.length()+1, html);
            pos += html.length()-1;
        }
        pos = line.indexOf('#', pos);
    }
    return line;
}

void Channel::checkFlash(const QString &haystack, const QString &needle)
{
    if (haystack.contains(QRegExp(needle, Qt::CaseInsensitive))) {
        emit pactivated(this);
    }
    /* Only activates if no window has focus */
    if (!QApplication::activeWindow()) {
        if (haystack.contains(QRegExp(needle, Qt::CaseInsensitive))) {
            QApplication::alert(client, 10000);
            client->raise();
        }
    }
}

void Channel::printLine(const QString &line, bool flashing, bool act)
{
    QString timeStr = "";
    if(client->showTS)
        timeStr = "(" + QTime::currentTime().toString() + ") ";
    if (line.length() == 0) {
        mainChat()->insertPlainText("\n");
        return;
    }

    if (act) {
        emit activated(this);
     }

    if (line.leftRef(3) == "***") {
        if (flashing)
            checkFlash(line, QString("\\b%1\\b").arg(QRegExp::escape(name(ownId()))));
        mainChat()->insertHtml("<span class='line action'>" + timeStr + removeDirectionOverride(addChannelLinks(escapeHtml(line))) + "</span><br />");
        return;
    }

    /* Let's add colors */
    int pos = line.indexOf(':');
    if ( pos != -1 ) {
        QString beg = line.left(pos);
        QString end = removeDirectionOverride(line.right(line.length()-pos-1));
        int id = client->id(beg);

        /* Messages from players from auth 3 and less have their html escaped */
        if (id == -1 || client->auth(id) <= 3)
            end = escapeHtml(end);
        else
            checkFlash(end, "<ping */ *>");

        if (flashing)
            checkFlash(end, QString("\\b%1\\b").arg(QRegExp::escape(name(ownId()))));

        end = addChannelLinks(end);

        const QRegExp nameNotInsideTag(QString("\\b(%1)\\b(?![^\\s<]*>)").arg(QRegExp::escape(name(ownId()))), Qt::CaseInsensitive);
        const QString addHilightClass("<span class='name-hilight'>\\1</span>");
        QString lineClass = "line";

        if (id != ownId() && end.contains(nameNotInsideTag)) { // Add stuff if we are to be flashed
            lineClass = "line line-hilight";
        }
 
        /* Add HTML to timeStr */
        timeStr = "<span class='timestamp'>" + timeStr + "</span>";

        QString nameClass = "name";
        if (id != -1)
            nameClass = "name name-auth-" + QString::number(client->auth(id));

        if (beg == "~~Server~~") {
            end = end.replace(nameNotInsideTag, addHilightClass);
            mainChat()->insertHtml("<span class='line server-message'><span class='server-message-begin'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + end + "</span><br />");
        } else if (beg == "Welcome Message") {
            mainChat()->insertHtml("<span class='line welcome-message'><span class='welcome-message-begin'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + end + "</span><br />");
        } else if (id == -1) {
            mainChat()->insertHtml("<span class='line script-message'><span class='script-message-begin'>" + timeStr + "<b>" + escapeHtml(beg)  + "</b>:</span>" + end + "</span><br />");
        } else {

            if (client->isIgnored(id))
                return;

            // If it is not our message, hilight our name if mentioned
            if (id != ownId())
                end = end.replace(nameNotInsideTag, addHilightClass);

            // Todo: maybe pull auth symbol from theme for all auth levels?
            QString authSymbol = client->auth(id) > 0 && client->auth(id) <= 3 ? "+" : ""; 

            QColor color = client->color(id);

            mainChat()->insertHtml("<span class='" + lineClass + "'><span style='color:" + color.name() + "'>" + timeStr + authSymbol + "<span class='" + nameClass + "'>" + escapeHtml(beg) + ":</span></span>" + end + "</span><br />");
        }
        
    } else {
        if (flashing) {
            checkFlash(line, QString("\\b%1\\b").arg(QRegExp::escape(name(ownId()))));
        }
        mainChat()->insertPlainText( timeStr + line + "\n");
    }
}


void Channel::printHtml(const QString &str, bool act)
{
    QRegExp id(QString("<\\s*([0-9]+)\\s*>"));
    if (str.contains(id) && client->isIgnored(id.cap(1).toInt())){
        return;
    }
    checkFlash(str, "<ping */ *>");

    QString timeStr = "";
    if(client->showTS)
        timeStr = "(" + QTime::currentTime().toString() + ") ";
    QRegExp rx("<timestamp */ *>",Qt::CaseInsensitive);
    mainChat()->insertHtml(removeDirectionOverride(QString(str).replace( rx, timeStr )) + "<br />");
    if (act) {
        emit activated(this);
    }
}

void Channel::addEvent(int event)
{
    if (events == -1)
        events = client->showPEvents;
    events |= event;
}

void Channel::removeEvent(int event)
{
    if (events == -1)
        events = client->showPEvents;
    events &= ~event;
}

bool Channel::eventEnabled(int event)
{
    int& showEvents = events == -1 ? client->showPEvents : events;
    return (showEvents & event) > 0;
}

void Channel::resetEvents()
{
    events = -1;
}

void Channel::restoreEventSettings()
{
    events = client->getEventsForChannel(myname);
}
