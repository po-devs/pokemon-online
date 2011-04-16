#ifndef SCRIPTDB_H
#define SCRIPTDB_H

#include <QObject>
#include <QtScript>
#include <QRegExp>
#include <QScriptable>
#include <QScriptEngine>
#include "server.h"

// Instance of this class act as a prototype for PORecord script objects.
class PORecordPrototype : public QObject, public QScriptable
{
Q_OBJECT
public:
    explicit PORecordPrototype(QObject *parent = 0) : QObject(parent) {};
    Q_INVOKABLE void save();
};

class ScriptDB : public QObject
{
Q_OBJECT
public:
    explicit ScriptDB(Server *s);
    ~ScriptDB();
    Q_INVOKABLE void ensureTable(const QString &tableName, const QScriptValue &properties);
    Q_INVOKABLE void insert(const QString &tableName, const QScriptValue &properties);

signals:

public slots:

private:
    Server *myserver;
    QScriptEngine *engine;
    QString makeFields(const QScriptValue &properties);
    QRegExp rxSafeNames;
    QScriptValue por_proto; // Use this as a prototype for PORecord objects.
    PORecordPrototype *por_proto_qobject;
};

#endif // SCRIPTDB_H
