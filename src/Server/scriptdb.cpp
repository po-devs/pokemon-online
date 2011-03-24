#include <QSqlQuery>
#include "scriptdb.h"
#include "sql.h"

ScriptDB::ScriptDB(Server *s) : myserver(s)
{
    rxSafeNames.setPattern("[a-z_0-9]+");
}

// Properties should be a JS hash. This function will iterate over all properties
// and will use property name is field name and property valus as type.
// Type used as is so you must use SQL types, like varchar(255), int, etc.
void ScriptDB::ensureTable(const QString &tableName, const QScriptValue &properties)
{
    if (!rxSafeNames.exactMatch(tableName)) {
        myserver->print("db.ensureTable: invalid table name.");
        return;
    }
    QString fields = makeFields(properties);
    if (fields.isEmpty()) {
        myserver->print("db.ensureTable: cannot create table without any fields.");
        return;
    }
    QString queryString;
    switch (SQLCreator::databaseType) {
    case SQLCreator::SQLite:
        queryString = "create table if not exists %1 (id integer primary key autoincrement" + fields + ")";
        break;
    case SQLCreator::MySQL:
        queryString = "create table if not exists %1 (id integer auto_increment" + fields + ", primary key(id))";
        break;
    case SQLCreator::PostGreSQL:
        queryString = "create table %1 (id serial" + fields + ", primary key(id))";
        break;
    default:
        throw QString("Using a not supported database");
    }
    QSqlQuery q;
    if (!q.exec(queryString.arg("poscript_" + tableName))) {
        myserver->print(QString("db.ensureTable: creation failed. %1").arg(q.lastError().text()));
    }
}

QString ScriptDB::makeFields(const QScriptValue &properties)
{
    if (!properties.isObject()) return QString();
    QString result = "";
    QScriptValueIterator it(properties);

    while (it.hasNext()) {
        it.next();
        QString name = it.name().toLower();
        QString value = it.value().toString();
        // id is automatic, names should be limited, type ("value")
        // should not contain % as we use substitutions and %1 can cause issues.
        if ((name != "id") && (rxSafeNames.exactMatch(name) && !value.contains("%"))) {
            result += QString(", %1 %2").arg(name, value);
        }
    }
    return result;
}

void ScriptDB::insert(const QString &tableName, const QScriptValue &properties)
{
    if (!rxSafeNames.exactMatch(tableName)) {
        myserver->print("db.insert: invalid table name.");
        return;
    }
    if (!properties.isObject()) return;

    QScriptValueIterator it(properties);

    // Preparing query.
    QString fields = "";
    QString values = "";
    bool it_has_next = it.hasNext();
    while (it_has_next) {
        it.next();
        QString name = it.name().toLower();
        if ((name != "id") && rxSafeNames.exactMatch(name)) {
            fields.append(name);
            values.append("?");
        }
        it_has_next = it.hasNext();
        if (it_has_next) {
            fields.append(", ");
            values.append(", ");
        }
    }
    if (values.isEmpty()) {
        myserver->print("db.insert: no data found.");
        return;
    }

    // Bind and execute.
    QString queryString = QString("INSERT INTO poscript_") + tableName + QString("(") + fields
                          + QString(") VALUES (") + values + QString(")");
    QSqlQuery q;
    q.prepare(queryString);
    it.toFront();
    while (it.hasNext()) {
        it.next();
        QString name = it.name().toLower();
        // Same condition as in the iteration above.
        if ((name != "id") && rxSafeNames.exactMatch(name)) {
            QScriptValue valueItself = it.value();
            if (valueItself.isNumber()) {
                q.addBindValue(valueItself.toNumber());
            } else if (valueItself.isBool()) {
                q.addBindValue(valueItself.toBool());
            } else {
                q.addBindValue(valueItself.toString());
            }
        }
    }
    if (!q.exec()) {
        myserver->print(QString("db.insert: failed. %1").arg(q.lastError().text()
            + "\nQuery string was: ") + queryString);
    }
}
