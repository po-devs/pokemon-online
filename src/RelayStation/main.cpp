#include <QtCore/QCoreApplication>
#include <QHash>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "relaystation.h"

#define PRINTOPT(a, b) (fprintf(stdout, "  %-25s\t%s\n", a, b))

int main(int argc, char *argv[])
{
    QString host = "localhost:5080";
    int port = 10508;
    QHash<QString, QString> aliases;

    for(int i = 0; i < argc; i++) {
        if(strcmp( argv[i], "-h") == 0 || strcmp( argv[i], "--help") == 0){
            fprintf(stdout, "Relay Station for Pokemon Online Help\n");
            fprintf(stdout, "Please visit http://www.pokemon-online.eu/ for more information.\n");
            fprintf(stdout, "Don't forget to add the IP of the relay station to the proxy servers and the trusted IPs of your server.\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "Usage: ./RelayStation [[options]]\n");
            fprintf(stdout, "Options:\n");
            PRINTOPT("-d, --default IP:port", "Sets the default target server. Default is localhost:5080.");
            PRINTOPT("-a, --alias IP1=IP2", "Sets an IP alias. People connecting to IP1 will instead connect to IP2. It's a good idea to do -a <publicIP>=localhost");
            PRINTOPT("-h, --help", "Displays this help.");
            PRINTOPT("-p, --port [PORT]", "Sets the relay station port.");
            fprintf(stdout, "\n");
            return 0;   //exit app
        } else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0){
            if (++i == argc){
                fprintf(stderr, "No relay port provided.\n");
                return 1;
            }
            port = atoi(argv[i]);
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--default") == 0) {
            if (++i == argc){
                fprintf(stderr, "No host provided.\n");
                return 1;
            }
            host = argv[i];
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--alias") == 0) {
            if (++i == argc){
                fprintf(stderr, "No alias provided.\n");
                return 1;
            }
            aliases[QString(argv[i]).section("=", 0, 0)] = QString(argv[i]).section("=", 1);
        }
    }

    srand(time(NULL));

    QCoreApplication a(argc, argv);

    fprintf(stdout, "Relay Station for Pokemon Online, use --help to get the help.\n\n");

    RelayStation station(port, host, aliases);
    station.start();
    
    return a.exec();
}
