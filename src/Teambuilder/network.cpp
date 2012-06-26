/**
 * See network protocol here: http://wiki.pokemon-online.eu/view/Network_Protocol_v2
*/

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
        if (this->bytesAvailable() < 4) {
            return;
        }
        /* Ok now we can start */
        commandStarted=true;
        /* getting the length of the message */
        char c1, c2, c3, c4;
        this->getChar(&c1), this->getChar(&c2); this->getChar(&c3), this->getChar(&c4);
        remainingLength= (uchar(c1) << 24) + (uchar(c2) << 16) + (uchar(c3) << 8) + uchar(c4);
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
    quint32 length = message.length();
    uchar c1, c2, c3, c4;
    c1 = length & 0xFF;
    length >>= 8;
    c2 = length & 0xFF;
    length >>= 8;
    c3 = length & 0xFF;
    length >>= 8;
    c4 = length & 0xFF;

    this->putChar(c4);
    this->putChar(c3);
    this->putChar(c2);
    this->putChar(c1);
    this->write(message);
}
