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

#endif // WAITINGOBJECT_H
