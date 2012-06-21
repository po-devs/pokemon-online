#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H



class RegistryWebInterface
{
    RegistryWebInterface(Registry *reg);

private slots:

    void handleRequest();

};

#endif
