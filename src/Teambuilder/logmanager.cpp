#include "logmanager.h"
#include <QDir>
#include <QTime>
#include <QFile>
#include "../Utilities/functions.h"

LogManager* LogManager::instance = NULL;

Log::Log(LogType type, const QString &title)
{
    key.type = type;
    key.title = title + QDate::currentDate().toString("dd MMMM yyyy")
                 + " at " + QTime::currentTime().toString("hh'h'mm");

    QRegExp removeIllegal(QString("[\\/:\"/*?<>|]"));
    key.title.remove(removeIllegal);

    if (type == ReplayLog) {
        key.title += ".poreplay";
    } else {
        key.title += ".html";
    }

    started = autolog = false;
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
    QFile out (dir.absoluteFilePath(title()));
    out.open(started ? QIODevice::Append : QIODevice::WriteOnly);
    out.write(isBinary() ? bdata : data.toUtf8());

    started = true;
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

    /* The reason for not checking if linecount > 100,
      is that sometimes we want to prepare a log while
      the user didn't select logging on yet, and in that
      case we don't want to call flush() every line after
      the 100 first lines */
    if (autolog && linecount % 100 == 0) {
        flush();
    }
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

    setDefaultValue(s, "logs_directory", QDir::homePath() + "/Documents/Pokemon-Online Logs/");
    setDefaultValue(s, "save_battle_logs", false);

    directory = s.value("logs_directory").toString();
    flags |= (s.value("save_battle_logs").toBool() << (BattleLog|ReplayLog));

    QDir d;
    d.mkpath(getDirectory());
    d.mkpath(getDirectoryForType(ReplayLog));
    d.mkpath(getDirectoryForType(BattleLog));
}

bool LogManager::logsType(LogType type)
{
    return flags & (1 << type);
}

void LogManager::changeLogSaving(LogType type, bool save)
{
    if (type == BattleLog) {
        QSettings s;
        s.setValue("save_battle_logs", save);
    }
    flags &= (0XFFFF ^ (1 << type));
}

void LogManager::changeBaseDirectory(const QString &directory)
{
    this->directory = directory;

    QSettings s;
    s.setValue("logs_directory", directory);
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
        return directory;
    }
}
