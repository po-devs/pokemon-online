#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include <QtGui>
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
    void advMenuServerChosen(QAction *action);
    void connectionError(int , const QString &mess);
private:
    Ui::ServerChoice *ui;

    Analyzer *registry_connection;

    QHash<QString, QString> descriptionsPerIp;
    QStringList savedServers;
};

#endif // SERVERCHOICE_H
