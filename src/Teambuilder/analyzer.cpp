#include "analyzer.h"

void Analyzer::push()
{
}

void Analyzer::pull()
{
}

void Analyzer::closeConnection(int id)
{
    (void) id;
}

void Analyzer::brokenConnection(int id)
{
    (void) id;
}

void Analyzer::sendCommand(int id, quint8 command, const QByteArray &data)
{
    (void) id;
    (void) command;
    (void) data;
}

void Analyzer::dataReceived(int id, const QByteArray &data)
{
    (void) id;
    (void) data;
}
