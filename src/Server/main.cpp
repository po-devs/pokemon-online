#include <QtGui/QApplication>
#include "mainwindow.h"
#include <cstdio>

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
    fprintf(stderr, "\n-----------------------\nNew Server, starting logs\n-----------------------\n\n");

    QApplication a(argc, argv);

    /* Names to use later for QSettings */
    QCoreApplication::setApplicationName("Server for Pokeymon-Online");
    QCoreApplication::setOrganizationName("Dreambelievers");

    MainWindow w;
    w.show();
    return a.exec();
}
