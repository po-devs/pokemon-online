#ifndef SQL_H
#define SQL_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
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
    static void createSQLConnection(const QString &name = "")
    {
        databaseType = PostGreSQL;
        QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", name);
        db.setHostName("localhost");
        db.setPort(5432);
        db.setDatabaseName("pokemon");
        db.setUserName("postgres");
        db.setPassword("admin");

        if (!db.open()) {
            throw (QString("Unable to establish a database connection.") + db.lastError().text());
        }

        /* Cleans the database on restart */
        if (name == "")
            db.exec("vacuum");
    }

    enum DataBaseType  {
        SQLite,
        PostGreSQL
    };

    static int databaseType;
};

#endif // SQL_H
