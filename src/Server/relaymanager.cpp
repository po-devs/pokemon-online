#include <QTimer>
#include "relaymanager.h"
#include "analyze.h"

RelayManager* RelayManager::inst;

void RelayManager::init()
{
    inst = new RelayManager();
}

void RelayManager::destroy()
{
    delete inst, inst = NULL;
}

RelayManager * RelayManager::obj()
{
    return inst;
}

RelayManager::~RelayManager()
{

}

RelayManager::RelayManager()
{
    dummy = new Analyzer(new QTcpSocket(this), 0, true);
    connect(this, SIGNAL(destroyed()), dummy, SLOT(deleteLater()));
    QTimer *timer = new QTimer(this);
    timer->setInterval(300*1000); //long interval, but who cares, we don't want broken pipes

    connect(timer, SIGNAL(timeout()), SLOT(cleanTrash()));
    timer->start();
}

void RelayManager::cleanTrash()
{
    foreach(Analyzer* relay, trash) {
        relay->deleteLater();
    }
    trash = recentTrash;
    recentTrash.clear();
}

void RelayManager::addTrash(Analyzer *relay, QObject *obj)
{
    if (relay->dummy) {
        qCritical() << "Dummy relay added to trash";
        //abort();
    }
    if (obj) {
        relay->disconnect(obj);
        obj->disconnect(relay);
    }
    relay->close();
    recentTrash.insert(relay);
}

Analyzer* RelayManager::dummyRelay()
{
    return dummy;
}

const Analyzer* RelayManager::dummyRelay() const
{
    return dummy;
}
