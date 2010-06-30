#ifndef WAITINGOBJECT_H
#define WAITINGOBJECT_H

#include <QtCore>

class WaitingObject : public QObject
{
    Q_OBJECT
public:
    void emitSignal() {
        emit waitFinished();
        disconnect();
    }
signals:
    void waitFinished();
};

class WaitingObjects : public QObject
{
    Q_OBJECT
public:
    static WaitingObject* getObject();
    static void freeObject(WaitingObject *c);
    static void useObject(WaitingObject *c);
    static WaitingObjects * getInstance();

public slots:
    /* Must be called with a waiting object to free as sender(), or segfault */
    void freeObject();
private:
    static WaitingObjects *instance;

    static QSet<WaitingObject*> freeObjects;
    static QSet<WaitingObject*> usedObjects;
};

#endif // WAITINGOBJECT_H
