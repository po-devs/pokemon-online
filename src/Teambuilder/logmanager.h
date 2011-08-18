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
};

class LogManager
{
public:
    LogManager* obj();

    Log* createLog(LogType type, const QString &title);
    void log(LogType type, const QString &title, const QString &txt);
    void logHtml(LogType type, const QString &title, const QString &html);
    void close(LogType type, const QString &title);

    bool logsType(Log::LogType type);
    QString getDirectoryForType(Log::LogType type);

private:
    static LogManager *instance;

    QHash<LogKey, Log> logs;
};

#endif // LOGMANAGER_H
