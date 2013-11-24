#include <QCoreApplication>
#include <QDir>

#include "battleserver.h"

int main(int argc, char *argv[])
{
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
    server.start();

    return a.exec();
}
