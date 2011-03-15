#include <QSqlQuery>
#include "scriptdb.h"
#include "sql.h"

ScriptDB::ScriptDB(Server *s) : myserver(s)
{
    rxSafeNames.setPattern("[a-z_]+");
}

// Properties should be a JS hash. This function will iterate over all properties
// and will use property name is field name and property valus as type.
// Type used as is so you must use SQL types, like varchar(255), int, etc.
void ScriptDB::ensureTable(const QString &tableName, const QScriptValue &properties)
{
    if (!rxSafeNames.exactMatch(tableName)) {
        myserver->print("ensureTable: invalid table name.");
        return;
    }
    QString fields = makeFields(properties);
    if (fields.isEmpty()) {
        myserver->print("ensureTable: cannot create table without any fields.");
        return;
    }
    QString queryString;
    switch (SQLCreator::databaseType) {
    case SQLCreator::SQLite:
        queryString = "create table if not exists %1 (" + fields + "id integer primary key autoincrement)";
        break;
    case SQLCreator::MySQL:
        queryString = "create table if not exists %1 (" + fields + "id integer auto_increment, primary key(id))";
        break;
    case SQLCreator::PostGreSQL:
        queryString = "create table %1 (" + fields + "id serial, primary key(id))";
        break;
    default:
        throw QString("Using a not supported database");
    }
    QSqlQuery q;
    if (!q.exec(queryString.arg("poscript_" + tableName))) {
        myserver->print(QString("ensureTable: creation failed. %1").arg(q.lastError().text()));
    }
}

QString ScriptDB::makeFields(const QScriptValue &properties)
{
    if (!properties.isObject()) return QString();
    QString result = "";
    QScriptValueIterator it(properties);
    int pos = 0;

    while (it.hasNext()) {
        it.next();
        QString name = it.name().toLower();
        QString value = it.value().toString();
        // id is automatic, names should be limited, type ("value")
        // should not contain % as we use substituions and %1 can cause issues.
        if ((name != "id") && (rxSafeNames.exactMatch(name) && !value.contains("%"))) {
            result += QString("%1 %2, ").arg(name, value);
        }
    }
    return result;
}
