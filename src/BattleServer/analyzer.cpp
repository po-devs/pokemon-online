#include "../Utilities/network.h"

#include "analyzer.h"

Analyzer::Analyzer(GenericSocket sock, int id) : socket(new Network<GenericSocket>(sock, id))
{
    connect(this, SIGNAL(sendCommand(QByteArray)), socket, SLOT(send(QByteArray)));
}
