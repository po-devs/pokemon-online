#include "waitingobject.h"

WaitingObjects * WaitingObjects::instance = new WaitingObjects();
QSet<WaitingObject*> WaitingObjects::freeObjects;

WaitingObject * WaitingObjects::getObject()
{
    if (!freeObjects.isEmpty()) {
        WaitingObject *w = *freeObjects.begin();
        freeObjects.remove(w);
        return w;
    } else {
        return new WaitingObject();
    }
}

void WaitingObjects::freeObject()
{
    freeObject((WaitingObject*)sender());
}

void WaitingObjects::freeObject(WaitingObject *c)
{
    freeObjects.insert(c);
    c->disconnect();
}

WaitingObjects *WaitingObjects::getInstance()
{
    return instance;
}
