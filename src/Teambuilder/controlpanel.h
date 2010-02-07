#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtGui>
#include "ui_controlpanel.h"

class UserInfo;

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
    void addNameToBanList(const QString &name, const QString &ip);
signals:
    void getUserInfo(const QString &name);
    void getBanList();
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
        getUser();
        emit getUserInfo(it->text());
    }
    void on_refresh_clicked() {
        banTable->setRowCount(0);
        emit getBanList();
    }
};

#endif // CONTROLPANEL_H
