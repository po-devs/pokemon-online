#include "network.h"


Network::Network(QTcpSocket *sock) : mysocket(sock), commandStarted(false)
{
    connect(socket(), SIGNAL(readyRead()), this, SLOT(onReceipt()));
}

void Network::onReceipt()
{
    if (commandStarted == false) {
	/* There it's a new message we are receiving.
	   To start receiving it we must know its length, i.e. the 2 first bytes */
	if (socket()->bytesAvailable() < 2) {
	    return;
	}
	/* Ok now we can start */
	commandStarted=true;
	/* getting the length of the message */
	char c1, c2;
	socket()->getChar(&c1), socket()->getChar(&c2);
	remainingLength=c1*256+c2;
	/* Recursive call to write less code =) */
	onReceipt();
    } else {
	/* Checking if the command is complete! */
	if (socket()->bytesAvailable() >= remainingLength) {
	    emit isFull(socket()->read(remainingLength));
	    commandStarted = false;
	    /* Recursive call to spare code =), there may be still data pending */
	    onReceipt();
	}
    }
}

int Network::error() const
{
    return socket()->error();
}

QString Network::errorString() const
{
    return socket()->errorString();
}

void Network::send(const QByteArray &message)
{
    socket()->putChar(message.length()/256);
    socket()->putChar(message.length()%256);
    socket()->write(message);
}

QTcpSocket * Network::socket()
{
    return mysocket;
}

const QTcpSocket * Network::socket() const
{
    return mysocket;
}
