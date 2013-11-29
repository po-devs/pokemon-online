/* For UnknownCommand */
#include "../Shared/networkcommands.h"

#include "baseanalyzer.h"

BaseAnalyzer::~BaseAnalyzer()
{
    blockSignals(true);
    /* Very important feature. If you don't do this it might crash.
        this makes the stillValid of Network redundant, but still.*/
    close();
}

void BaseAnalyzer::connectTo(const QString &host, quint16 port)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    socket().connectToHost(host, port);
}

void BaseAnalyzer::close() {
    if (dummy) {return;}
    socket().close();
}

QString BaseAnalyzer::ip() const {
    if (dummy) {return QString("dummyip");}
    return socket().ip();
}

void BaseAnalyzer::changeIP(const QString &ip) {
    if (dummy) return;
    socket().changeIP(ip);
}

void BaseAnalyzer::setLowDelay(bool lowDelay)
{
    if (dummy) {return;}
    socket().setLowDelay(lowDelay);
}

void BaseAnalyzer::error()
{
    if (dummy) {return;}
    emit connectionError(socket().error(), socket().errorString());
}

bool BaseAnalyzer::isConnected() const
{
    if (dummy) {return false;}
    return socket().isConnected();
}

void BaseAnalyzer::stopReceiving()
{
    if (dummy) {return;}
    blockSignals(true);
    socket().close();
}

void BaseAnalyzer::dealWithCommand(const QByteArray &commandline)
{
    DataStream in (commandline);
    uchar command;

    in >> command;

    switch (command) {
    /* case Xxx: doSomething(), read from in */
    default:
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received"));
        break;
    }
}

void BaseAnalyzer::commandReceived(const QByteArray &command)
{
    if (delayCount > 0) {
        delayedCommands.push_back(command);
    } else {
        dealWithCommand(command);
    }
}

void BaseAnalyzer::delay()
{
    delayCount += 1;
}

void BaseAnalyzer::swapIds(BaseAnalyzer *other)
{
    int id = other->socket().id();
    other->socket().changeId(socket().id());
    socket().changeId(id);
}

void BaseAnalyzer::setId(int id)
{
    socket().changeId(id);
}

void BaseAnalyzer::sendPacket(const QByteArray &packet)
{
    emit packetToSend(packet);
}

void BaseAnalyzer::undelay()
{
    delayCount -=1;

    while(delayedCommands.size() > 0 && delayCount==0) {
        dealWithCommand(delayedCommands.takeFirst());
    }
}

GenericNetwork & BaseAnalyzer::socket()
{
    return *mysocket;
}

const GenericNetwork & BaseAnalyzer::socket() const
{
    return *mysocket;
}
