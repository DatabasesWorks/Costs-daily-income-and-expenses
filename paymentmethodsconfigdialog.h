#ifndef PAYMENTMETHODSCONFIGDIALOG_H
#define PAYMENTMETHODSCONFIGDIALOG_H

#include <QDialog>

#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>


#include "databaseapi.h"
#include "myqsqlrelationaltablemodel.h"

namespace Ui {
class PaymentMethodsConfigDialog;
}

class PaymentMethodsConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentMethodsConfigDialog(QWidget *parent = 0);
    ~PaymentMethodsConfigDialog();


    int createPaymentsMethodView(SqliteDatabase *sqliteDb1);

private slots:
    void on_addButton_clicked();

    int on_removeButton_clicked();

    void on_saveButton_clicked();

private:
    Ui::PaymentMethodsConfigDialog *ui;

    MyQSqlRelationalTableModel *paymentmethodmodel;

    void submit(MyQSqlRelationalTableModel *model);
};

#endif // PAYMENTMETHODSCONFIGDIALOG_H
