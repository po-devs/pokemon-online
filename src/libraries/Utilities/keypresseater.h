#ifndef KEYPRESSEATER_H
#define KEYPRESSEATER_H

#include <QObject>

class KeyPressEater : public QObject
{
    Q_OBJECT
public:
    explicit KeyPressEater(QObject *parent = 0);

signals:

public slots:

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // KEYPRESSEATER_H
