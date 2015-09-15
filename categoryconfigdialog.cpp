#include "categoryconfigdialog.h"
#include "ui_categoryconfigdialog.h"

#include <QtWidgets>
#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

#include "databaseapi.h"
#include "myqsqlrelationaltablemodel.h"

CategoryConfigDialog::CategoryConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CategoryConfigDialog)
{
    ui->setupUi(this);
}

CategoryConfigDialog::~CategoryConfigDialog()
{
    delete ui;
}

int CategoryConfigDialog::createCategoriesView(SqliteDatabase *sqliteDb1)
{
    categoriesmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    categoriesmodel->setTable("categories");
    categoriesmodel->setEditStrategy(QSqlTableModel::OnRowChange);
    categoriesmodel->select();

    categoriesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    categoriesmodel->setHeaderData(1, Qt::Horizontal, "Category");
    categoriesmodel->setHeaderData(2, Qt::Horizontal, "Description");

    categoriesmodel->setReadOnly(0, true);

    ui->categoriesTableView->setModel(categoriesmodel);
    ui->categoriesTableView->resizeColumnsToContents();

    return 0;
}

void CategoryConfigDialog::on_addButton_clicked()
{
    int row = categoriesmodel->rowCount();
    categoriesmodel->insertRows(row,1);
}

int CategoryConfigDialog::on_removeButton_clicked()
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

        selmodel = ui->categoriesTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            categoriesmodel->removeRows( selected.at(i).row(), 1);
        }
        submit(categoriesmodel);
        categoriesmodel->select();
        return 0;
    }
    return 0;
}

void CategoryConfigDialog::on_saveButton_clicked()
{
    if( categoriesmodel->isDirty() ) {
        submit(categoriesmodel);
    }
    this->close();
}

void CategoryConfigDialog::submit(MyQSqlRelationalTableModel *model)
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
