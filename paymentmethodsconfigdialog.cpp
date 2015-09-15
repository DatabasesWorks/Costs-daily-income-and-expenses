#include "paymentmethodsconfigdialog.h"
#include "ui_paymentmethodsconfigdialog.h"

#include <QtWidgets>
#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

#include "databaseapi.h"
#include "myqsqlrelationaltablemodel.h"

PaymentMethodsConfigDialog::PaymentMethodsConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PaymentMethodsConfigDialog)
{
    ui->setupUi(this);
}

PaymentMethodsConfigDialog::~PaymentMethodsConfigDialog()
{
    delete ui;
}

int PaymentMethodsConfigDialog::createPaymentsMethodView(SqliteDatabase *sqliteDb1)
{
    paymentmethodmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    paymentmethodmodel->setTable("paymentmethods");
    paymentmethodmodel->setEditStrategy(QSqlTableModel::OnRowChange);
    paymentmethodmodel->select();

    paymentmethodmodel->setHeaderData(0, Qt::Horizontal, "ID");
    paymentmethodmodel->setHeaderData(1, Qt::Horizontal, "Payment Method");
    paymentmethodmodel->setHeaderData(2, Qt::Horizontal, "Description");

    paymentmethodmodel->setReadOnly(0, true);

    ui->paymentMethodsTableView->setModel(paymentmethodmodel);
    ui->paymentMethodsTableView->resizeColumnsToContents();

    return 0;
}

void PaymentMethodsConfigDialog::on_addButton_clicked()
{
    int row = paymentmethodmodel->rowCount();
    paymentmethodmodel->insertRows(row,1);
}

int PaymentMethodsConfigDialog::on_removeButton_clicked()
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Costs"),
                 tr("Warning: A bug prevents expenses entries to show up if used categories are deleted.\n\n"
                    "Are you sure you want to remove the selected categories?"),
                 QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        QItemSelectionModel *selmodel;
        QModelIndex current;
        QModelIndexList selected;

        selmodel = ui->paymentMethodsTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            paymentmethodmodel->removeRows( selected.at(i).row(), 1);
        }
        submit(paymentmethodmodel);
        paymentmethodmodel->select();
        return 0;
    }
    return 0;
}

void PaymentMethodsConfigDialog::on_saveButton_clicked()
{
    if( paymentmethodmodel->isDirty() ) {
        submit(paymentmethodmodel);
    }
    this->close();
}

void PaymentMethodsConfigDialog::submit(MyQSqlRelationalTableModel *model)
{
    model->database().transaction();
    if (model->submitAll()) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::warning(this, tr("Costs"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
    model->select();
}
