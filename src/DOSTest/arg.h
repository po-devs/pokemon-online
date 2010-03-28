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
    IOManager() {
        qDebug() << "Connecting to localhost";

        connect(&a, SIGNAL(connected()), SLOT(connectionEstablished()));
        a.connectTo("127.0.0.1", 5080);
    }
public slots:
    void connectionEstablished();

public:
    Analyzer a;
};

#endif // ARG_H
