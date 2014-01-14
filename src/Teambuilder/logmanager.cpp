#include "logmanager.h"
#include <QDir>
#include <QTime>
#include <QFile>
#include <QDesktopServices>
#include <Utilities/functions.h>

LogManager* LogManager::instance = NULL;

Log::Log(LogType type, const QString &title)
{
    key.type = type;
    key.title = title;

    QRegExp removeIllegal(QString("[\\~:\"/*?<>|]"));
    key.title.remove(removeIllegal);

    if (type == BattleLog || type == ReplayLog) {
        key.title = QDate::currentDate().toString("dd MMMM yyyy") + "/" + key.title + " at " + QTime::currentTime().toString("hh'h'mm");
    } else {
        key.title = key.title + "/" + QDate::currentDate().toString("dd MMMM yyyy") + " at " + QTime::currentTime().toString("hh'h'mm");
    }

    if (type == ReplayLog) {
        key.title += ".poreplay";
    } else {
        key.title += ".html";
    }

    appendOk = type == PMLog || type == ChannelLog;
    autolog = false;
    master = NULL;
    override = NoOverride;
    linecount = 0;
}

void Log::flush()
{
    if (!master) {
        qWarning() << "Log::flush() - Logging without support";
        return;
    }

    if (override == OverrideNo || (override == NoOverride && !master->logsType(type()))) {
        if (autolog && linecount > 500) {
            data.clear();
            linecount = 0;
        }
        return;
    }

    if (data.isEmpty() && bdata.isEmpty()) {
        return;
    }

    QString directory = master->getDirectoryForType(type());
    /* For upcoming PM & Channel logs, adding the other party/channel
      to the directoy path would be a good idea */
    if(!QDir::home().exists(directory)) {
        QDir::home().mkpath(directory);
    }

    QDir dir = QDir::home();
    dir.cd(directory);
    dir.mkpath(dir.absoluteFilePath(QFileInfo(title()).path()));
    QFile out (dir.absoluteFilePath(title()));
    out.open(appendOk ? QIODevice::Append : QIODevice::WriteOnly);
    out.write(isBinary() ? bdata : data.toUtf8());

    linecount = 0;
    data.clear();
}

void Log::setBinary(const QByteArray &bdata)
{
    this->bdata = bdata;
}

bool Log::isBinary() const
{
    return type() == ReplayLog;
}

void Log::pushHtml(const QString &html)
{
    data += html + "\n";
    pushedData();
}

void Log::pushTxt(const QString &txt)
{
    data += txt + "<br/>\n";
    pushedData();
}

void Log::pushList(const QStringList &list)
{
    data += list.join("");
    data += "\n";
    pushedData();
}

void Log::close()
{
    flush();
    master->deleteLog(this);
}

void Log::pushedData()
{
    linecount += 1;
}

LogManager * LogManager::obj()
{
    if (!instance) {
        instance = new LogManager();
    }

    return instance;
}

LogManager::LogManager()
{
    flags = 0;
    QSettings s;

#ifdef QT5
    const QString docLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    const QString docLocation = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
    setDefaultValue(s, "logs_directory", docLocation + "/Pokemon Online/Logs/");
    setDefaultValue(s, "Battle/SaveLogs", false);

    directory = s.value("logs_directory").toString();
    flags |= (s.value("Battle/SaveLogs").toBool() << (BattleLog|ReplayLog));
    flags |= (s.value("PMs/Logged").toBool() << PMLog);

    QDir d;
    d.mkpath(getDirectory());
    d.mkpath(getDirectoryForType(ReplayLog));
    d.mkpath(getDirectoryForType(BattleLog));
    d.mkpath(getDirectoryForType(PMLog));

    QTimer *t = new QTimer(this);

    connect(t, SIGNAL(timeout()), SLOT(autolog()));

    /* Autologs triggers every minute */
    t->start(1000*60);
}

bool LogManager::logsType(LogType type)
{
    return flags & (1 << type);
}

void LogManager::changeLogSaving(LogType type, bool save)
{
    QSettings s;
    if (type == BattleLog) {
        s.setValue("Battle/SaveLogs", save);
    } else if (type == PMLog) {
        s.setValue("PMs/Logged", save);
    }
    flags &= (0XFFFF ^ (1 << type));
}

void LogManager::autolog()
{
    foreach(Log *l, logs) {
        if (l->autolog) {
            l->flush();
        }
    }
}

void LogManager::changeBaseDirectory(const QString &directory)
{
    this->directory = directory;

    QSettings s;
    s.setValue("logs_directory", directory + "/");
}

Log * LogManager::createLog(LogType type, const QString &title, bool autolog)
{
    Log *ret = new Log(type, title);
    ret->master = this;
    ret->autolog = autolog;

    logs[ret->key] = ret;

    return ret;
}

void LogManager::deleteLog(Log *log)
{
    logs.remove(log->key);
    delete log;
}

void LogManager::close(LogType type, const QString &title)
{
    LogKey key = {type, title};

    if (logs.contains(key)) {
        logs[key]->close();
    }
}

Log * LogManager::getOrCreateLog(LogType type, const QString &title)
{
    LogKey key = {type, title};

    if (logs.contains(key)) {
        return logs[key];
    } else {
        return createLog(type, title);
    }
}

void LogManager::log(LogType type, const QString &title, const QString &txt)
{
    getOrCreateLog(type, title)->pushTxt(txt);
}

void LogManager::logHtml(LogType type, const QString &title, const QString &html)
{
    getOrCreateLog(type, title)->pushHtml(html);
}

QString LogManager::getDirectory() const
{
    return directory;
}

QString LogManager::getDirectoryForType(LogType type)
{
    if (type == BattleLog) {
        return directory + "Battle Logs/";
    } else if (type == ReplayLog) {
        return directory + "Battle Replays/";
    } else {
        if(type == PMLog) {
            return directory + "Private Messages/";
        } else {
            return directory;
        }
    }
}
