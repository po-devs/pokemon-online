#include <QTimer>
#include "analyze.h"
#include "battlecommunicator.h"

BattleCommunicator::BattleCommunicator(QObject *parent) :
    QObject(parent), battleserver_connection(nullptr)
{
    system("./BattleServer");

    QTimer::singleShot(5000, this, SLOT(connectToBattleServer()));
}

void BattleCommunicator::connectToBattleServer()
{
    if (battleserver_connection) {
        if (battleserver_connection->isConnected()) {
            return;
        }
        else
            battleserver_connection->deleteLater();
    }

    battleserver_connection = nullptr;

    emit info("Connecting to battle server on port 5096...");

    QTcpSocket * s = new QTcpSocket(nullptr);
    s->connectToHost("localhost", 5096);

    connect(s, SIGNAL(connected()), this, SLOT(battleConnected()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(battleConnectionError()));

    battleserver_connection = new Analyzer(s,0);
}

void BattleCommunicator::battleConnected()
{
    emit info("Connected to battle server!");
}

void BattleCommunicator::battleConnectionError()
{
    emit info("Error when connecting to the battle server. Will try again in 10 seconds");
    emit error();

    QTimer::singleShot(10000, this, SLOT(connectToBattleServer()));
}
