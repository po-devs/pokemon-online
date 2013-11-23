#ifndef BATTLESERVER_H
#define BATTLESERVER_H

#include <QObject>

class BattleServer : public QObject
{
    Q_OBJECT
public:
    explicit BattleServer(QObject *parent = 0);
    
    void start();
signals:
    
public slots:
    
};

#endif // BATTLESERVER_H
