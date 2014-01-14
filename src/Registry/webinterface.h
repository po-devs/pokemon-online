#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include "HttpServer.h"
#include "HttpHandler.h"
#include "HttpHandlerSimpleRouter.h"
#include "HttpConnection.h"
#include <QObject>

class Registry;

class RegistryWebInterface : QObject
{
Q_OBJECT

public:

    RegistryWebInterface(Registry *reg);
    virtual ~RegistryWebInterface();

private slots:

    void showServers(Pillow::HttpConnection*);
    void showBans(Pillow::HttpConnection*);
    void updateBans(Pillow::HttpConnection*);
    void showAnnouncement(Pillow::HttpConnection*);
    void updateAnnouncement(Pillow::HttpConnection*);

private:

    QString escapeHtml(QString const&);
    QMap<QString,QString> parsePost(QByteArray const&);

    Registry *regptr;

    Pillow::HttpServer server;
    Pillow::HttpHandlerSimpleRouter router;

    QString pass;
};

#endif
