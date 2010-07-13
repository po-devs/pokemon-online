#ifndef WAITINGOBJECT_H
#define WAITINGOBJECT_H

#include <QtCore>

class WaitingObject : public QObject
{
    Q_OBJECT
public:
    void emitSignal() {
        emit waitFinished();
    }

    QVariantHash data;
signals:
    void waitFinished();
};

class WaitingObjects : public QObject
{
    Q_OBJECT

    friend class ScriptEngine;
public:
    static WaitingObject* getObject();
    static void freeObject(WaitingObject *c);
    static WaitingObjects * getInstance();

public slots:
    /* Must be called with a waiting object to free as sender(), or segfault */
    void freeObject();
private:
    /* Use publicly as read-only to know how many WaitingObject instances have been allocated in total */
    static int objectCount;

    static WaitingObjects *instance;

    static QSet<WaitingObject*> freeObjects;
};

#endif // WAITINGOBJECT_H
