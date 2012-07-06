#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include "centralwidget.h"

class ServerInfo;
class Analyzer;

namespace Ui {
class ServerChoice;
}

class ServerChoice : public QFrame, public CentralWidgetInterface
{
    Q_OBJECT

public:
    ServerChoice(const QString &nick);
    ~ServerChoice();

    void saveSettings();

    QMenuBar *createMenuBar(MainEngine *);
signals:
    void serverChosen(const QString &ip, const quint16 port, const QString &nick);
    void rejected();
    void clearList();
private slots:
    void serverAdded();
    void showDetails(const QModelIndex&);
    void regServerChosen(const QModelIndex&);
    void advServerChosen();
    void connectionError(int , const QString &mess);
    void connected();
    void anchorClicked(const QUrl&);
    void timeout();

    void on_switchPort_clicked();
private:
    Ui::ServerChoice *ui;

    Analyzer *registry_connection;
    QSortFilterProxyModel *filter;

    QList<QStringList> savedServers;

    bool wasConnected;

    void addSavedServer(const QString &ip, const QString &name="");
};

#endif // SERVERCHOICE_H
