#include "logmanager.h"
#include <QDir>
#include <QTime>

Log::Log(LogType type, const QString &title)
{
    key.type = type;
    key.title = title + QTime::currentTime().toString("hh'h'mm") + ".html";

    started = autolog = false;
    master = NULL;
    linecount = 0;
}

void Log::flush()
{
    if (!master) {
        qWarning() << "Log::flush() - Logging without support";
        return;
    }

    if (!master->logsType(type)) {
        return;
    }

    QString directory = master->getDirectoryForType(key.type);
    /* For upcoming PM & Channel logs, adding the other party/channel
      to the directoy path would be a good idea */
    if(!QDir::home().exists(directory)) {
        QDir::home().mkpath(directory);
    }

    QDir dir = QDir::home();
    dir.cd(directory);
    QFile out (dir.absoluteFilePath(file);
    out.open(started ? QIODevice::Append : QIODevice::WriteOnly);
    out.write(data.toUtf8());

    started = true;
    linecount = 0;
    data.clear();
}

void Log::pushHtml(const QString &hmtl)
{
    data += html + "\n";
    linecount += 1;

    /* The reason for not checking if linecount > 100,
      is that sometimes we want to prepare a log while
      the user didn't select logging on yet, and in that
      case we don't want to call flush() every line after
      the 100 first lines */
    if (autolog && linecount % 100 == 0) {
        flush();
    }
}
