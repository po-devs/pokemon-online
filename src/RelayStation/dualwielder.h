#ifndef DUALWIELDER_H
#define DUALWIELDER_H

#include <QObject>

class QWsSocket;
class Network;

class DualWielder : public QObject
{
    Q_OBJECT
public:
    DualWielder(QObject *parent = NULL);
    ~DualWielder();

    void init(QWsSocket *web, QString host="localhost:5080");

    QString ip() const;
public slots:
    void readWebSocket(const QString&);
    void socketConnected();
    void socketDisconnected();
    void webSocketDisconnected();
private:
    QWsSocket *web;
    Network *network;
    QString mIp;
};

#endif // DUALWIELDER_H
