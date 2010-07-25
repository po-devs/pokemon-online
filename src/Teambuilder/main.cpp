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

#ifdef Q_OS_MACX
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFBundle.h>
#endif


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
    freopen("stdout.txt", "a", stderr);
    qInstallMsgHandler(myMessageOutput);
#endif
#ifdef Q_OS_MACX
    // On Mac, switch working directory to resources folder
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    QString path( CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding()) );
    path += "/Contents/Resources";
    QDir::setCurrent( path );
    CFRelease(pluginRef);
    CFRelease(macPath);
#endif

    srand(time(NULL));
    try
    {
	QApplication a(argc, argv);

        QFontDatabase::addApplicationFont("LCD.tff");
        QFontDatabase::addApplicationFont("Signshsc.tff");

	/* Names to use later for QSettings */
        QCoreApplication::setApplicationName("Pokeymon-Online");
	QCoreApplication::setOrganizationName("Dreambelievers");

        QSettings settings;
        if (settings.value("language").isNull()) {
            settings.setValue("language", QLocale::system().name().section('_', 0, 0));
        }

        QString locale = settings.value("language").toString();

        QTranslator translator;
        translator.load(QString("translation_") + locale);
        a.installTranslator(&translator);


        /* icon ;) */
	a.setWindowIcon(QIcon("db/icon.png"));

        MainEngine w;

	return a.exec();
    } catch (const QString &e) {
	qDebug() << "Exception: " << e;
    } catch (...) {
        qDebug() << "Received exception.";
    }

    return 0;
}
