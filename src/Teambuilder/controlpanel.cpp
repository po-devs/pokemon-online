#include "controlpanel.h"
#include "../Utilities/functions.h"
#include "../PokemonInfo/networkstructs.h"

ControlPanel::ControlPanel(int myauth, const UserInfo &ui)
{
    setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose, true);

    if (myauth < 2) {
        ban->hide();
        unban->hide();
        banIP->hide();
    }

    banIP->setDisabled(true);

    setPlayer(ui);

    setCurrentIndex(0);
}

void ControlPanel::setPlayer(const UserInfo &ui)
{
    userName->setText(ui.name);
    aliasName->setText(ui.name);
    authority->setText(authorityText(ui.auth));
    status->setText(statusText(ui));
    ip->setText(ui.ip);
    lastAp->setText(ui.date);

    mute->setDisabled(true);
    if (!ui.online()) {
        kick->setDisabled(true);
        pm->setDisabled(true);
    }
    else {
        kick->setEnabled(true);
        pm->setEnabled(true);
    }
}


QString ControlPanel::authorityText(int auth) const
{
    if (auth == 0) {
        return tr("User");
    } else if (auth == 1){
        return tr("Moderator");
    } else if (auth == 2) {
        return tr("Administrator");
    } else if (auth == 3) {
        return tr("Owner");
    } else {
        return "";
    }
}

QString ControlPanel::statusText(const UserInfo &ui) const
{
    QString ret;

    if (!ui.exists()) {
        ret = "";
    } else if (ui.banned()) {
        ret = toBoldColor(tr("Banned"), Qt::red);
    } else if (ui.tempBanned()) {
        ret = toBoldColor(tr("Tempbanned"), QColor("orange"));
    } else if (ui.online()) {
        ret = toBoldColor(tr("Online"), Qt::green);
    } else {
        ret = tr("Offline");
    }

    if (ui.muted()) {
        ret += " ";
        ret += toBoldColor(tr("[Muted]"), Qt::red);
    }

    return ret;
}

void ControlPanel::addAlias(const QString &name)
{
    aliasList->addItem(name);
}

void ControlPanel::addNameToBanList(const QString &name, const QString &ip, const QDateTime& expires)
{
    int rowcount = banTable->rowCount();
    banTable->setRowCount(rowcount+1);
    banTable->setItem(rowcount, 0, new QTableWidgetItem(name));
    banTable->setItem(rowcount, 1, new QTableWidgetItem(ip));
    if (expires.toTime_t() == 0) {
        banTable->setItem(rowcount, 2, new QTableWidgetItem(tr("Never")));
	} else {
        banTable->setItem(rowcount, 2, new QTableWidgetItem(expires.toLocalTime().toString("yyyy.MM.dd hh:mm:ss")));
	}
}

void ControlPanel::on_unban_clicked()
{
    int row = banTable->currentRow();

    if (row == -1 || row >= banTable->rowCount()) {
        return;
    }

    emit unbanRequested(banTable->item(row, 0)->text());
    banTable->removeRow(row);
}
