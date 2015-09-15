#include "databaseapi.h"

#include <QString>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteDatabase::SqliteDatabase(QString dbFilename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilename);
    db.open();
}

SqliteDatabase::~SqliteDatabase()
{
    db.close();
}

void SqliteDatabase::close()
{
    db.close();
}

int SqliteDatabase::CreateDatabase(QString dbFilename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilename);
    db.open();
    QSqlQuery query;
    if(!query.exec("create table expenses "
              "(id integer primary key, "
              "amount real, "
              "date date, "
              "description varchar(128), "
              "what integer, "
              "category integer, "
              "payment integer)"
               )
            )
    qDebug() << query.lastError();
    if(!query.exec("create table monthlyexpenses "
              "(id integer primary key, "
              "amount real, "
              "description varchar(128), "
              "what integer, "
              "category integer, "
              "payment integer)"
               )
            )
    qDebug() << query.lastError();
    if(!query.exec("create table categories "
              "(id integer primary key, "
              "category varchar(128), "
              "description varchar(128)) "
               )
            )
    qDebug() << query.lastError();
    if(!query.exec("create table paymentmethods "
              "(id integer primary key, "
              "payment varchar(128), "
              "description varchar(128)) "
               )
            )
    qDebug() << query.lastError();
    db.close();

    return 0;
}
