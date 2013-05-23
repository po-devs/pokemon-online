#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

class PlayerInterface;

class ServerInterface
{
public:
    virtual ~ServerInterface(){}

    virtual PlayerInterface *playeri(int id) const = 0;
    virtual QObject *getAntiDos() const = 0;
};

#endif // SERVERINTERFACE_H
