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

    try {
        recurseUpdate(source);
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

    QStringList files = d.entryList(QDir::Files | QDir::NoSymLinks);
    QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QString ds, dirs) {
        recurseUpdate(d.absoluteFilePath(ds));
    }

    /* Version.ini contains the updated info of the version.
      We *don't* want it to be updated unless the update is fully
      successful, so we put it at the end of the list */
    if (files.contains("versoin.ini")) {
        files.removeAll("version.ini");
        files.push_back("version.ini");
    }

    for (int i = 0; i < files.length(); i++) {
        QString fs = files[i];

        emit progress(tr("Updating %1...").arg(d.absoluteFilePath(fs)));

        /* Todo: are those 3 lines necessary? */
        QFile fl(src.relativeFilePath(fs));
        fl.remove();
        fl.close();

        QFile s(src.absoluteFilePath(fs));
        if (!s.rename(src.relativeFilePath(fs))) {
            emit problem(fs);

            pause();

            if (skip) {
                skip = false;
            } else {
                --i;
            }
        }
    }
}
