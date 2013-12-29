#include "keypresseater.h"
#include <QKeyEvent>


KeyPressEater::KeyPressEater(QObject *parent) :
    QObject(parent)
{
}

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        bool res = QObject::eventFilter(obj, event);

        if (keyEvent->key() == Qt::Key_Return) {
            return true;
        } else {
            return res;
        }
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
