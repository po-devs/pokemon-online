#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include "centralwidget.h"

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
public slots:
    void addServer(const QString &name, const QString &desc, quint16 num, const QString &ip, quint16 max, quint16 port, bool passwordProtected);
signals:
    void serverChosen(const QString &ip, const quint16 port, const QString &nick);
    void rejected();
private slots:
    void showDetails(int row);
    void regServerChosen(int row);
    void advServerChosen();
    void connectionError(int , const QString &mess);
    void connected();
    void anchorClicked(const QUrl&);
    void timeout();

    void on_switchPort_clicked();
private:
    Ui::ServerChoice *ui;

    Analyzer *registry_connection;

    QHash<QString, QString> descriptionsPerIp;
    QList<QStringList> savedServers;

    bool wasConnected;

    void addSavedServer(const QString &ip, const QString &name="");
};

#endif // SERVERCHOICE_H
