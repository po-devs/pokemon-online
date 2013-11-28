#ifndef POKEMONTESTRUNNER_H
#define POKEMONTESTRUNNER_H

#include "testrunner.h"

class PokemonTestRunner : public TestRunner
{
    Q_OBJECT
public:
    explicit PokemonTestRunner(QObject *parent = 0);

    void loadDatabase();
signals:

public slots:

};

#endif // POKEMONTESTRUNNER_H
