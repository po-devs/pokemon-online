#include <Utilities/functions.h>
#include "webinterface.h"
#include "registry.h"
#include "server.h"

RegistryWebInterface::RegistryWebInterface(Registry* reg) : QObject(reg), regptr(reg), server()
{
    pass = QString::fromUtf8(getFileContent("registry_pass.txt").trimmed());
    if (pass.length() > 0) {
        pass.insert(0, '/');
    }

    if (server.listen(QHostAddress::Any, 4567))
    {
        qDebug() << "Listening to port 4567";
    }
    router.setUnmatchedRequestAction(Pillow::HttpHandlerSimpleRouter::Return4xxResponse);
    router.addRoute("GET", pass + "/index", this, SLOT(showServers(Pillow::HttpConnection*)));
    router.addRoute("GET", pass + "/servers", this, SLOT(showServers(Pillow::HttpConnection*)));
    router.addRoute("GET", pass + "/bans", this, SLOT(showBans(Pillow::HttpConnection*)));
    router.addRoute("POST", pass + "/updatebans", this, SLOT(updateBans(Pillow::HttpConnection*)));
    router.addRoute("GET", pass + "/announcement", this, SLOT(showAnnouncement(Pillow::HttpConnection*)));
    router.addRoute("POST", pass + "/updateannouncement", this, SLOT(updateAnnouncement(Pillow::HttpConnection*)));
    QObject::connect(&server, SIGNAL(requestReady(Pillow::HttpConnection*)), &router, SLOT(handleRequest(Pillow::HttpConnection*)));
}

RegistryWebInterface::~RegistryWebInterface() {}

namespace {
    QByteArray const header = QString(
        "<html>"
        "<head>"
        "<title>Pokemon Online Registry</title>"
        "</head>"
        "<body>"
        "<a href='./index'>Server listing</a> "
        "| <a href='./announcement'>Announcement</a> "
        "| <a href='./bans'> Registry bans</a>"
        "<hr>").toUtf8();
    QByteArray const footer = QString("</ul></body></html>").toUtf8();
}

void RegistryWebInterface::showServers(Pillow::HttpConnection *conn)
{
    conn->writeHeaders(200);
    conn->writeContent(header);
    QString const serverInfo = "<li>Server IP: <b>%1</b>, Server name: <b>%2</b>, Players: <b>%4/%5</b>, Description: <center>%3</center></li>";
    conn->writeContent(QByteArray("<h1>Servers</h1><ul>"));
    foreach (Server* server, regptr->servers) {
        conn->writeContent(serverInfo
            .arg(server->ip())
            .arg(escapeHtml(server->name()))
            .arg(server->desc())
            .arg(server->players())
            .arg(server->maxPlayers()).toUtf8()
    );
    }
    conn->writeContent(QByteArray("</ul>"));
    conn->writeContent(footer);
    conn->endContent();
}
void RegistryWebInterface::showBans(Pillow::HttpConnection *conn)
{
    conn->writeHeaders(200);
    conn->writeContent(header);
    conn->writeContent(QString("<h1>Registry bans</h1><h2>Web bans</h2><ul>").toUtf8());
    QString const ipline = "<li>IP: %1</li>";
    foreach (QString const &ip, regptr->bannedIPs) {
    conn->writeContent(ipline.arg(ip).toUtf8());

    }
    conn->writeContent(QString("</ul><h2>From network banlists</h2><ul>").toUtf8());
    foreach (QString const &ip, regptr->tbanIPs) {
    if (ip.size() > 0)
        conn->writeContent(ipline.arg(ip).toUtf8());
    }
    conn->writeContent(QString("</ul><form method='POST' enctype='application/x-www-form-urlencoded' action='./updatebans'><h2>Modify bans</h2><br><label for='add'>Add IP ban</label><input type='text' name='add'><br><label for='remove'>Remove IP ban</label><input type='text' name='remove'><br><input type='submit'></form>").toUtf8());
    conn->writeContent(footer);
    conn->endContent();
}
void RegistryWebInterface::updateBans(Pillow::HttpConnection *conn)
{
    QMap<QString,QString> post = parsePost(conn->requestContent());
    if (post.contains("remove"))
        regptr->bannedIPs.remove(post["remove"]);
    if (post.contains("add") && post["add"].size() > 0)
        regptr->bannedIPs.insert(post["add"]);

    Pillow::HttpHeaderCollection headers;
    headers.push_back(qMakePair(QByteArray("Location"), QByteArray("./bans?updated=true")));
    conn->writeResponse(303, headers);
}
void RegistryWebInterface::showAnnouncement(Pillow::HttpConnection *conn)
{
    conn->writeHeaders(200);
    conn->writeContent(header);
    conn->writeContent(QString("<h1>Registry announcement</h1><div style='border 2px coral solid' id='announcement'><center>%1</center></div><hr><form method='POST' action='./updateannouncement' enctype='application/x-www-form-urlencoded'><label for='announcement'>Update announcement:<br></label><textarea name='announcement'>%1</textarea><input type='submit'></form>").arg(regptr->registry_announcement).toUtf8());
    conn->writeContent(footer);
    conn->endContent();
}
void RegistryWebInterface::updateAnnouncement(Pillow::HttpConnection *conn)
{
    QMap<QString,QString> post = parsePost(conn->requestContent());
    if (post.contains("announcement")) {
        regptr->registry_announcement = post["announcement"];
        QFile file("announcement.txt");
            if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << regptr->registry_announcement;
            file.close();
        }
    }

    Pillow::HttpHeaderCollection headers;
    headers.push_back(qMakePair(QByteArray("Location"), QByteArray("./announcement?updated=true")));
    conn->writeResponse(303, headers);
}

QString RegistryWebInterface::escapeHtml(QString const& in)
{
    return QString(in).replace("&","&amp;").replace(">","&gt;").replace("<","&lt;");
}

QMap<QString,QString> RegistryWebInterface::parsePost(QByteArray const& in)
{
    QMap<QString,QString> POST;
    QByteArray decoded = in;
    QList<QByteArray> key_vals = decoded.replace('+', ' ').split('&');
    foreach(const QByteArray key_eq_val, key_vals) {
        const QList<QByteArray> key_val = key_eq_val.split('=');
    const QString key = QUrl::fromPercentEncoding(key_val[0]);
    const QString value = key_val.size() > 0 ? QUrl::fromPercentEncoding(key_val[1]) : "";
    POST[key] = value;
    }
    return POST;
}
