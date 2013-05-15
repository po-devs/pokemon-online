#include "playerswindow.h"

#include "security.h"

#include <QSqlQuery>

PlayersWindow::PlayersWindow(QWidget *parent, int expireDays)
    : QWidget (parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    resize(726, this->height());

    QGridLayout *mylayout = new QGridLayout(this);

    mytable = new QCompactTable(0,7);

    mytable->setShowGrid(true);
    mylayout->addWidget(mytable,0,0,1,6);

    QMap<int, QString> authgrade;
    authgrade[0] = "User";
    authgrade[1] = "Mod";
    authgrade[2] = "Admin";
    authgrade[3] = "Owner";

    QStringList headers;
    headers << "Player" << "Authority" << "Banned Status" << "Registered" << "IP" << "Last Appearance" << "Expires In";
    mytable->setHorizontalHeaderLabels(headers);

    QSqlQuery q;
    q.setForwardOnly(true);

    q.exec("select count(*) from trainers");

    if (q.next()) {
        mytable->setRowCount(q.value(0).toInt());
    }

    q.exec("select name, auth, banned, hash, ip, laston, ban_expire_time from trainers order by name asc");

    int i = 0;

    while(q.next()) {
        QTableWidgetItem *witem = new QTableWidgetItem(q.value(0).toString());
        mytable->setItem(i, 0, witem);

        witem = new QTableWidgetItem(authgrade[q.value(1).toInt()]);
        mytable->setItem(i, 1, witem);

        QString bannedString = "Banned";
        int expiration = q.value(6).toInt() - QDateTime::currentDateTimeUtc().toTime_t();
        if(expiration < 0) {
            if (q.value(6).toInt() != 0)
            bannedString = "Expires on Login";
        } else {
            if(expiration < 60) {
                if(expiration == 1) {
                    bannedString.append(QString(" (%1 second)").arg(expiration));
                } else {
                    bannedString.append(QString(" (%2 seconds)").arg(expiration));
                }
            } else {
                if(expiration >= 60) {
                    expiration = expiration / 60;
                    if(expiration == 1) {
                        bannedString.append(QString(" (%1 minute)").arg(expiration));
                    } else {
                        bannedString.append(QString(" (%2 minutes)").arg(expiration));
                    }
                }
            }
        }
        witem = new QTableWidgetItem(q.value(2).toBool() ? bannedString : "Fine");
        mytable->setItem(i, 2, witem);

        witem = new QTableWidgetItem(q.value(3).toString().length() > 0 ? "Yes" : "No");
        mytable->setItem(i, 3, witem);

        witem = new QTableWidgetItem(q.value(4).toString());
        mytable->setItem(i, 4, witem);

        witem = new QTableWidgetItem(q.value(5).toString());
        mytable->setItem(i, 5, witem);

        witem = new QTableWidgetItem(QString::number(expireDays - QDate::fromString(q.value(5).toString(), Qt::ISODate).daysTo(QDate::currentDate())) + " Days");
        mytable->setItem(i, 6, witem);

        i++;
    }

    mytable->sortByColumn(0, Qt::AscendingOrder);

    mytable->setSortingEnabled(true);

    QPushButton *_authority = new QPushButton(tr("&Authority"));
    QMenu *m = new QMenu(_authority);
    m->addAction(tr("User"), this, SLOT(user()));
    m->addAction(tr("Moderator"), this, SLOT(mod()));
    m->addAction(tr("Administrator"), this, SLOT(admin()));
    m->addAction(tr("Owner"), this, SLOT(owner()));
    _authority->setMenu(m);

    QPushButton *_ban = new QPushButton(tr("&Ban"));
    QPushButton *_unban = new QPushButton(tr("U&nban"));
    QPushButton *_clpass = new QPushButton(tr("&Clear Password"));

    mylayout->addWidget(_authority,1,0);
    mylayout->addWidget(_ban,1,2);
    mylayout->addWidget(_unban,1,3);
    mylayout->addWidget(_clpass,1,4);

    if (mytable->rowCount() == 0)
        return;

    connect(_ban,SIGNAL(clicked()),SLOT(ban()));
    connect(_unban,SIGNAL(clicked()),SLOT(unban()));
    connect(_clpass,SIGNAL(clicked()),SLOT(clpass()));
}

QString PlayersWindow::currentName()
{
    if(mytable->rowCount() > 0) {
        return mytable->item(mytable->currentRow(), 0)->text();
    }
    return 0;
}

void PlayersWindow::ban()
{
    SecurityManager::ban(currentName());
    /* Otherwise we may have time from a temp ban before */
    SecurityManager::setBanExpireTime(currentName(), 0);
    mytable->item(mytable->currentRow(), 2)->setText("Banned");
    emit banned(currentName());
}

void PlayersWindow::unban()
{
    SecurityManager::unban(currentName());
    mytable->item(mytable->currentRow(), 2)->setText("Fine");
}

void PlayersWindow::user()
{
    QString name = currentName();
    if(!name.isNull()) {
        SecurityManager::setAuth(name, 0);
        mytable->item(mytable->currentRow(), 1)->setText("User");
        emit authChanged(name,0);
    }
}

void PlayersWindow::mod()
{
    QString name = currentName();
    if(!name.isNull()) {
        SecurityManager::setAuth(name, 1);
        mytable->item(mytable->currentRow(), 1)->setText("Mod");
        emit authChanged(name,1);
    }
}

void PlayersWindow::admin()
{
    QString name = currentName();
    if(!name.isNull()) {
        SecurityManager::setAuth(name, 2);
        mytable->item(mytable->currentRow(), 1)->setText("Admin");
        emit authChanged(name,2);
    }
}

void PlayersWindow::owner()
{
    QString name = currentName();
    if(!name.isNull()) {
        SecurityManager::setAuth(name, 3);
        mytable->item(mytable->currentRow(), 1)->setText("Owner");
        emit authChanged(name,3);
    }
}

void PlayersWindow::clpass()
{
    SecurityManager::clearPass(currentName());
    mytable->item(mytable->currentRow(), 3)->setText("No");
}
