#ifndef RELAYSTATION_H
#define RELAYSTATION_H

#include <QObject>
#include <QHash>

class QWsServer;
class QWsSocket;
class RegistryStation;

class RelayStation : public QObject
{
    Q_OBJECT
public:
    explicit RelayStation(int port = 10508, QString host = "localhost:5080", QHash<QString,QString> aliases=QHash<QString,QString>(), QObject *parent = 0);
    
    void start();
signals:
    
public slots:
    void onNewConnection();
private:
    int port;
    QString host;
    QHash<QString, QString> _aliases;

    QWsServer *webserver;
    RegistryStation *registry;
};

#endif // RELAYSTATION_H
