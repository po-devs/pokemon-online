#include "network.h"


Network::Network(QTcpSocket *sock) : mysocket(sock), commandStarted(false)
{
    connect(socket(), SIGNAL(readyRead()), this, SLOT(onReceipt()));
    connect(socket(), SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(socket(), SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(socket(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(manageError(QAbstractSocket::SocketError)));
    /* SO THE SOCKET IS SAFELY DELETED LATER WHEN DISCONNECTED! */
    connect(socket(), SIGNAL(disconnected()), socket(), SLOT(deleteLater()));
}

void Network::manageError(QAbstractSocket::SocketError err)
{
    qDebug() << "Error received: " << err;
    if (socket()) {
	qDebug() << "Error string: " << socket()->errorString();
    }
    myerror = err;
}

void Network::close() {
    socket()->disconnectFromHost();
    mysocket = NULL;
}

Network::~Network()
{
    if (isConnected()) {
        close();
    }
}

bool Network::isConnected() const
{
    if (socket())
	return socket()->isOpen();
    else
	return false;
}

QString Network::ip() const {
    return socket()->peerAddress().toString();
}

void Network::onDisconnect()
{
    mysocket = NULL;
}

void Network::onReceipt()
{
    if (socket())
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
	    remainingLength=uchar(c1)*256+uchar(c2);
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
}

int Network::error() const
{
    if (socket())
	return socket()->error();
    else
	return myerror;
}

QString Network::errorString() const
{
    if (socket())
	return socket()->errorString();
    else
	return myerrorString;
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
