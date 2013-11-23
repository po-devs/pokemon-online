#ifndef ANALYZER_H
#define ANALYZER_H

#include <QObject>

#include "../Utilities/asiosocket.h"

class GenericNetwork;

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(GenericSocket sock, int id);
    
signals:
    
public slots:
    
private:
    GenericNetwork *socket;
};

#endif // ANALYZER_H
