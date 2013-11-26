#ifndef QPSQLQUERY_H
#define QPSQLQUERY_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantMap>

class QPsqlQuery : public QSqlQuery
{
public:
    QPsqlQuery();
    QPsqlQuery(QSqlDatabase db) : QSqlQuery(db){}

    void prepare(const QString &query);
    bool exec();
    bool exec(const QString &s) {return QPsqlQuery::exec(s);}

    void bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType type = QSql::InOut);
private:

    QString data;
    QVariantMap values;
};

#endif // QPSQLQUERY_H
