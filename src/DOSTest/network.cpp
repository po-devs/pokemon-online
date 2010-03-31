#include "network.h"

Network::Network(const QString &host, quint16 port) : commandStarted(false)
{
    if (port == 0) {
        //Default argument, not worth going further
    } else {
        connectToHost(host, port);
    }

    connect(this, SIGNAL(readyRead()), SLOT(onReceipt()));
}

void Network::onReceipt()
{
    if (commandStarted == false) {
        /* There it's a new message we are receiving.
           To start receiving it we must know its length, i.e. the 2 first bytes */
        if (this->bytesAvailable() < 2) {
            return;
        }
        /* Ok now we can start */
        commandStarted=true;
        /* getting the length of the message */
        char c1, c2;
        this->getChar(&c1), this->getChar(&c2);
	remainingLength= uchar(c1)*256+uchar(c2);
        /* Recursive call to write less code =) */
        onReceipt();
    } else {
        /* Checking if the command is complete! */
        if (this->bytesAvailable() >= remainingLength) {
            emit isFull(read(remainingLength));
            commandStarted = false;
            /* Recursive call to spare code =), there may be still data pending */
            onReceipt();
        }
    }
}

void Network::send(const QByteArray &message)
{
    this->putChar(message.length()/256);
    this->putChar(message.length()%256);
    this->write(message);
}
