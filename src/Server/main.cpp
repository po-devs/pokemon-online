#include <QtGui/QApplication>
#include "mainwindow.h"
#include <cstdio>
#include "../Utilities/mtrand.h"
#include "server.h"
#include "consolereader.h"
#include "../Utilities/CrossDynamicLib.h"
#include "plugininterface.h"
#include <exception>

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

    try {
        cross::DynamicLibrary dyn("myplugins/pokemonOnlineStats-plugin.dll");
        PluginInstanceFunction f = (PluginInstanceFunction)(dyn.GetFunction("createPluginClass"));
        ServerPlugin * s = f();
        if (s) {
            qDebug() << "Sucessfully opened plugin " << s->pluginName();
        }
    } catch (const std::exception &ex) {
        qDebug() << "Loading of the plugin failed: " << ex.what();
    }

    //default: show a window
    bool showWindow = true;

    //parse commandline arguments
    for(int i = 0; i < argc; i++){
        if(strcmp( argv[i], "--headless") == 0){
            showWindow = false;
        } else if(strcmp( argv[i], "--help") == 0){
            fprintf(stdout, "Server for Pokeymon-Online Help\n");
            fprintf(stdout, "Please visit http://www.pokemon-online.eu/ for more information.\n");
            fprintf(stdout, "\n");
            fprintf(stdout, "Usage: ./Server [[options]]\n");
            fprintf(stdout, "Options:\n");
            fprintf(stdout, "  --headless\tRuns server without GUI (no X server required)\n");
            fprintf(stdout, "\n");
            return 0;   //exit app
        }
    }

    fprintf(stderr, "\n-----------------------\nNew Server, starting logs\n-----------------------\n\n");

    QSettings s;
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
            myserver->start();
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
