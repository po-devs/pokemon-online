#ifndef WAITINGOBJECT_H
#define WAITINGOBJECT_H

#include <QtCore>

class WaitingObject : public QObject
{
    Q_OBJECT
public:
    void connectToSlot(QObject *o, const char *slot) {
        receiver = o;
        this->slot = slot;
        connect(this, SIGNAL(waitFinished()), o, slot);
    }
    void emitSignal() {
        emit waitFinished();
        disconnect();
    }
signals:
    void waitFinished();
private:
    QObject *receiver;
    const char *slot;
};

#endif // WAITINGOBJECT_H
