#ifndef SERVER_H
#define SERVER_H

#include <QtCore>

class SocketSQ;

class Server : public QObject
{
    Q_OBJECT
public:
    Server(SocketSQ *serv);
public slots:
    void received();
private:
    SocketSQ * myserver;
};

#endif // SERVER_H
