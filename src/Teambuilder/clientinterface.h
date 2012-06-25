#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H

class QString;
class OnlineClientPlugin;

class ClientInterface {
public:
    virtual ~ClientInterface(){}
    virtual void printLine(const QString &line)=0;
    virtual void printHtml(const QString &html)=0;
    virtual void printChannelMessage(const QString &mess, int channel, bool html)=0;

    virtual void addPlugin(OnlineClientPlugin *o) = 0;
    virtual void removePlugin(OnlineClientPlugin *o) = 0;
};

#endif // CLIENTINTERFACE_H
