#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QTableView>

#include "databaseapi.h"

#include "myqsqlrelationaltablemodel.h"

#include "categoryconfigdialog.h"
#include "paymentmethodsconfigdialog.h"
#include "databasedialog.h"

#define STDSTATUSTIME 5000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Disable database entry actions until database gets opened
    ui->actionNew_Entry->setEnabled(false);
    ui->actionDelete_Entry->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionUpdate->setEnabled(false);
    ui->actionClose_Database->setEnabled(false);
    ui->actionEdit_Categories->setEnabled(false);
    ui->actionEdit_Payment_Methods->setEnabled(false);

    // Disable tableViews until database gets opened
    ui->expensesTableView->setEnabled(false);
    ui->monthlyExpensesTableView->setEnabled(false);

    readSettings();
    setupSignals();

    qDebug() << "MainWindow(QWidget *parent) isOpen = " << isOpen;

   // if(isOpen)
   //     updateCalculations();

//    QString filename;
//    if(!isOpen)
//        openDatabaseDialog(filename);

    ui->tabWidget->removeTab(categoriesID);

    QObject::connect(expensesmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(expensesRowHeaderChanged(Qt::Orientation,int,int)));
//    QObject::connect(earningsmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
//            this, SLOT(earningsRowHeaderChanged(Qt::Orientation,int,int)));
    QObject::connect(monthlyexpensesmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(monthlyExpensesRowHeaderChanged(Qt::Orientation,int,int)));
//    QObject::connect(monthlyearningsmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
//            this, SLOT(monthlyEarningsRowHeaderChanged(Qt::Orientation,int,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSignals()
{
    QObject::connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged,
                     this, &MainWindow::checkMenubar);
}

void MainWindow::updateslot()
{
    updateCalculations();
}

void MainWindow::openDatabaseDialog(QString &filename) {
    // Open Category Edit dialog
    DatabaseDialog *dialog = new DatabaseDialog;
    dialog->exec();
    delete dialog; dialog=0;
}

void MainWindow::checkMenubar()
{
    if( ui->tabWidget->currentIndex() == projectionsID )
    {
        ui->mainToolBar->hide();
    } else {
        ui->mainToolBar->show();
    }
}

void MainWindow::expensesRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->expensesTableView->hideRow(i);
        }
    }
}

void MainWindow::earningsRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->earningsTableView->hideRow(i);
        }
    }
}

void MainWindow::monthlyExpensesRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->monthlyExpensesTableView->hideRow(i);
        }
    }
}

void MainWindow::monthlyEarningsRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->monthlyEarningsTableView->hideRow(i);
        }
    }
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

    if(isOpen) {
        isOpen = false;
        openDatabase(dbfilename);
    }

    checkMenubar();
}

bool MainWindow::askClose()
{
    if (isOpen)
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
                } else {
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
    expensesmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    expensesmodel->setTable("expenses");
    expensesmodel->setEditStrategy(MyQSqlRelationalTableModel::OnManualSubmit);
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

    ui->expensesTableView->setEnabled(true);

    // Connect updateslot
    QObject::connect(expensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    return 0;
}

int MainWindow::createMonthlyExpensesView()
{
    monthlyexpensesmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
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

    ui->monthlyExpensesTableView->setEnabled(true);

    // Connect updateslot
    QObject::connect(monthlyexpensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    return 0;
}

int MainWindow::createCategoriesView()
{
    categoriesmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    categoriesmodel->setTable("categories");
    categoriesmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    categoriesmodel->select();

    categoriesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    categoriesmodel->setHeaderData(1, Qt::Horizontal, "Category");
    categoriesmodel->setHeaderData(2, Qt::Horizontal, "Description");

    ui->categoriesTableView->setModel(categoriesmodel);
    ui->categoriesTableView->hideColumn(0); // Don't show id

    // Connect updateslot
    QObject::connect(categoriesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    return 0;
}

int MainWindow::openDatabase(QString fileName)
{
    if(QFile(fileName).exists()){
        sqliteDb1 = new SqliteDatabase(fileName);
            //Show table for expenses
            createExpensesView();

            // Show table for monthlyexpenses
            createMonthlyExpensesView();

            // get the table for categories
            createCategoriesView();

            isOpen=true;
            dbfilename=fileName;

            // Update UI
            this->setWindowTitle("Costs - "+ dbfilename);
            ui->actionNew_Entry->setEnabled(true);
            ui->actionDelete_Entry->setEnabled(true);
            ui->actionSave->setEnabled(true);
            ui->actionUpdate->setEnabled(true);
            ui->actionClose_Database->setEnabled(true);
            ui->actionEdit_Categories->setEnabled(true);
            ui->actionEdit_Payment_Methods->setEnabled(true);
            ui->statusBar->showMessage(tr("Database opened"), STDSTATUSTIME);

            return 0;
    } else {
        QMessageBox::warning(this, tr("Costs"),
                             tr("Database cannot be opened: %1")
                             .arg(fileName));
        isOpen = false;
        return 1;
    }

}

void MainWindow::on_actionOpen_Database_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Database"),"",
                                                    tr("SQLite DB (*.db)"));
    if (!fileName.isEmpty())
        openDatabase(fileName);
}

void MainWindow::on_actionNew_Database_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Database"),"",
                                                    tr("SQLite DB (*.db)"));
    if (!fileName.isEmpty()) {
        SqliteDatabase::CreateDatabase(fileName);
        openDatabase(fileName);
    }
}

void MainWindow::on_actionNew_Entry_triggered()
{
    int row=0;

    QDate date;

    QString curdate = date.currentDate().toString("yyyy-MM-dd");
    QSqlRecord rec;

    // get first category id
    categoriesmodel->select();
    int catid = categoriesmodel->record(0).value("id").toInt();

    switch(ui->tabWidget->currentIndex())
    {
        case expensesTabID:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("date", QVariant::String));
            rec.append(QSqlField("category", QVariant::Int));
            rec.setValue(0, 0.0);
            rec.setValue(1, curdate);
            rec.setValue(2, catid);
            row = expensesmodel->rowCount();
            expensesmodel->insertRecord(row,rec);
            break;
        case monthlyExpensesTabID:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("category", QVariant::Int));
            rec.setValue(0, 0.0);
            rec.setValue(1, catid);
            row = monthlyexpensesmodel->rowCount();
            monthlyexpensesmodel->insertRecord(row,rec);
            break;
        case categoriesID:
            row = categoriesmodel->rowCount();
            categoriesmodel->insertRows(row,1);
            expensesmodel->relationModel(5)->select();
            break;
    }
    updateCalculations();
}

void MainWindow::uhideAllRows(QTableView *view)
{
    for (int i=0; i<100;i++)
    {
        view->showRow(i);
    }
}

void MainWindow::on_actionSave_triggered()
{
    if( expensesmodel->isDirty() )
        submit(expensesmodel);
        uhideAllRows(ui->expensesTableView);
    if( monthlyexpensesmodel->isDirty() )
        submit(monthlyexpensesmodel);
        uhideAllRows(ui->monthlyExpensesTableView);
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
    if(isOpen) {
        ui->statusBar->showMessage(tr("Calculating..."));
        double total=0;
        int rowcount = expensesmodel->rowCount();
        for (int row=0; row < rowcount; row++)
        {
            total += expensesmodel->record(row).value("amount").toDouble();
        }
        calcres.total = total;
        ui->totalCostsLine->setText(QString::number(total, 'f', 2));
        ui->statusBar->showMessage(tr("Calculations updated"), STDSTATUSTIME);
    }
}

void MainWindow::on_actionUpdate_triggered()
{
    updateCalculations();
}

void MainWindow::submit(MyQSqlRelationalTableModel *model)
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

    ui->statusBar->showMessage(tr("Database saved"), STDSTATUSTIME);
}

void MainWindow::on_actionDelete_Entry_triggered()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;
    QModelIndexList selected;

    switch(ui->tabWidget->currentIndex())
    {
    case expensesTabID:
        selmodel = ui->expensesTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            expensesmodel->removeRows( selected.at(i).row(), 1);
        }
        break;
    case monthlyExpensesTabID:
        selmodel = ui->monthlyExpensesTableView->selectionModel();
        current = selmodel->currentIndex();
        selected = selmodel->selectedIndexes(); // list of "selected" items
        for (int i = 0; i < selected.size(); ++i) {
            selected.at(i).row();
            monthlyexpensesmodel->removeRows( selected.at(i).row(), 1);
        }
        break;
    case categoriesID:
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

void MainWindow::on_actionFull_Screen_triggered()
{
    if(isFullScreen()) {
        this->showNormal();
    } else {
        this->showFullScreen();
    }
}

void MainWindow::on_actionQuit_triggered()
{

}

void MainWindow::on_actionEdit_Categories_triggered()
{
    // Open Category Edit dialog
    CategoryConfigDialog *dialog = new CategoryConfigDialog;
    dialog->createCategoriesView(sqliteDb1);
    dialog->exec();

    // Update database relations
    expensesmodel->relationModel(5)->select();
    monthlyexpensesmodel->relationModel(4)->select();
}

void MainWindow::on_actionToggle_Menubar_triggered()
{
    if(ui->menuBar->isHidden()) {
        ui->menuBar->show();
    } else {
        ui->menuBar->hide();
        ui->statusBar->showMessage(tr("Menubar hidden, press CTRL-M to show again"));
    }
}

void MainWindow::on_actionToggle_Toolbar_triggered()
{
    if(ui->mainToolBar->isHidden()) {
        ui->mainToolBar->show();
    } else {
        ui->mainToolBar->hide();
    }
 }

void MainWindow::on_actionClose_Database_triggered()
{
    if(isOpen) {
        if (askClose()) {
            sqliteDb1->close();
            ui->expensesTableView->setEnabled(false);
            ui->monthlyExpensesTableView->setEnabled(false);

            isOpen=false;

            // Disable database entry actions until database gets opened
            ui->actionNew_Entry->setEnabled(false);
            ui->actionDelete_Entry->setEnabled(false);
            ui->actionSave->setEnabled(false);
            ui->actionUpdate->setEnabled(false);
            ui->actionClose_Database->setEnabled(false);
            ui->actionEdit_Categories->setEnabled(false);
            ui->actionEdit_Payment_Methods->setEnabled(false);

            ui->statusBar->showMessage(tr("Database closed"), STDSTATUSTIME);
        } else {
            // do nothing
        }
    }
}

void MainWindow::on_actionEdit_Payment_Methods_triggered()
{
    // Open Category Edit dialog
    PaymentMethodsConfigDialog *dialog = new PaymentMethodsConfigDialog;
    dialog->createPaymentsMethodView(sqliteDb1);
    dialog->exec();

    // Update database relations
    // expensesmodel->relationModel(5)->select();
    // monthlyexpensesmodel->relationModel(4)->select();
}
