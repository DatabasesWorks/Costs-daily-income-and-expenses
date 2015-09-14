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
    setupSignals();
    if(isOpen)
        updateCalculations();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSignals()
{
    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged,
                     this, &MainWindow::checkMenubar);
    QObject::connect(expensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
}

void MainWindow::updateslot()
{
    updateCalculations();
}

void MainWindow::checkMenubar()
{
//    if( ui->tabWidget->currentIndex() == 2 )
//    {
//        ui->mainToolBar->hide();
//    } else {
//        ui->mainToolBar->show();
//    }
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

    checkMenubar();
}

bool MainWindow::askClose()
{
    if (expensesmodel->isDirty() || monthlyexpensesmodel->isDirty() || categoriesmodel->isDirty() ) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Costs"),
                     tr("The database has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            int sret = save();
            if(sret == 0) {
                return true;
            }else{
                return false;
            }
        }

        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (askClose()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::on_actionAbout_Costs_triggered()
{
    QMessageBox::about(this, tr("About Costs"),
             tr("The <b>Costs</b> application should help you "
                "manage your income/outcome and is designed for "
                "easy use and by far not competing with true accounting apps."));
}

int MainWindow::createExpensesView()
{
    expensesmodel = new QSqlRelationalTableModel(this, sqliteDb1->db);
    expensesmodel->setTable("expenses");
    expensesmodel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    expensesmodel->setRelation(5, QSqlRelation("categories", "id", "category"));
    expensesmodel->select();

    expensesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    expensesmodel->setHeaderData(1, Qt::Horizontal, "Amount / EUR");
    expensesmodel->setHeaderData(2, Qt::Horizontal, "Date");
    expensesmodel->setHeaderData(3, Qt::Horizontal, "Description");
    expensesmodel->setHeaderData(4, Qt::Horizontal, "What / Where");
    expensesmodel->setHeaderData(5, Qt::Horizontal, "Category");
    expensesmodel->setHeaderData(6, Qt::Horizontal, "Payment");

    ui->expensesTableView->setItemDelegate(new QSqlRelationalDelegate(ui->expensesTableView));
    ui->expensesTableView->setModel(expensesmodel);
    ui->expensesTableView->hideColumn(0); // Don't show id

    return 0;
}

int MainWindow::createMonthlyExpensesView()
{
    monthlyexpensesmodel = new QSqlRelationalTableModel(this, sqliteDb1->db);
    monthlyexpensesmodel->setTable("monthlyexpenses");
    monthlyexpensesmodel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    monthlyexpensesmodel->setRelation(4, QSqlRelation("categories", "id", "category"));
    monthlyexpensesmodel->select();

    monthlyexpensesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    monthlyexpensesmodel->setHeaderData(1, Qt::Horizontal, "Amount / EUR");
    monthlyexpensesmodel->setHeaderData(2, Qt::Horizontal, "Description");
    monthlyexpensesmodel->setHeaderData(3, Qt::Horizontal, "What / Where");
    monthlyexpensesmodel->setHeaderData(4, Qt::Horizontal, "Category");
    monthlyexpensesmodel->setHeaderData(5, Qt::Horizontal, "Payment");

    ui->monthlyExpensesTableView->setModel(monthlyexpensesmodel);
    ui->monthlyExpensesTableView->setItemDelegate(new QSqlRelationalDelegate(ui->monthlyExpensesTableView));
    ui->monthlyExpensesTableView->hideColumn(0); // Don't show id

    return 0;
}

int MainWindow::createCategoriesView()
{
    categoriesmodel = new QSqlRelationalTableModel(this, sqliteDb1->db);
    categoriesmodel->setTable("categories");
    categoriesmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    categoriesmodel->select();

    categoriesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    categoriesmodel->setHeaderData(1, Qt::Horizontal, "Category");

    ui->categoriesTableView->setModel(categoriesmodel);
    ui->categoriesTableView->hideColumn(0); // Don't show id

    return 0;
}

int MainWindow::openDatabase(QString fileName)
{
    sqliteDb1 = new SqliteDatabase(fileName);

    //Show table for expenses
    createExpensesView();

    // Show table for monthlyexpenses
    createMonthlyExpensesView();

    // get the table for categories
    createCategoriesView();

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
            rec.setValue(0, 10.0);
            rec.setValue(1, curdate);
            rec.setValue(2, 1);
            row = expensesmodel->rowCount();
            expensesmodel->insertRecord(row,rec);
            break;
        case 1:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("category", QVariant::Int));
            rec.setValue(0, 10.0);
            rec.setValue(1, 1);
            row = monthlyexpensesmodel->rowCount();
            monthlyexpensesmodel->insertRecord(row,rec);
            break;
        case 3:
            row = categoriesmodel->rowCount();
            categoriesmodel->insertRows(row,1);
            expensesmodel->relationModel(5)->select();
            break;
    }
    updateCalculations();
}



void MainWindow::on_actionSave_triggered()
{
    if( expensesmodel->isDirty() )
        submit(expensesmodel);
    if( monthlyexpensesmodel->isDirty() )
        submit(monthlyexpensesmodel);
    if( categoriesmodel->isDirty() ) {
        submit(categoriesmodel);
        expensesmodel->relationModel(5)->select();
        monthlyexpensesmodel->relationModel(4)->select();
    }

    updateCalculations();
}

bool MainWindow::save()
{
    on_actionSave_triggered();

    return 0;
}

void MainWindow::updateCalculations()
{
    double total=0;
    int rowcount = expensesmodel->rowCount();
    for (int row=0; row < rowcount; row++)
    {
        total += expensesmodel->record(row).value("amount").toDouble();
    }
    calcres.total = total;
    ui->totalCostsLine->setText(QString::number(total, 'f', 2));
}

void MainWindow::on_actionUpdate_triggered()
{
    updateCalculations();
}

void MainWindow::submit(QSqlRelationalTableModel *model)
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

void MainWindow::on_actionDelete_Entry_triggered()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;
    QModelIndexList selected;

    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        selmodel = ui->expensesTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            expensesmodel->removeRows( selected.at(i).row(), 1);
        }
        break;
    case 1:
        selmodel = ui->monthlyExpensesTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            monthlyexpensesmodel->removeRows( selected.at(i).row(), 1);
        }
        break;
    case 3:
        selmodel = ui->categoriesTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            categoriesmodel->removeRows( selected.at(i).row(), 1);
        }
        break;
    }

    updateCalculations();
}
