#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QHash>
#include <QObject>

enum LogType {
    BattleLog,
    PMLog,
    ChannelLog,
    ReplayLog
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
    /* Can we just append to an existing file? */
    bool appendOk;
    /* Do we push to file periodically without needing to be told to? */
    bool autolog;
    LogManager *master;

    QString data;
    QByteArray bdata;

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
    void pushList(const QStringList &list);
    void setBinary(const QByteArray &bdata);
    void flush();
    void close();

    bool isBinary() const;

    const QString  &title() const {
        return key.title;
    }

    const LogType &type() const {
        return key.type;
    }

    /* Private */
    void pushedData();
};

class LogManager : public QObject
{
    Q_OBJECT
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
    QString getDirectory() const;
    void changeBaseDirectory(const QString &directory);
    //void changeDirectoryForType(LogType type, const QString &directory);
    void changeLogSaving(LogType type, bool save);
public slots:
    /* Logs all pending data */
    void autolog();
private:
    LogManager();
    Log* getOrCreateLog(LogType type, const QString &title);

    static LogManager *instance;

    QHash<LogKey, Log*> logs;
    QString directory;
    int flags;
};

#endif // LOGMANAGER_H
