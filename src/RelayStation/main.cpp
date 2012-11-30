#include <QtCore/QCoreApplication>
#include <cstdio>
#include <ctime>
#include "relaystation.h"

#define PRINTOPT(a, b) (fprintf(stdout, "  %-25s\t%s\n", a, b))

int main(int argc, char *argv[])
{
    QString host = "localhost:5080";
    int port = 10508;

    for(int i = 0; i < argc; i++) {
        if(strcmp( argv[i], "-h") == 0 || strcmp( argv[i], "--help") == 0){
            fprintf(stdout, "Relay Station for Pokeymon-Online Help\n");
            fprintf(stdout, "Please visit http://www.pokemon-online.eu/ for more information.\n");
            fprintf(stdout, "Don't forget to add the IP of the relay station to the proxy servers and the trusted IPs of your server.\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "Usage: ./RelayStation [[options]]\n");
            fprintf(stdout, "Options:\n");
            PRINTOPT("-a, --address IP:port", "Sets the target server. Default is localhost:5080.");
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
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--address") == 0) {
            if (++i == argc){
                fprintf(stderr, "No host provided.\n");
                return 1;
            }
            host = argv[i];
        }
    }

    srand(time(NULL));

    QCoreApplication a(argc, argv);

    RelayStation station(port, host);
    station.start();
    
    return a.exec();
}
