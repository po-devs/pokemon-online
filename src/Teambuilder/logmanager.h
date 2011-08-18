#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QHash>

enum LogType {
    BattleLog,
    PMLog,
    ChannelLog
};

struct LogKey
{
    LogType type;
    QString title;

    bool operator ==(const LogKey &other) const {
        return type == other.type && title == other.title;
    }
};

inline int qHash(const LogKey &key)
{
    return qHash(QPair<int, QString>(key.type, key.title));
}

class LogManager;

struct Log
{
    bool started;
    bool autolog;
    LogManager *master;
    QString data;
    LogKey key;
    int linecount;
    enum OverRide {
        NoOverride,
        OverrideYes,
        OverrideNo
    };
    OverRide override;

    Log(LogType type, const QString &title);

    void pushHtml(const QString& html);
    void pushTxt(const QString &txt);
    void flush();
    void close();

    const QString  &title() const {
        return key.title;
    }

    const LogType &type() const {
        return key.type;
    }

    /* Private */
    void pushedData();
};

class LogManager
{
public:
    static LogManager* obj();

    Log* createLog(LogType type, const QString &title, bool autolog=false);
    void log(LogType type, const QString &title, const QString &txt);
    void logHtml(LogType type, const QString &title, const QString &html);
    void close(LogType type, const QString &title);

    void deleteLog(Log *log); /* Use log->close() instead */

    /* Is logging enabled for those? */
    bool logsType(LogType type);

    /* Base directory for those kind of logs */
    QString getDirectoryForType(LogType type);
    void changeDirectoryForType(LogType type, const QString &directory);
    void changeLogSaving(LogType type, bool save);

private:
    LogManager();
    Log* getOrCreateLog(LogType type, const QString &title);

    static LogManager *instance;

    QHash<LogKey, Log*> logs;
    QHash<LogType, QString> directories;
    int flags;
};

#endif // LOGMANAGER_H
