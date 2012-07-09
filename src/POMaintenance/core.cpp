#include <QDir>
#include "core.h"

Core::Core(QObject *parent) :
    QThread(parent), quit(false), skip(false)
{
}

void Core::setSource(const QString &source)
{
    this->source = source;
}

void Core::run()
{
    if (source.isEmpty()) {
        return;
    }

    /*
        pause a bit , to let caller quit (if we need to overwrite the PO file,
        for example, we need PO to quit first
    */
    msleep(250);

    try {
        recurseUpdate(source);

        QDir d(source);
        d.cdUp();
        d.rmdir(d.relativeFilePath(source));
    } catch (const int &i) {
        //qDebug() << "thread ended prematurely, user abort?";
    }
}

void Core::skipFile()
{
    skip = true;
}

void Core::stop()
{
    quit = true;
    sem.release(1000);
}

void Core::pause()
{
    sem.acquire(1);

    if (quit) {
        throw 0;
    }
}

void Core::restart()
{
    sem.release(1);
}

void Core::recurseUpdate(const QString &dir)
{
    QDir src(source);
    QDir d(dir);
    QDir local;

    QStringList files = d.entryList(QDir::Files | QDir::NoSymLinks);
    QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QString ds, dirs) {
        recurseUpdate(d.absoluteFilePath(ds));
    }

    /* Version.ini contains the updated info of the version.
      We *don't* want it to be updated unless the update is fully
      successful, so we put it at the end of the list */
    if (files.contains("version.ini")) {
        files.removeAll("version.ini");
        files.push_back("version.ini");
    }

    for (int i = 0; i < files.length(); i++) {
        QString fs = d.absoluteFilePath(files[i]);

        emit progress(tr("Updating %1...").arg(fs));

        QString target = local.absoluteFilePath(src.relativeFilePath(fs));

        /* Todo: are those 3 lines necessary? */
        QFile fl(target);
        fl.remove();
        fl.close();

        QFile s(src.absoluteFilePath(fs));

        if (!s.rename(target)) {
            emit problem(target);

            pause();

            if (skip) {
                skip = false;
            } else {
                --i;
            }
        }
    }

    src.rmpath(src.relativeFilePath(dir));
}
