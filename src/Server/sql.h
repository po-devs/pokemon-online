#ifndef SQL_H
#define SQL_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
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
        databaseType = s.value("sql_driver").toInt();
        databaseSchema = s.value("sql_db_schema", "").toString();
        doVacuum = s.value("sql_do_vacuum", true).toBool();

        QString driver;
        switch(databaseType)
        {
            case PostGreSQL:
                driver = "QPSQL";
            break;

            case MySQL:
                driver = "QMYSQL";
            break;

            case SQLite:
            default:
                driver = "QSQLITE";
            break;
        }

        QSqlDatabase db;
        if (name == "")
            db = QSqlDatabase::addDatabase(driver);
        else
            db = QSqlDatabase::addDatabase(driver, name);

        db.setDatabaseName(s.value("sql_db_name").toString());

        if (databaseType == PostGreSQL || databaseType == MySQL) {
            db.setHostName(s.value("sql_db_host").toString());
            db.setPort(s.value("sql_db_port").toInt());
            db.setUserName(s.value("sql_db_user").toString());
            db.setPassword(s.value("sql_db_pass").toString());
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
