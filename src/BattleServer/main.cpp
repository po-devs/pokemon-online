#include <QCoreApplication>
#include <QDir>

#include "battleserver.h"

#define PRINTOPT(a, b) (fprintf(stdout, "  %-25s\t%s\n", a, b))

int main(int argc, char *argv[])
{
    int port = 5096;

    //parse commandline arguments
    for(int i = 0; i < argc; i++){
        if(strcmp( argv[i], "-h") == 0 || strcmp( argv[i], "--help") == 0){
            fprintf(stdout, "Battle Server for Pokemon-Online Help\n");
            fprintf(stdout, "Please visit http://www.pokemon-online.eu/ for more information.\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "Usage: ./BattleServer [[options]]\n");
            fprintf(stdout, "Options:\n");
            PRINTOPT("-h, --help", "Displays this help.");
            //PRINTOPT("-p, --port [PORT]", "Sets the server port.");
            fprintf(stdout, "\n");
            return 0;   //exit app
        } else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0){
            if (++i == argc){
                fprintf(stderr, "No server port provided.\n");
                return 1;
            }
            port = atoi(argv[i]);
        }
    }

    qDebug() << "Server is running in headless mode";
    qDebug() << "Use ./BattleServer --help to get some help";
    qDebug() << "Console commands:";
    qDebug() << "- addp <plugin-path>: Add the plugin to the battle server";
    qDebug() << "- removep <plugin-name>: Remove the plugin with said name";
    qDebug() << "- listp: List the plugins in their order";

    /* Names to use later for QSettings */
    QCoreApplication::setApplicationName("Battle Server for Pokemon-Online");
    QCoreApplication::setOrganizationName("Dreambelievers");

    QString homeDir = PO_HOME_DIR;

    if (homeDir.leftRef(1) == "~") {
        homeDir = QDir::homePath() + homeDir.mid(1);
    }

    (QDir()).mkpath(homeDir);
    QDir::setCurrent(homeDir);

    QCoreApplication a(argc, argv);
    
    BattleServer server;
    server.start(port);

    return a.exec();
}
