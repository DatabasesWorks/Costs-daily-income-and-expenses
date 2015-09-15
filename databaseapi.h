#ifndef DATABASEAPI_H
#define DATABASEAPI_H

#include <QString>

#include <QSqlDatabase>

class SqliteDatabase
{
public:
    SqliteDatabase(QString dbfilename);
    ~SqliteDatabase();
    static int CreateDatabase(QString dbFilename);

    QSqlDatabase db;

    void close();
};

#endif // DATABASEAPI_H
