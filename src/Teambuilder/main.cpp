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
        //HotKeyClass HotKeyEvent;
	QApplication a(argc, argv);
        //a.installEventFilter(&HotKeyEvent);

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

        QFile out ("out.txt");
        out.open(QIODevice::WriteOnly);

        QStringList moves;
        for (int i = 1; i < MoveInfo::NumberOfMoves(); i++) {
            QStringList stuff;

            stuff.push_back(MoveInfo::Name(i));

            int rate = MoveInfo::RateOfStat(i, 5) >> 16;
            if (rate == 0)
                rate = 100;

            int stats = MoveInfo::StatAffected(i, 5);
            int boosts = MoveInfo::BoostOfStat(i, 5);

            if (stats != 0) {
                stuff.push_back(QString("Chance of changing stats: %1").arg(rate));

                for (int j = 2; j >= 0; j--) {
                    int stat = (signed char) (stats >> (j*8));
                    int boost = (signed char) (boosts >> (j*8));

                    if (stat == 0)
                        break;

                    stuff.push_back(QString("%1%2 %3").arg(boost > 0 ? "+" : "").arg(boost).arg(stat < 8 ? StatInfo::Stat(stat) : "All Stats"));
                }
            }
            int c = MoveInfo::Classification(i, 5);

            if (c == Move::StatusInducingMove || c == Move::OffensiveStatusInducingMove || c == Move::StatAndStatusMove) {

                int rate = MoveInfo::EffectRate(i, 5);

                if (rate == 0)
                    rate = 100;

                int status = MoveInfo::Status(i, 5);

                if (status > 0 && status <= Pokemon::Confused) {
                    QString s;

                    if (status == Pokemon::Confused) {
                        s = "Confusion";
                    } else if (status == Pokemon::Poisoned){
                        s = "Poison";

                        if (MoveInfo::MinTurns(i, 5) >= 10)
                            s += " (Toxic)";
                    } else if (status == Pokemon::Burnt) {
                        s = "Burn";
                    } else if (status == Pokemon::Frozen) {
                        s = "Freeze";
                    } else if (status == Pokemon::Paralysed) {
                        s = "Paralysis";
                    } else {
                        s = "Sleep";
                    }
                    stuff.push_back(QString("Chance of %2: %1").arg(rate).arg(s));
                }
            }

            if (MoveInfo::FlinchRate(i, 5) > 0) {
                stuff.push_back(QString("Flinch Rate: %1").arg(MoveInfo::FlinchRate(i, 5)));
            }

            if (MoveInfo::Flags(i, 5) & Move::UnthawingFlag) {
                stuff.push_back("Unthawing move");
            }

                moves.push_back(stuff.join(", "));
        }

        moves.sort();
        qDebug() << moves.size();
        out.write(moves.join("\n").toUtf8());
        out.close();

	return a.exec();
    } catch (const QString &e) {
	qDebug() << "Exception: " << e;
    } catch (...) {
        qDebug() << "Received exception.";
    }
    return 0;
}
