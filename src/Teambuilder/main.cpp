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

#include "../Shared/config.h"

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

QTranslator qtTranslator, translator;

int main(int argc, char *argv[])
{
#ifdef WIN32
    qInstallMsgHandler(myMessageOutput);
#endif
#if defined(Q_OS_MACX)
    // On Mac, switch working directory to resources folder
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    QString path( CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding()) );
    path += "/Contents/Resources";
    QDir::setCurrent( path );
    CFRelease(pluginRef);
    CFRelease(macPath);
#elif defined(PO_DATA_REPO)
    QDir::setCurrent(PO_DATA_REPO);
#endif

    bool quit = false;

    for (int i = 0; i < argc; i++) {
        /* Update parameter: move file from one place to another. Called by commandline on windows
          when you need admin rights to update the auto updater for example */
        if (strcmp(argv[i], "-update") == 0) {
            quit = true;
        }

        if (i + 2 >= argc) {
            break;
        }

        QString dest = QUrl::fromEncoded(argv[++i]).toString();
        QString src = QUrl::fromEncoded(argv[++i]).toString();

        /* Todo: check if those 3 lines are necessary ? */
        QFile s (dest);
        s.remove();
        s.close();

        QFile f (src);
        if (!f.rename(dest)) {
            //QMessageBox::critical(NULL, tr("Error during PO update"), tr("Couldn't update file %1.").arg(rel));
            qCritical() << QString("Couldn't update file %1.").arg(dest);
        }
    }

    if (quit) {
        return 0;
    }

    srand(time(NULL));
    try
    {
        //HotKeyClass HotKeyEvent;
        QApplication a(argc, argv);

        a.setQuitOnLastWindowClosed(true);

        //a.installEventFilter(&HotKeyEvent);

        /* Names to use later for QSettings */
        QCoreApplication::setApplicationName("Pokemon-Online");
        QCoreApplication::setOrganizationName("Dreambelievers");
        QCoreApplication::setApplicationVersion(VERSION);

        QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

        QSettings settings;

        if (settings.value("Updates/Ready").toBool()) {
            /* Check Updates/ReadyFor, match it with current updateId,
              then if true then set Updates/Ready to false and spawn the
              auto updater with the correct target directory and quit */
            //Vals: Updates/Ready, Updates/ReadyFor, Updates/ReadyTarget (directory with updates)
        }

        if (settings.value("language").isNull()) {
            settings.setValue("language", QLocale::system().name().section('_', 0, 0));
        }

        QString locale = settings.value("language").toString();

        qtTranslator.load(QString("qt_") + locale,
                QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        a.installTranslator(&qtTranslator);

        translator.load(QString("trans/%1/translation_%1").arg(locale));
        a.installTranslator(&translator);

        /* icon ;) */
#if not defined(Q_OS_MACX)
        a.setWindowIcon(QIcon("db/icon.png"));
#endif

        MainEngine w;

        return a.exec();
    }  /*catch (const std::exception &e) {
        qDebug() << "Caught runtime " << e.what();
    } catch (const QString &s) {
        qDebug() << "Caught string " << s;
    } */catch (const char* s) {
        qDebug() << "Caught const char*  " << s;
    } /*catch (...) {
        qDebug() << "Caught Exception.";
    }*/
    return 0;
}
