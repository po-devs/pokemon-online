#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QHash>

struct LogKey
{
    int type;
    QString title;
};

inline int qHash(const LogKey &key)
{
    return qHash(QPair<int, QString>(key.type, key.title));
}

struct Log
{
    enum LogType {
        Battle,
        PM,
        Channel
    };

    bool started;
    bool autolog;
    LogManager *master;
    QString data;
    LogKey key;
    int linecount;

    Log(LogType type, const QString &title);

    void pushHtml(const QString& hmtl);
    void pushTxt(const QString &txt);
    void flush();
    void close();

    /* Private */
    void pushedData();
};

class LogManager
{
public:
    LogManager* obj();

    Log* createLog(LogType type, const QString &title, bool autolog=false);
    void log(LogType type, const QString &title, const QString &txt);
    void logHtml(LogType type, const QString &title, const QString &html);
    void close(LogType type, const QString &title);

    void deleteLog(Log *log); /* Use log->close() instead */

    /* Is logging enabled for those? */
    bool logsType(Log::LogType type);

    /* Base directory for those kind of logs */
    QString getDirectoryForType(Log::LogType type);
    void changeDirectoryForType(Log::LogType type, const QString &directory);

private:
    LogManager();
    Log* getOrCreateLog(LogType type, const QString &title);

    static LogManager *instance;

    QHash<LogKey, Log*> logs;
    QHash<Log::LogType, QString> directories;
    int flags;
};

#endif // LOGMANAGER_H
