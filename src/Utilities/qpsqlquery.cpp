#include "qpsqlquery.h"

QPsqlQuery::QPsqlQuery()
{
}

void QPsqlQuery::prepare(const QString &query)
{
    data = query;
}

void QPsqlQuery::bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType type)
{
    (void) type;
    if (val.type() == QVariant::String) {
        QString s = val.toString();

        s.replace("\\", "\\\\").replace("'", "\'");
        values.insert(placeholder, "'"+s+"'");
    } else {
        values.insert(placeholder, val);
    }
}

bool QPsqlQuery::exec()
{
    QString ex = data;

    foreach(QString key, values.keys()) {
        ex.replace(key, values[key].toString());
    }

    return exec(ex);
}
