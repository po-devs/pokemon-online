#ifndef REGISTRYCOMMUNICATOR_H
#define REGISTRYCOMMUNICATOR_H

#include <QObject>

class Analyzer;

class RegistryCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit RegistryCommunicator(QObject *parent = 0);
    
signals:
    void info(const QString &message);
public slots:
    void connectToRegistry();
    void disconnectFromRegistry();
    void regConnected();
    void regConnectionError();

    void setPrivate(bool serverPrivate);
    void regSendPlayers();
    void nameChange(const QString &name);
    void descChange(const QString &desc);
    void maxChange(int numMax);
    void passChange(bool enabled);

    void accepted();
    void invalidName();
    void nameTaken();
    void ipRefused();
private:
    Analyzer* registry_connection;
    bool serverPrivate;

    bool testConnection();
};

#endif // REGISTRYCOMMUNICATOR_H
