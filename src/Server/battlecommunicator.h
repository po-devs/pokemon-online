#ifndef BATTLECOMMUNICATOR_H
#define BATTLECOMMUNICATOR_H

#include <QObject>
#include <QString>

class Analyzer;

class BattleCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit BattleCommunicator(QObject *parent = 0);
    
signals:
    void info(QString &message);
public slots:
    void connectToBattleServer();
    void battleConnected();
    void battleConnectionError();
    void error();
private:
    Analyzer* battleserver_connection;
};

#endif // BATTLECOMMUNICATOR_H
