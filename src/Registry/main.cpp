#include <QtCore>
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
    fprintf(stderr, "\n-----------------------\nNew Server, starting logs\n-----------------------\n\n");

    QCoreApplication a(argc, argv);

    /* Names to use later for QSettings */
    QCoreApplication::setApplicationName("Registry for Pokeymon-Online");
    QCoreApplication::setOrganizationName("Dreambelievers");

    MainWindow w;
    return a.exec();
}
