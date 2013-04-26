#include <QApplication>
#include <QProcess>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QString source, caller;
    int nb = -1;

    for (int i = 0; i < argc; i++) {
        /* The folder that contains the files that will be used for the update */
        if (strcmp(argv[i], "-src") == 0) {
            if (++i < argc) {
                source = argv[i];
            }
            continue;
        }
        /* Optional - the total number of files to update.
          Used as an indicator, to do a progress bar */
        if (strcmp(argv[i], "-nb") == 0) {
            if (++i < argc) {
                nb = atoi(argv[i]);
            }
            continue;
        }
        /* The caller, we call them back once we ended the update */
        if (strcmp(argv[i], "-caller") == 0) {
            if (++i < argc) {
                caller = argv[i];
            }
        }
    }

    (void) nb;

    if (source.isEmpty()) {
        qDebug() << "-src flag missing to find where to update from";
        return 0;
    }

    bool success = true;

    QApplication a(argc, argv);
    MainWindow w;
    w.setAttribute(Qt::WA_DeleteOnClose, true);
    w.show();

    w.setSuccessIndicator(&success);
    w.run(source);

    a.exec();

    qDebug() << "ended updater";

    if (caller.length() > 0) {
        QProcess *p = new QProcess();
        if (success) {
            p->startDetached(caller, QStringList() << "--updated");
        } else {
            p->startDetached(caller, QStringList());
        }

        delete p;
    }

    exit(0);
}
