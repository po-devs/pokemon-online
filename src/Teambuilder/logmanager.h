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
    bool autolog;
    LogManager *master;
    QString data;
    LogKey key;

    void pushHtml(const QString& hmtl);
    void pushTxt(const QString &txt);
    void flush();
};

class LogManager
{
public:
    enum LogType {
        Battle,
        PM,
        Channel
    };

    LogManager* obj();

    Log* createLog(LogType type, const QString &title);
    void log(LogType type, const QString &title, const QString &txt);
    void logHtml(LogType type, const QString &title, const QString &html);
    void close(LogType type, const QString &title);

private:
    static LogManager *instance;

    QHash<LogKey, Log> logs;
};

#endif // LOGMANAGER_H
