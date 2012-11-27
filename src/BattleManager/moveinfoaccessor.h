#ifndef MOVEINFOACCESSOR_H
#define MOVEINFOACCESSOR_H

#include <QObject>
#include <QString>

#include "../PokemonInfo/geninfo.h"

class MoveInfoAccessor : public QObject
{
    Q_OBJECT
public:
    explicit MoveInfoAccessor(QObject *parent = 0, Pokemon::gen gen = Pokemon::gen());
    
    Q_INVOKABLE QString name(int move);
    Q_INVOKABLE int type(int move);
    Q_INVOKABLE int power(int move);
signals:
    
public slots:
    
private:
    Pokemon::gen mGen;
};

#endif // MOVEINFOACCESSOR_H
