#ifndef SQL_H
#define SQL_H

#include <QSqlDatabase>
#include <QSqlError>
#include "../Utilities/qpsqlquery.h"
#include <QRegExp>
#include "server.h"
/*
    This file defines a helper function to open a connection to an
    in-memory SQLITE database and to create a test table.

    If you want to use another database, simply modify the code
    below. All the examples in this directory use this function to
    connect to a database.
*/

class SQLCreator {
public:
    static void createSQLConnection(const QString &name = QString())
    {
        QMutexLocker m(&mutex);
        QSettings s("config", QSettings::IniFormat);
        databaseType = s.value("SQL/Driver").toInt();
        databaseSchema = s.value("SQL/DatabaseSchema").toString();
        doVacuum = s.value("SQL/VacuumOnStartup").toBool();

        QString driver;
        switch(databaseType)
        {
            case PostGreSQL:
                driver = "QPSQL";
                QPsqlQuery::postgres = true;
            break;

            case MySQL:
                driver = "QMYSQL";
                QPsqlQuery::postgres = false;
            break;

            case SQLite:
            default:
                driver = "QSQLITE";
                QPsqlQuery::postgres = false;
            break;
        }

        QSqlDatabase db;
        if (name == "")
            db = QSqlDatabase::addDatabase(driver);
        else
            db = QSqlDatabase::addDatabase(driver, name);

        db.setDatabaseName(s.value("SQL/Database").toString());

        if (databaseType == PostGreSQL || databaseType == MySQL) {
            db.setHostName(s.value("SQL/Host").toString());
            db.setPort(s.value("SQL/Port").toInt());
            db.setUserName(s.value("SQL/User").toString());
            db.setPassword(s.value("SQL/Pass").toString());
        }
		
        if (databaseType == MySQL) {
            db.setConnectOptions("MYSQL_OPT_RECONNECT=1");
        }

        bool result = db.open();
        // Set default schema for PostgreSQL if defined in config.
        if (result && (databaseType == PostGreSQL) && !databaseSchema.isEmpty()) {
            QRegExp rx("[a-z_0-9]+");
            if (rx.exactMatch(databaseSchema)) {
                db.exec(QString("SET search_path TO %1;").arg(databaseSchema));
            } else {
                throw(QString("Invalid schema name."));
            }
        }
        if (!result && name=="") {
            throw (QString("Unable to establish a database connection.") + db.lastError().text());
        } else if (name == "") {
            if ((databaseType != MySQL) && (doVacuum)) db.exec("vacuum");
            throw (QString("Connection to the database established!"));
        }
    }

    static void removeDatabases() {
        foreach (QString db, QSqlDatabase::connectionNames()) {
            QSqlDatabase::removeDatabase(db);
        }
    }

    enum DataBaseType  {
        SQLite,
        PostGreSQL,
        MySQL
    };

    static int databaseType;
    static QString databaseSchema;
    static QMutex mutex;
    static bool doVacuum;
};

#endif // SQL_H
