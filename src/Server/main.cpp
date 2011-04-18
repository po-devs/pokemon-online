#ifndef WIN32
#include "main.h"
#endif

#ifndef PO_NO_GUI
# include <QtGui/QApplication>
# include "mainwindow.h"
#else
# include <QtCore/QCoreApplication>
#endif

#include <cstdio>
#include <iostream>
#include <signal.h>

#ifndef WIN32
#include <execinfo.h>
#endif
#include "server.h"
#include "consolereader.h"

using namespace std;

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
#else
    //set_terminate( stacktrace );
#endif

    /* Names to use later for QSettings */
    QCoreApplication::setApplicationName("Server for Pokeymon-Online");
    QCoreApplication::setOrganizationName("Dreambelievers");

    //default: show a window
    bool showWindow = true;

    QSettings s("config", QSettings::IniFormat);
    QStringList ports;

    //parse commandline arguments
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--announce") == 0){
            if (++i == argc){
                fprintf(stderr, "No server announcement provided.\n");
                return 1;
            }
            s.setValue("server_announcement", argv[i]);
        } else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--channel") == 0){
            if (++i == argc){
                fprintf(stderr, "No main channel name provided.\n");
                return 1;
            }
            s.setValue("mainchanname", argv[i]);
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
            PRINTOPT("-a, --announce [ANNOUNCE]", "Sets the server announcement.");
            PRINTOPT("-c, --channel [NAME]", "Sets the main channel name.");
            PRINTOPT("-d, --desc [DESC]", "Sets the server description.");
            PRINTOPT("-h, --help", "Displays this help.");
            PRINTOPT("-H, --headless", "Runs server without GUI (no X server required)");
            PRINTOPT("-L, --low-latency", "Runs the server in low-latency mode.");
            PRINTOPT("-n, --name [NAME]", "Sets the server name.");
            PRINTOPT("-p, --port [PORT]", "Sets the server port.");
            PRINTOPT("-P, --private", "Makes the server private.");
            fprintf(stdout, "\n");
            return 0;   //exit app
        } else if(strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--headless") == 0){
            showWindow = false;
        } else if(strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "--low-latency") ==0){
            s.setValue("low_TCP_delay", true);
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
            ports.append(argv[i]);
        } else if(strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--private") == 0){
            s.setValue("server_private", 1);
        }
    }

    fprintf(stderr, "\n-----------------------\nNew Server, starting logs\n-----------------------\n\n");

    if (ports.isEmpty()) {
        if (s.value("server_port").isNull())
            s.setValue("server_port", 5080);
    } else
        s.setValue("server_port", ports.join(","));

    QList<quint16> serverPorts;
    foreach (QString sport, s.value("server_port").toString().split(",")) {
        serverPorts.append(quint16(sport.toInt()));
    }
    Server * myserver;
    myserver = new Server(serverPorts);

#ifndef WIN32
    /* Occurs on some specific platform, so ignoring it ;_; */
    signal(SIGPIPE, SIG_IGN);
#endif

    try{
        int i = -1;
#ifndef PO_NO_GUI
        if(showWindow == false){
#endif
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

#ifndef PO_NO_GUI
        } else {
            qDebug() << "Server is running in windowed mode";
            QApplication a(argc, argv);

            MainWindow w(myserver);
            w.show();
            i = a.exec();
        }
#endif
        qDebug() << "Returned with status " << i;

    } catch (const QString &s) {
        qDebug() << "Caught string " << s;
    } catch (const char* s) {
        qDebug() << "Caught const char*  " << s;
    } /*catch (const std::exception &e) {
        qDebug() << "Caught runtime " << e.what();
    } catch (...) {
        qDebug() << "Caught Exception.";
    }*/

    return 0;
}

#ifndef WIN32
void stacktrace() 
{ 
    cout << "----------------------------------------------" << endl;
    void *trace_elems[100]; 
    int trace_elem_count(backtrace( trace_elems, 100 )); 
    char **stack_syms(backtrace_symbols( trace_elems, trace_elem_count )); 
    for ( int i = 0 ; i < trace_elem_count ; ++i ) 
    { 
        cout << stack_syms[i] << endl; 
    } 
    free( stack_syms );
    cout << "----------------------------------------------" << endl; 
    // exit(1); 
}
#endif
