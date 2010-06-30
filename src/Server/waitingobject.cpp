#include "waitingobject.h"

WaitingObjects * WaitingObjects::instance = new WaitingObjects();
QSet<WaitingObject*> WaitingObjects::freeObjects;
QSet<WaitingObject*> WaitingObjects::usedObjects;

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
    if (!sender())
        return;
    freeObject((WaitingObject*)sender());
}

void WaitingObjects::freeObject(WaitingObject *c)
{
    usedObjects.remove(c);
    freeObjects.insert(c);
}

void WaitingObjects::useObject(WaitingObject *c)
{
    freeObjects.remove(c);
    usedObjects.insert(c);
}

WaitingObjects *WaitingObjects::getInstance()
{
    return instance;
}
