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
    db.removeDatabase(db.database().connectionName());
}

int SqliteDatabase::CreateDatabase(QString dbFilename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilename);
    db.open();
    QSqlQuery query;
    // Create expenses table
    if(!query.exec("create table expenses "
              "(id integer primary key, "
              "amount real, "
              "date date, "
              "description varchar(128), "
              "what varchar(128), "
              "category integer, "
              "payment integer,"
              "img BLOB)"
               )
            )
        qDebug() << query.lastError();
    // Create earnings table
    if(!query.exec("create table earnings "
              "(id integer primary key, "
              "amount real, "
              "date date, "
              "description varchar(128), "
              "what varchar(128), "
              "category integer, "
              "payment integer)"
               )
            )
        qDebug() << query.lastError();
    // Create monthly expenses table
    if(!query.exec("create table monthlyexpenses "
              "(id integer primary key, "
              "amount real, "
              "description varchar(128), "
              "what varchar(128), "
              "category integer, "
              "payment integer)"
               )
            )
        qDebug() << query.lastError();
    // Create monthly earnings table
    if(!query.exec("create table monthlyearnings "
              "(id integer primary key, "
              "amount real, "
              "description varchar(128), "
              "what varchar(128), "
              "category integer, "
              "payment integer)"
               )
            )
        qDebug() << query.lastError();
    // Create categories table
    if(!query.exec("create table categories "
              "(id integer primary key, "
              "category varchar(128), "
              "description varchar(128)) "
               )
            )
        qDebug() << query.lastError();
    // Insert default category "Other" into "categories" table
    if(!query.exec("insert into categories "
              "values (1, 'Other', 'Default category')")
            )
        qDebug() << query.lastError();
    // Create payments table
    if(!query.exec("create table paymentmethods "
              "(id integer primary key, "
              "payment varchar(128), "
              "description varchar(128)) "
               )
            )
        qDebug() << query.lastError();
    // Insert default payment method "Cash" into "payments" table
    if(!query.exec("insert into paymentmethods "
              "values (1, 'Cash', 'Cash')")
            )
        qDebug() << query.lastError();
    // Create version table and entries
    if(!query.exec("create table dbversion "
              "(id integer primary key, "
              "tablename varchar(128), "
              "tableversion real) "
               )
            )
        qDebug() << query.lastError();
    if(!query.exec("insert into dbversion "
              "values (1, 'expenses', 1.0)")
            )
        qDebug() << query.lastError();
    if(!query.exec("insert into dbversion "
              "values (2, 'earnings', 1.0)")
            )
        qDebug() << query.lastError();
    if(!query.exec("insert into dbversion "
              "values (3, 'monthlyexpenses', 1.0)")
            )
        qDebug() << query.lastError();
    if(!query.exec("insert into dbversion "
              "values (4, 'monthlyearnings', 1.0)")
            )
        qDebug() << query.lastError();
    if(!query.exec("insert into dbversion "
              "values (5, 'categories', 1.0)")
            )
        qDebug() << query.lastError();
    if(!query.exec("insert into dbversion "
              "values (6, 'paymentmethods', 1.0)")
            )
        qDebug() << query.lastError();
    db.close();

    return 0;
}
