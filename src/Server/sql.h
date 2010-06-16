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
    static void createSQLConnection(const QString &name = QString())
    {
        databaseType = PostGreSQL;

        QSqlDatabase db;
        if (name == "")
            db = QSqlDatabase::addDatabase("QPSQL");
        else
            db = QSqlDatabase::addDatabase("QPSQL", name);

        db.setHostName("localhost");
        db.setPort(5432);
        db.setDatabaseName("pokemon");
        db.setUserName("postgres");
        db.setPassword("admin");

        if (!db.open() && name=="") {
            throw (QString("Unable to establish a database connection.") + db.lastError().text());
        } else if (name == "") {
            throw (QString("Connection to the database established!"));
        }

        QSqlQuery query;
        query.exec("create table trainers (id serial, "
                                             "name varchar(20), laston char(10), auth int, banned boolean,"
                                             "salt varchar(7), hash varchar(32), ip varchar(39), primary key(id), unique(name))");

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
