#ifndef SERVER_H
#define SERVER_H

#include <QtCore>
#include "../Utilities/functions.h"

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
    PROPERTY(bool, listed);
public:
    Server(int id, QTcpSocket *s);

    void refuseIP();
    void refuseName();
    void accept();
    void kick();
public slots:
    void login(const QString &, const QString &, quint16);
    void login(const QString &, const QString &, quint16, quint16);
    void numChanged(quint16);
    void nameChanged(const QString &);
    void descChanged(const QString &);
    void maxChanged(const quint16);
    void disconnected();
signals:
    void nameChangedReq(int id, const QString &name);
    void disconnection(int id);
private:
    QNickValidator *m_validator;
    Analyzer *m_relay;
};

#endif // SERVER_H
