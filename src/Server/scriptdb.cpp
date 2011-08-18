#include <QSqlQuery>
#include "scriptdb.h"
#include "sql.h"

ScriptDB::ScriptDB(Server *s, QScriptEngine *e) : myserver(s), engine(e)
{
    rxSafeNames.setPattern("[a-z_0-9]+");
    por_proto_qobject = new PORecordPrototype();
    por_proto = engine->newQObject(por_proto_qobject);
}

ScriptDB::~ScriptDB() {
    delete por_proto_qobject;
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
    declaredFieldsList.clear();
    declaredFieldsList.append("id");
    declaredFields = "id";
    if (!properties.isObject()) return QString();
    QString result = "";
    QScriptValueIterator it(properties);
    while (it.hasNext()) {
        it.next();
        QString name = it.name().toLower();
        QString value = it.value().toString();
        // id is automatic, names should be limited, type ("value")
        // should not contain % as we use substitutions and %1 can cause issues.
        // TODO: check value data?
        if ((name != "id") && (rxSafeNames.exactMatch(name) && !value.contains("%"))) {
            result += QString(", %1 %2").arg(name, value);
            declaredFieldsList.append(name);
            declaredFields += ", ";
            declaredFields += name;
        }
    }
    return result;
}

void ScriptDB::insert(const QString &tableName, const QScriptValue &properties)
{
    if (isBlocked()) {
        myserver->print("db.insert: ensureTable failed or not called. Cannot run.");
        return;
    }
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

void PORecordPrototype::save()
{
     // thisObject().blahblah
}

PORecordPrototype::PORecordPrototype(QObject *parent) : QObject(parent)
{
    this->setObjectName("PORecord");
}

QScriptValue ScriptDB::findBy(const QString &tableName, const QString &key, const QScriptValue &value)
{
    if (isBlocked()) {
        myserver->print("db.findBy: ensureTable failed or not called. Cannot run.");
        return engine->nullValue();
    }
    if (!rxSafeNames.exactMatch(tableName)) {
        myserver->print("db.findBy: invalid table name.");
        return engine->nullValue();
    }
    QString real_key = key.toLower();
    if (!rxSafeNames.exactMatch(real_key)) {
        myserver->print("db.findBy: invalid key.");
        return engine->nullValue();
    }
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT " + declaredFields + " FROM poscript_" + tableName + " WHERE " + real_key + " = ?");
    if (value.isBool()) {
        q.addBindValue(value.toBool());
    } else if (value.isDate()) {
        q.addBindValue(value.toDateTime());
    } else if (value.isNumber()) {
        q.addBindValue(value.toNumber());
    } else if (value.isString()) {
        q.addBindValue(value.toString());
    } else {
        myserver->print("db.findBy: invalid value.");
        return engine->nullValue();
    }
    bool res = q.exec();
    if (!res) {
        return engine->nullValue();
    }
    // Shaping result for script.
    // An array of data. Each array item = 1 row.
    // q.size() can be -1. So...
    QScriptValue resulting_data = engine->newArray();
    int current_row = 0;
    while (q.next()) {
        // A single row in hash form.
        QScriptValue hash = engine->newObject();
        for (int i = 0; i < declaredFieldsList.size(); ++i) {
            QVariant current_value = q.value(i);
            QScriptValue current_result;
            if (current_value.isValid()) {
                current_result = QScriptValue(current_value.toString());
            } else {
                current_result = engine->nullValue();
            }
            QString propertyName = declaredFieldsList.at(i);
            if (propertyName == "id") {
                hash.setProperty(propertyName, current_result, QScriptValue::ReadOnly | QScriptValue::Undeletable);
            } else {
                hash.setProperty(propertyName, current_result, QScriptValue::Undeletable);
            }
        }
        resulting_data.setProperty(current_row, hash, QScriptValue::Undeletable);
        ++current_row;
    }
    resulting_data.setProperty("length", QScriptValue(current_row), QScriptValue::ReadOnly | QScriptValue::Undeletable);
    QScriptValue resulting_object = engine->newObject();
    resulting_object.setProperty("data", resulting_data, QScriptValue::ReadOnly | QScriptValue::Undeletable);
    resulting_object.setPrototype(por_proto);
    return resulting_object;
}

// Checks whether environment was set up.
bool ScriptDB::isBlocked()
{
    return declaredFieldsList.size() < 2;
}
