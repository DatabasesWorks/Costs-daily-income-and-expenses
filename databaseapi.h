#ifndef DATABASEAPI_H
#define DATABASEAPI_H

#include <QString>

#include <QSqlDatabase>

class SqliteDatabase
{
public:
    SqliteDatabase(QString dbfilename);
    static int CreateDatabase(QString dbFilename);

    QSqlDatabase db;
};

#endif // DATABASEAPI_H
