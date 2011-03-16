#ifndef SCRIPTDB_H
#define SCRIPTDB_H

#include <QObject>
#include <QtScript>
#include <QRegExp>
#include "server.h"

class ScriptDB : public QObject
{
Q_OBJECT
public:
    explicit ScriptDB(Server *s);
    Q_INVOKABLE void ensureTable(const QString &tableName, const QScriptValue &properties);
    Q_INVOKABLE void insert(const QString &tableName, const QScriptValue &properties);

signals:

public slots:

private:
    Server *myserver;
    QString makeFields(const QScriptValue &properties);
    QRegExp rxSafeNames;
};

#endif // SCRIPTDB_H
