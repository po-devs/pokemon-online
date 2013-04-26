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
#include <QApplication>

#ifdef __WIN32
#include <windows.h>
#include <windef.h>
#include <Shellapi.h>
#endif

#ifdef Q_OS_MACX
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFBundle.h>
#include "mac/SparkleAutoUpdater.h"
#include "mac/CocoaInitializer.h"
#endif

//#include <QSharedMemory>

#include "../Shared/config.h"

#ifdef _WIN32
# ifdef QT5
void myMessageOutput(QtMsgType type, const QMessageLogContext&, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s\n", msg.toLocal8Bit().data());
        fflush(stderr);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg.toLocal8Bit().data());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg.toLocal8Bit().data());
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg.toLocal8Bit().data());
        abort();
    }
}
# else
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
# endif
#endif

QTranslator qtTranslator, translator;

int main(int argc, char *argv[])
{
#ifdef _WIN32
    freopen("stdout.txt", "w", stderr);
# ifdef QT5
    qInstallMessageHandler(myMessageOutput);
# else
    qInstallMsgHandler(myMessageOutput);
# endif
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
    bool updated = false;

    for (int i = 0; i < argc; i++) {
        /* Update parameter: move file from one place to another. Called by commandline on windows
          when you need admin rights to update the auto updater for example */
        if (strcmp(argv[i], "-update") == 0) {
            quit = true;

            if (i + 2 >= argc) {
                break;
            }

            QString dest = argv[++i];
            QString src = argv[++i];

            /* Todo: check if those 3 lines are necessary ? */
            QFile s (dest);
            s.remove();
            s.close();

            QFile f (src);
            if (!f.rename(dest)) {
                //QMessageBox::critical(NULL, tr("Error during PO update"), tr("Couldn't update file %1.").arg(rel));
                qCritical() << QString("Couldn't update file %1.").arg(dest);
            }
        } else if (strcmp(argv[i], "--updated") == 0) {
            updated = true;
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
#if defined(Q_OS_MACX)

        // Check if there are updates using Sparkle
        CocoaInitializer initializer;
        SparkleAutoUpdater* updater = new SparkleAutoUpdater;
        updater->checkForUpdates();
#else
        // Auto updator and Icon are not applicable on Mac OSX
        // Mac OSX has it's own updater using Sparkle
        // Additionally, setting Icon here seems to break native OSX way.
        //
        /* icon ;) */
        a.setWindowIcon(QIcon("db/icon.png"));

        //QSharedMemory memory("Pokemon Online at " + QDir().absolutePath());

        if (settings.value("Updates/Ready").toBool()) {
            /* Check Updates/ReadyFor, match it with current updateId,
              then if true then set Updates/Ready to false and spawn the
              auto updater with the correct target directory and quit */
            //Vals: Updates/Ready, Updates/ReadyFor, Updates/ReadyTarget (directory with updates)


            QSettings in("version.ini", QSettings::IniFormat);
            int readyFor = settings.value("Updates/ReadyFor").toInt();

            if (readyFor == std::max(UPDATE_ID, in.value("updateId").toInt())) {
                /* Ensures that the PO process is the only one running */
                if (settings.value("Updates/RunningTime").toInt() + 3*60 < ::time(NULL)) {
                //if (memory.create(20, QSharedMemory::ReadOnly)) {

                    QString target = settings.value("Updates/ReadyTarget").toString();

                    if (testWritable()) {
                        QProcess p;
                        p.startDetached("./pomaintenance", QStringList() << "-src" << target << "-caller" << argv[0]);

                        goto success;
                    } else {
#ifdef Q_OS_WIN
                        qDebug() << "running as admin";
                        ::ShellExecute(0, // owner window
                                                   L"runas",
                                                   L"pomaintenance.exe", //update exe
                                                   (LPCWSTR)(QString("-src '%1' -caller '%2'").arg(target, argv[0]).utf16()), // params
                                                   0, // directory
                                                   SW_SHOWNORMAL);

                        goto success;
#else
                        goto failure;
#endif
                    }

                    success:
                    settings.setValue("Updates/Ready", false); //In case any problem happens, we won't try to update again anyway

                    //memory.detach();
                    return 0;

#ifndef Q_OS_WIN
                    failure:;
                    /* The current dir is not writable, and no way to run an auto updater with the correct permission */
                    //memory.detach();
#endif
                }
            }
        }

        /* Share memory, so that we can make sure to know the number of other apps from the same folder running atm */
        //if (!memory.create(20, QSharedMemory::ReadOnly)) {
            //memory.attach(QSharedMemory::ReadOnly);
        //}
#endif

        if (settings.value("language").isNull()) {
            settings.setValue("language", QLocale::system().name().section('_', 0, 0));
        }

        QString locale = settings.value("language").toString();

        qtTranslator.load(QString("qt_") + locale,
                QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        a.installTranslator(&qtTranslator);

        translator.load(QString("trans/%1/translation_%1").arg(locale));
        a.installTranslator(&translator);

        MainEngine w(updated);

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
