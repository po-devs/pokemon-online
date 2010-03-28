#include "playerswindow.h"

#include "security.h"

PlayersWindow::PlayersWindow(QWidget *parent)
    : QWidget (parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QGridLayout *mylayout = new QGridLayout(this);

    QMap<QString, SecurityManager::Member> members = SecurityManager::getMembers();

    mytable = new QCompactTable(members.size(),6);
    mytable->setShowGrid(true);
    mylayout->addWidget(mytable,0,0,1,6);

    QMap<int, QString> authgrade;
    authgrade[0] = "User";
    authgrade[1] = "Mod";
    authgrade[2] = "Admin";

    QStringList headers;
    headers << "Player" << "Authority" << "Banned Status" << "Registered" << "IP" << "Last Appearance";
    mytable->setHorizontalHeaderLabels(headers);

    int i = 0;

    foreach(SecurityManager::Member m, members) {
        playersbynum.push_back(m.name);

        QTableWidgetItem *witem = new QTableWidgetItem(m.name);
        witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
        mytable->setItem(i, 0, witem);

        witem = new QTableWidgetItem(authgrade[m.authority()]);
        witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
        mytable->setItem(i, 1, witem);

        witem = new QTableWidgetItem(m.isBanned() ? "Banned" : "Fine");
        witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
        mytable->setItem(i, 2, witem);

        witem = new QTableWidgetItem(m.isProtected() ? "Yes" : "No");
        witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
        mytable->setItem(i, 3, witem);

        witem = new QTableWidgetItem(m.ip.trimmed());
        witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
        mytable->setItem(i, 4, witem);

        witem = new QTableWidgetItem(m.date);
        witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
        mytable->setItem(i, 5, witem);

        i++;
    }

    QPushButton *_user = new QPushButton(tr("&User"));
    QPushButton *_mod = new QPushButton(tr("&Mod"));
    QPushButton *_admin = new QPushButton(tr("&Admin"));
    QPushButton *_ban = new QPushButton(tr("&Ban"));
    QPushButton *_unban = new QPushButton(tr("U&nban"));
    QPushButton *_clpass = new QPushButton(tr("&Clear Password"));

    mylayout->addWidget(_user,1,0);
    mylayout->addWidget(_mod,1,1);
    mylayout->addWidget(_admin,1,2);
    mylayout->addWidget(_ban,1,3);
    mylayout->addWidget(_unban,1,4);
    mylayout->addWidget(_clpass,1,5);

    if (mytable->rowCount() == 0)
        return;

    connect(_user,SIGNAL(clicked()),SLOT(user()));
    connect(_mod,SIGNAL(clicked()),SLOT(mod()));
    connect(_admin,SIGNAL(clicked()),SLOT(admin()));
    connect(_ban,SIGNAL(clicked()),SLOT(ban()));
    connect(_unban,SIGNAL(clicked()),SLOT(unban()));
    connect(_clpass,SIGNAL(clicked()),SLOT(clpass()));
}

void PlayersWindow::ban()
{
    SecurityManager::ban(playersbynum[mytable->currentRow()]);
    mytable->item(mytable->currentRow(), 2)->setText("Banned");
    emit banned(playersbynum[mytable->currentRow()]);
}

void PlayersWindow::unban()
{
    SecurityManager::unban(playersbynum[mytable->currentRow()]);
    mytable->item(mytable->currentRow(), 2)->setText("Fine");
}

void PlayersWindow::user()
{
    SecurityManager::setauth(playersbynum[mytable->currentRow()], 0);
    mytable->item(mytable->currentRow(), 1)->setText("User");
    emit authChanged(playersbynum[mytable->currentRow()],0);
}

void PlayersWindow::mod()
{
    SecurityManager::setauth(playersbynum[mytable->currentRow()], 1);
    mytable->item(mytable->currentRow(), 1)->setText("Mod");
    emit authChanged(playersbynum[mytable->currentRow()],1);
}

void PlayersWindow::admin()
{
    SecurityManager::setauth(playersbynum[mytable->currentRow()], 2);
    mytable->item(mytable->currentRow(), 1)->setText("Admin");
    emit authChanged(playersbynum[mytable->currentRow()],2);
}

void PlayersWindow::clpass()
{
    SecurityManager::clearPass(playersbynum[mytable->currentRow()]);
    mytable->item(mytable->currentRow(), 3)->setText("No");
}
