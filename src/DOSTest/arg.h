#ifndef ARG_H
#define ARG_H

#include <QtCore>
#include <QtNetwork>
#include "../Teambuilder/analyze.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/networkstructs.h"
#include <ctime>
#include <cstdlib>


class IOManager : public QObject
{
    Q_OBJECT
public:
    bool on;

    IOManager();
    ~IOManager();
public slots:
    void connectionEstablished();
    void goodToDelete();
protected:
    void timerEvent(QTimerEvent *t);
signals:
    void disconnected();
public:
    Analyzer *a;
    QBasicTimer t;
private:
    int count;
};

class DosManager : public QObject
{
    Q_OBJECT
public:
    DosManager();
private slots:
    void removeStuff();
protected:
    void timerEvent(QTimerEvent *t);
private:
    QBasicTimer t;

    QSet<IOManager *> bots;
};


#endif // ARG_H
