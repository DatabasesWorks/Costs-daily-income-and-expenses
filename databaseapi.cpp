#include "databaseapi.h"

#include <QString>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>

SqliteDatabase::SqliteDatabase(QString dbFilename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilename);
    db.open();

    db.close();
}

int SqliteDatabase::CreateDatabase(QString dbFilename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilename);
    db.open();
    QSqlQuery query;
    query.exec("create table person "
              "(id integer primary key, "
              "firstname varchar(20), "
              "lastname varchar(30), "
              "age integer)");
    db.close();

    return 0;
}
