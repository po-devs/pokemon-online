#include <QRegExp>
#include <QSqlDriver>
#include "qpsqlquery.h"

QPsqlQuery::QPsqlQuery()
{
}

void QPsqlQuery::prepare(const QString &query)
{
    if (!postgres) {
        QSqlQuery::prepare(query);
        return;
    }
    data = query;
}

void QPsqlQuery::bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType type)
{
    if (!postgres) {
        QSqlQuery::bindValue(placeholder, val, type);
        return;
    }
    (void) type;
    if (val.type() == QVariant::String || val.type() == QVariant::ByteArray) {
        QString s = val.toString();

        s.replace("\\", "\\\\").replace("'", "\\'");
        values.insert(placeholder, "E'"+s+"'");
    } else {
        values.insert(placeholder, val);
    }
}

bool QPsqlQuery::exec()
{
    if (!postgres) {
        return QSqlQuery::exec();
    }
    QString ex = data;

    foreach(QString key, values.keys()) {
        ex.replace(key, values[key].toString());
    }

    ex.replace(QRegExp(":[a-z]+"), "\''");

    return exec(ex);
}

bool QPsqlQuery::postgres = false;

