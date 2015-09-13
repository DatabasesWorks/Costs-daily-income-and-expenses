#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QSqlTableModel>

#include "databaseapi.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_Costs_triggered()
{
    QMessageBox::about(this, tr("About Costs"),
             tr("The <b>Costs</b> application should help you "
                "manage your income/outcome and is designed for "
                "easy use and by far not competing with true accounting apps."));
}

void MainWindow::on_actionOpen_Database_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        sqliteDb1 = new SqliteDatabase(fileName);

    //get the table
    model = new QSqlTableModel(this, sqliteDb1->db);

    model->setTable("person");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->setHeaderData(0,Qt::Horizontal,"Nom");
    model->setHeaderData(1,Qt::Horizontal,"Adresse");

    ui->expensesTableView->setModel(model);
}

void MainWindow::on_actionNew_Database_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (!fileName.isEmpty())
        SqliteDatabase::CreateDatabase(fileName);
}

void MainWindow::on_actionNew_Entry_triggered()
{
    int row = model->rowCount();
    model->insertRows(row,1);
}

void MainWindow::on_actionSave_triggered()
{
    model->submitAll();
}
