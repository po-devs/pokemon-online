#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include "centralwidget.h"
#include <QtNetwork>

struct ServerInfo;
class TeamHolder;
class Analyzer;

namespace Ui {
class ServerChoice;
}

class ServerChoice : public QFrame, public CentralWidgetInterface
{
    Q_OBJECT

public:
    ServerChoice(TeamHolder *team);
    ~ServerChoice();

    void saveSettings();

    QMenuBar *createMenuBar(MainEngine *);
signals:
    void serverChosen(const QString &ip, const quint16 port, const QString &nick);
    void teambuilder();
    void clearList();
protected:
    bool event(QEvent *e);
public slots:
    void loadTeam();
    void loadAll(const TeamHolder&);
private slots:
    void serverAdded();
    void showDetails(const QModelIndex&);
    void regServerChosen(const QModelIndex&);
    void advServerChosen();
    void connectionError(int , const QString &mess);
    void connected();
    void anchorClicked(const QUrl&);
    void timeout();
    void announcementReceived(QNetworkReply* reply);

    void on_switchPort_clicked();
private:
    Ui::ServerChoice *ui;

    Analyzer *registry_connection;
    QSortFilterProxyModel *filter;

    QList<QStringList> savedServers;

    bool wasConnected;
    TeamHolder *team;
    QNetworkAccessManager manager;
    void addSavedServer(const QString &ip, const QString &name="");
    void getAnnouncement();
};

#endif // SERVERCHOICE_H
