#ifndef PLAYER_H
#define PLAYER_H

#include <QtCore>
#include "../Utilities/functions.h"

class QTcpSocket;
class Analyzer;
class Server;

class Player : public QObject
{
    Q_OBJECT
    PROPERTY(int, id);
    PROPERTY(QString, ip);
    PROPERTY(bool, listed);
public:
    Player(int id, QTcpSocket *s);

    void sendServer(const Server &s);
    void kick();
public slots:
    void disconnected();
signals:
    void disconnection(int id);
private:
    Analyzer *m_relay;
};

#endif // PLAYER_H
