#ifndef CATEGORYCONFIGDIALOG_H
#define CATEGORYCONFIGDIALOG_H

#include <QDialog>

#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>


#include "databaseapi.h"
#include "myqsqlrelationaltablemodel.h"

namespace Ui {
class CategoryConfigDialog;
}

class CategoryConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryConfigDialog(QWidget *parent = 0);
    ~CategoryConfigDialog();

        int createCategoriesView(SqliteDatabase *sqliteDb1);

private slots:
        void on_addButton_clicked();

        int on_removeButton_clicked();

        void on_saveButton_clicked();

private:
    Ui::CategoryConfigDialog *ui;

    MyQSqlRelationalTableModel *categoriesmodel;

    void submit(MyQSqlRelationalTableModel *model);
};

#endif // CATEGORYCONFIGDIALOG_H
