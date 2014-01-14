#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H

template <class T, class U> class QHash;

class QString;
class OnlineClientPlugin;
class QScriptEngine;

class ClientInterface {
public:
    virtual ~ClientInterface(){}
    virtual void printLine(const QString &line)=0;
    virtual void printHtml(const QString &html)=0;
    virtual void printChannelMessage(const QString &mess, int channel, bool html)=0;
    virtual const QHash<qint32, QString>& getChannelNames() const = 0;

    virtual void addPlugin(OnlineClientPlugin *o) = 0;
    virtual void removePlugin(OnlineClientPlugin *o) = 0;
    virtual void registerMetaTypes(QScriptEngine *){}
};

#endif // CLIENTINTERFACE_H
