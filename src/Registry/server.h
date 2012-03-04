#ifndef SERVER_H
#define SERVER_H

#include <QtCore>
#include "macro.h"

class QTcpSocket;
class Analyzer;
class QNickValidator;

class Server : public QObject
{
    Q_OBJECT
    PROPERTY(int, id);
    PROPERTY(QString, ip);
    PROPERTY(QString, name);
    PROPERTY(QString, desc);
    PROPERTY(quint16, players);
    PROPERTY(quint16, maxPlayers)
    PROPERTY(quint16, port)
    PROPERTY(bool, passwordProtected)
    PROPERTY(bool, listed);
public:
    Server(int id, QTcpSocket *s);

    void refuseIP();
    void refuseName();
    void accept();
    void kick();
    QString getAddress(int port) const;
public slots:
    void login(const QString &, const QString &, quint16, quint16,quint16, bool);
    void numChanged(quint16);
    void nameChanged(const QString &);
    void descChanged(const QString &);
    void maxChanged(const quint16);
    void passToggled(bool);
    void disconnected();
signals:
    void nameChangedReq(int id, const QString &name);
    void portSet(int id, int port, int oldport);
    void disconnection(int id);
private:
    Analyzer *m_relay;
};

#endif // SERVER_H
