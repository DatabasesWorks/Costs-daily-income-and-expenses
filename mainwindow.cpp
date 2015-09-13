#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

#include "databaseapi.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::writeSettings()
{
    QSettings settings("Abyle Org", "Costs");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("currentTab", ui->tabWidget->currentIndex());
    settings.endGroup();

    ui->tabWidget->currentIndex();

    settings.beginGroup("Database");
    settings.setValue("isOpen", isOpen);
    settings.setValue("dbfilename", dbfilename);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("Abyle Org", "Costs");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    ui->tabWidget->setCurrentIndex(settings.value("currentTab").toInt());
    settings.endGroup();

    settings.beginGroup("Database");
    isOpen = settings.value("isOpen").toBool();
    dbfilename = settings.value("dbfilename").toString();
    settings.endGroup();

    if(isOpen)
        openDatabase(dbfilename);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::on_actionAbout_Costs_triggered()
{
    QMessageBox::about(this, tr("About Costs"),
             tr("The <b>Costs</b> application should help you "
                "manage your income/outcome and is designed for "
                "easy use and by far not competing with true accounting apps."));
}

int MainWindow::openDatabase(QString fileName)
{
    sqliteDb1 = new SqliteDatabase(fileName);

    //get the table for expenses
    expensesmodel = new QSqlRelationalTableModel(this, sqliteDb1->db);
    expensesmodel->setTable("expenses");
    expensesmodel->setRelation(5, QSqlRelation("categories", "id", "category"));
    ui->expensesTableView->setItemDelegate(new QSqlRelationalDelegate(ui->expensesTableView));
    expensesmodel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    expensesmodel->select();
    ui->expensesTableView->setModel(expensesmodel);

    ui->expensesTableView->hideColumn(0); // Don't show id

    // get the table for monthlyexpenses
    monthlyexpensesmodel = new QSqlRelationalTableModel(this, sqliteDb1->db);
    monthlyexpensesmodel->setTable("monthlyexpenses");
    monthlyexpensesmodel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    monthlyexpensesmodel->select();
    ui->monthlyExpensesTableView->setModel(monthlyexpensesmodel);

    ui->monthlyExpensesTableView->hideColumn(0); // Don't show id

    // get the table for categories
    categoriesmodel = new QSqlTableModel(this, sqliteDb1->db);
    categoriesmodel->setTable("categories");
    categoriesmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    categoriesmodel->select();
    ui->categoriesTableView->setModel(categoriesmodel);

    ui->categoriesTableView->hideColumn(0); // Don't show id

    isOpen=true;
    dbfilename=fileName;

    this->setWindowTitle("Costs - "+ dbfilename);

    return 0;
}

void MainWindow::on_actionOpen_Database_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        openDatabase(fileName);
}

void MainWindow::on_actionNew_Database_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (!fileName.isEmpty())
        SqliteDatabase::CreateDatabase(fileName);

    openDatabase(fileName);
}

void MainWindow::on_actionNew_Entry_triggered()
{
    int row=0;

    QDate date;

    QString curdate = date.currentDate().toString("yyyy-MM-dd");
    QSqlRecord rec;

    switch(ui->tabWidget->currentIndex())
    {
        case 0:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("date", QVariant::String));
            rec.append(QSqlField("category", QVariant::Int));
            rec.setValue(0, 10.1f);
            rec.setValue(1, curdate);
            rec.setValue(2, 0);
            row = expensesmodel->rowCount();
            expensesmodel->insertRecord(row,rec);
            break;
        case 1:
            row = monthlyexpensesmodel->rowCount();
            monthlyexpensesmodel->insertRows(row,1);
            break;
        case 3:
            row = categoriesmodel->rowCount();
            categoriesmodel->insertRows(row,1);
            break;
    }
}

void MainWindow::on_actionSave_triggered()
{
    expensesmodel->submitAll();
    monthlyexpensesmodel->submitAll();
    categoriesmodel->submitAll();
    qDebug() << expensesmodel->lastError();
}

void MainWindow::updateCalculations()
{

}

void MainWindow::on_actionUpdate_triggered()
{
    updateCalculations();
}
