#include "mainwindow.h"

/* Ok guys here is the structs for the files:

   - mainwindow.cpp: The mainwindow, which manages switching between different parts of the program (teambuilder, menu, client, ..)
   - teambuilder.cpp: the teambuilder
   - client.cpp: the client
   - menu.cpp : the menu

   Oh, also, pokemoninfo, pokemonstructs, networkstructs, battlestructs are general structs used here and there.
*/

#include <QMainWindow>
#include <iostream>
#include <ctime>

int main(int argc, char *argv[])
{
    srand(time(NULL));
    try
    {
	QApplication a(argc, argv);

	/* Names to use later for QSettings */
	QCoreApplication::setApplicationName("Pogeymon-Online");
	QCoreApplication::setOrganizationName("Dreambelievers");
	/* icon ;) */
	a.setWindowIcon(QIcon("db/icon.png"));

	MainWindow w;

	w.show();
	return a.exec();
    } catch (const QString &e) {
	qDebug() << "Exception: " << e;
    }

    return 0;
}
