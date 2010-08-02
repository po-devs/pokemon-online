#include "channel.h"
#include "client.h"

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

    battleList->setColumnCount(2);
    battleList->setHeaderLabels(QStringList() << tr("Player 1") << tr("Player 2"));
    battleList->setSortingEnabled(true);
    battleList->resizeColumnToContents(0);
    battleList->setIndentation(0);

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myplayers, SIGNAL(itemActivated(QTreeWidgetItem*, int)), client, SLOT(seeInfo(QTreeWidgetItem*)));
    connect(battleList, SIGNAL(itemActivated(QTreeWidgetItem*,int)), client, SLOT(battleListActivated(QTreeWidgetItem*)));
}

void Channel::showContextMenu(const QPoint &requested)
{
    QIdTreeWidgetItem *item = dynamic_cast<QIdTreeWidgetItem*>(myplayers->itemAt(requested));

    if (item && item->id() != 0)
    {
        QMenu *menu = new QMenu(this);

        createIntMapper(menu->addAction(tr("&Challenge")), SIGNAL(triggered()), client, SLOT(seeInfo(int)), item->id());

        createIntMapper(menu->addAction(tr("&View Ranking")), SIGNAL(triggered()), client, SLOT(seeRanking(int)), item->id());
        if (item->id() == ownId()) {
            if (away()) {
                createIntMapper(menu->addAction(tr("Go &Back")), SIGNAL(triggered()), client, SLOT(goAway(int)), false);
            } else {
                createIntMapper(menu->addAction(tr("Go &Away")), SIGNAL(triggered()), client, SLOT(goAway(int)), true);
            }
        } else {
            createIntMapper(menu->addAction(tr("&Send Message")), SIGNAL(triggered()), client, SLOT(startPM(int)), item->id());
            if (player(item->id()).battling())
                createIntMapper(menu->addAction(tr("&Watch Battle")), SIGNAL(triggered()), client, SLOT(watchBattleOf(int)), item->id());
            if (myIgnored.contains(item->id()))
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

        menu->exec(mapToGlobal(requested));
    }
}
