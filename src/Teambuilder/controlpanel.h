#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtGui>
#include "ui_controlpanel.h"

struct UserInfo;

class ControlPanel : public QTabWidget, Ui_ControlPanel
{
    Q_OBJECT
public:
    ControlPanel(int myauth, const UserInfo &ui);

    QString authorityText(int auth) const;
    QString statusText(const UserInfo &ui) const;

    QString playerName() const { return userName->text();}
public slots:
    void setPlayer(const UserInfo &ui);
    void addAlias(const QString &name);
    void addNameToBanList(const QString &name, const QString &ip, const QDateTime& time);
signals:
    void getUserInfo(const QString &name);
    void getBanList();
    void banRequested(const QString &name);
    void unbanRequested(const QString &name);
    void tempBanRequested(const QString &name,const int &time);
    void pmcp(const QString &p);
private slots:
    void getUser() {
        aliasList->clear();
    }

    void on_searchUser_clicked() {
        getUser();
        emit getUserInfo(playerName());
    }
    void on_userName_returnPressed() {
        getUser();
        emit getUserInfo(playerName());
    }
    void on_searchAlias_clicked() {
        getUser();
        emit getUserInfo(aliasName->text());
    }
    void on_aliasName_returnPressed() {
        getUser();
        emit getUserInfo(aliasName->text());
    }
    void on_aliasList_itemActivated(QListWidgetItem *it) {
        aliasName->setText(it->text());
    }
    void on_refresh_clicked() {
        banTable->setRowCount(0);
        emit getBanList();
    }
    void on_unban_clicked();
    void on_ban_clicked() {
        emit banRequested(userName->text());
    }
    void on_tBan_clicked() {
        emit tempBanRequested(userName->text(),time->value());
    }
    void on_pm_clicked() {
        emit pmcp(userName->text());
    }

    void on_banTable_clicked(const QModelIndex& index);
    void on_updateBantime_clicked();
};

#endif // CONTROLPANEL_H
