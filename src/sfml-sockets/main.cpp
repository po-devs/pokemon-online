#include <QtCore/QCoreApplication>
#include "sfmlsocket.h"
#include "server.h"

/** Usage:
      ./sfml-sockets port=[port] host=[host]
      If port is not specified, 5061 is assumed.
      If no host is specifed, acts as a server.

      Example: ./sfml-sockets port=5000 host=88.88.88.88
*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    quint16 port = 5061;
    std::string host;

    for(int i = 1; i < argc; i++) {
        QStringList args = QString(argv[i]).split("=");

        if (args.size() == 2) {
            if (args[0] == "port") {
                port = args[1].toInt();
            } else if (args[0] == "host") {
                host = args[1].toStdString();
            }
        }
    }

    SocketManager manager;
    manager.Launch();

    if (host.length() == 0) {
        qDebug() << "Running a server on port " << port;

        GenericSocket *s = manager.createSocket();
        if (!s->listen(port)) {
            qDebug() << "Listening to port failed.";
            return 0;
        }

        Server server(s);
        (void) server;
    } else {
        qDebug() << "Running a client connecting to " << host.c_str() << ":" << port;

        GenericSocket *s = manager.createSocket();
        if (!s->connectTo(port, host)) {
            qDebug() << "Connecting failed";
            return 0;
        }

        QByteArray data;

        for (int i = 0; i < 256000; i++) {
            data.push_back(char(i/1000));
        }

        s->write(data);
    }

    return a.exec();
}
