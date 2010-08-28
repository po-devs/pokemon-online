#include <QtGui/QApplication>
#include <cstdio>
#include <exception>
#include "mainwindow.h"
#include "server.h"
#include "consolereader.h"

void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
	break;
    case QtWarningMsg:
	fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
	fprintf(stderr, "Critical: %s\n", msg);
	break;
    case QtFatalMsg:
	fprintf(stderr, "Fatal: %s\n", msg);
	abort();
    }
}

int main(int argc, char *argv[])
{
#ifdef WIN32
    freopen("logs.txt", "a", stderr);
    qInstallMsgHandler(myMessageOutput);
#endif

    /* Names to use later for QSettings */
    QCoreApplication::setApplicationName("Server for Pokeymon-Online");
    QCoreApplication::setOrganizationName("Dreambelievers");

    //default: show a window
    bool showWindow = true;

    QSettings s;

    //parse commandline arguments
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--announce") == 0){
            if (++i == argc){
                fprintf(stderr, "No server announcement provided.\n");
                return 1;
            }
            s.setValue("server_announcement", argv[i]);
        } else if(strcmp( argv[i], "-d") == 0 || strcmp(argv[i], "--desc") == 0){
            if (++i == argc){
                fprintf(stderr, "No server description provided.\n");
                return 1;
            }
            s.setValue("server_description", argv[i]);
        } else if(strcmp( argv[i], "-h") == 0 || strcmp( argv[i], "--help") == 0){
            fprintf(stdout, "Server for Pokeymon-Online Help\n");
            fprintf(stdout, "Please visit http://www.pokemon-online.eu/ for more information.\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "Usage: ./Server [[options]]\n");
            fprintf(stdout, "Options:\n");
            fprintf(stdout, "  %.20s\t%s\n", "-a, --announce [ANNOUNCE]", "Sets the server announcement.");
            fprintf(stdout, "  %.20s\t%s\n", "-d, --desc [DESC]", "Sets the server description.");
            fprintf(stdout, "  %.20s\t%s\n", "-h, --help", "Displays this help.");
            fprintf(stdout, "  %.20s\t%s\n", "-H, --headless", "Runs server without GUI (no X server required)");
            fprintf(stdout, "  %.20s\t%s\n", "-n, --name [NAME]", "Sets the server name.");
            fprintf(stdout, "  %.20s\t%s\n", "-p, --port [PORT]", "Sets the server port.");
            fprintf(stdout, "  %.20s\t%s\n", "-P, --private", "Makes the server private.");
            fprintf(stdout, "\n");
            return 0;   //exit app
        } else if(strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--headless") == 0){
            showWindow = false;
        } else if(strcmp( argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0){
            if (++i == argc){
                fprintf(stderr, "No server name provided.\n");
                return 1;
            }
            s.setValue("server_name", argv[i]);
        } else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0){
            if (++i == argc){
                fprintf(stderr, "No server port provided.\n");
                return 1;
            }
            s.setValue("server_port", argv[i]);
        } else if(strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--private") == 0){
            s.setValue("server_private", 1);
        }
    }

    fprintf(stderr, "\n-----------------------\nNew Server, starting logs\n-----------------------\n\n");

    if (s.value("server_port").isNull())
        s.setValue("server_port", 5080);

    quint16 serverPort = quint16(s.value("server_port").toInt());
    Server * myserver;
    myserver = new Server(serverPort);

    try{
        int i = -1;
        if(showWindow == false){

            qDebug() << "Server is running in headless mode";
            qDebug() << "Notice that it is not possible (yet) to configure the server in headless mode!";
            qDebug() << "Please change the configuration manually or in windowed mode.";
            qDebug() << "A web-tool for configuring and maintaining will be implemented later.\n";

            //in headless mode let's use QCoreApplication instead of QApplication
            QCoreApplication b(argc, argv);

            //This is done by MainWindow automatically too.
            QObject::connect(&b, SIGNAL(aboutToQuit()), myserver, SLOT(atServerShutDown()));

            myserver->start();

            ConsoleReader reader(myserver);
            QSocketNotifier notifier(fileno(stdin), QSocketNotifier::Read);
            QObject::connect(&notifier, SIGNAL(activated(int)), &reader, SLOT(read(int)));

            i = b.exec();
            qDebug() << "Returned with status " << i;

        } else {
            qDebug() << "Server is running in windowed mode";
            QApplication a(argc, argv);

            MainWindow w(myserver);
            w.show();
            i = a.exec();
        }

        qDebug() << "Returned with status " << i;

    } catch (const std::exception &e) {
        qDebug() << "Caught runtime " << e.what();
    } catch (const QString &s) {
        qDebug() << "Caught string " << s;
    } catch (const char* s) {
        qDebug() << "Caught const char*  " << s;
    } catch (...) {
        qDebug() << "Caught Exception.";
    }

    return 0;
}
