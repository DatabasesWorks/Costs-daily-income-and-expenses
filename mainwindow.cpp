#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QSqlTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QTableView>
#include <QtConcurrent>
#include <QSqlQuery>

#include "databaseapi.h"

#include "myqsqlrelationaltablemodel.h"

#include "categoryconfigdialog.h"
#include "paymentmethodsconfigdialog.h"
#include "csvimportdialog.h"

#define STDSTATUSTIME 5000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setEnableUIDB(false);

    ui->actionDatabase_ID->setChecked(true);

    // Progress bar
    progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->setValue(0);
    progressBar->hide();

    readSettings();
    setupSignals();

    QPalette earningspalette;
    earningspalette.setColor(QPalette::Base,QColor(182, 215, 168, 255));
    ui->totalEarningsLine->setPalette(earningspalette);

    QPalette expensespalette;
    expensespalette.setColor(QPalette::Base,QColor(249, 106, 106, 255));
    ui->totalCostsLine->setPalette(expensespalette);

    ui->tabWidget->removeTab(categoriesID);

    // Setup actions for QTableView context menu
    showReceiptAct = new QAction("Show receipt", this);
    addReceiptAct = new QAction("Add receipt", this);
    removeReceiptAct = new QAction("Remove receipt", this);
    connect(addReceiptAct, SIGNAL(triggered()), this, SLOT(addReceipt()));
    connect(showReceiptAct, SIGNAL(triggered()), this, SLOT(showReceipt()));
    connect(removeReceiptAct, SIGNAL(triggered()), this, SLOT(removeReceipt()));

    menu = new QMenu(this);
    menu->addAction(showReceiptAct);
    menu->addAction(addReceiptAct);
    menu->addAction(removeReceiptAct);

    // Setup stuff for receipt view
    scene = new QGraphicsScene;
    view = new QGraphicsView(scene);
    item = new QGraphicsPixmapItem;
    view->setGeometry(100, 100, 800, 500);

    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    QString filename, ext;

    // check for our needed mime type, here a file or a list of files
    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // extract the local paths of the files
        for (int i = 0; i < urlList.size(); ++i) {
            filename = urlList.at(i).toLocalFile();
            ext = QFileInfo(filename).suffix().toLower();
            if ( QString::compare(ext,"csv") == 0 )
                pathList.append(filename);
            else
                ui->statusBar->showMessage(tr("Dropped file not recognized: ") + QFileInfo(filename).fileName(), STDSTATUSTIME);
        }

        // call a function to open the files
        for (int i = 0; i < pathList.size(); i++) {
            fileToImportDragged(pathList.value(i));
        }
    }
}

void MainWindow::addReceipt()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;

    selmodel = ui->expensesTableView->selectionModel();
    current = selmodel->currentIndex();
    int id = expensesmodel->record(current.row()).value("id").toInt();

    if(expensesmodel->isDirty()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Costs"),
                     tr("To add a receipt the database has to be saved.\n"
                        "Do you want to save the database now?"),
                     QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Ok) {
            save();
        } else {
            return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Add receipt"),"",
                                                    tr("Image (*.jpg *.png *.bmp)"));
    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly)) return;

    QByteArray byteArray = file.readAll();

    QSqlQuery query;
     query.prepare("UPDATE expenses SET img=(?) WHERE id=(?)");
     query.addBindValue(byteArray);
     query.addBindValue(id);
     if (query.exec()) {
         expensesmodel->select();
     } else {
        QMessageBox::information(this, "Add receipt", query.lastError().text());
     }
}

void MainWindow::showReceipt()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;

    selmodel = ui->expensesTableView->selectionModel();
    current = selmodel->currentIndex();
    int id = expensesmodel->record(current.row()).value("id").toInt();

    if(expensesmodel->isDirty()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Costs"),
                     tr("To view a receipt the database has to be saved.\n"
                        "Do you want to save the database now?"),
                     QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Ok) {
            save();
        } else {
            return;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT img FROM expenses WHERE id=(?)");
    query.addBindValue(id);
    if (! query.exec()) {
       QMessageBox::information(this, "Show receipt - Load pixmap", query.lastError().text());
    }
    query.first();
    QByteArray array = query.value(0).toByteArray();
    if (! array.isEmpty()) {
        QPixmap pixmap = QPixmap();
        bool isvalidpixmap = pixmap.loadFromData(array);
        if(!isvalidpixmap){
            QMessageBox::information(this, "Show receipt", "Pixmap data stored in the DB is not valid!");
            return;
        }

        query.prepare("SELECT description FROM expenses WHERE id=(?)");
        query.addBindValue(id);
        if (! query.exec()) {
           QMessageBox::information(this, "Show receipt - Load description", query.lastError().text());
        }
        query.first();
        QString desc = query.value(0).toString();

        item->setPixmap(pixmap);

        scene->addItem(item);
        scene->setSceneRect(QRectF(0,0, pixmap.size().width(), pixmap.size().height()));
        scene->update();

        view->setWindowTitle("Receipt - " + desc);
        view->update();

        view->show();
    }
}

void MainWindow::removeReceipt()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;

    selmodel = ui->expensesTableView->selectionModel();
    current = selmodel->currentIndex();
    int id = expensesmodel->record(current.row()).value("id").toInt();

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Costs"),
                 tr("Do you want to remove the stored receipt (cannot be undone)?"),
                 QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        QSqlQuery query;
         query.prepare("UPDATE expenses SET img=NULL WHERE id=(?)");
         query.addBindValue(id);
         if (query.exec()) {
             expensesmodel->select();
         } else {
            QMessageBox::information(this, "Remove receipt", query.lastError().text());
         }
    }
}

void MainWindow::setupSignals()
{
    QObject::connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged,
                     this, &MainWindow::checkMenubar);
}

void MainWindow::updateslot()
{
    // QtConcurrent::run(this, &MainWindow::updateCalculations);
    updateCalculations();
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
            expensesHiddenRows.append(i);
        }
    }
}

void MainWindow::earningsRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->earningsTableView->hideRow(i);
            earningsHiddenRows.append(i);
        }
    }
}

void MainWindow::monthlyExpensesRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->monthlyExpensesTableView->hideRow(i);
            monthlyExpensesHiddenRows.append(i);
        }
    }
}

void MainWindow::monthlyEarningsRowHeaderChanged(Qt::Orientation orientation, int first,int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            ui->monthlyEarningsTableView->hideRow(i);
            monthlyEarningsHiddenRows.append(i);
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
        if (expensesmodel->isDirty() || monthlyexpensesmodel->isDirty() ||
            earningsmodel->isDirty() || monthlyearningsmodel->isDirty() ||
            categoriesmodel->isDirty() ) {
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
             tr("<b>Costs</b> should help you "
                "manage your income/expenses and is designed to be "
                "easy to use and simple. It is by far not competing with true accounting software."));
}

int MainWindow::createExpensesView()
{
    expensesmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    expensesmodel->setTable("expenses");
    expensesmodel->setEditStrategy(MyQSqlRelationalTableModel::OnManualSubmit);
    expensesmodel->setRelation(5, QSqlRelation("categories", "id", "category"));
    expensesmodel->setRelation(6, QSqlRelation("paymentmethods", "id", "payment"));
    expensesmodel->select();

    expensesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    expensesmodel->setHeaderData(1, Qt::Horizontal, "Amount / EUR");
    expensesmodel->setHeaderData(2, Qt::Horizontal, "Date");
    expensesmodel->setHeaderData(3, Qt::Horizontal, "Description");
    expensesmodel->setHeaderData(4, Qt::Horizontal, "What / Where");
    expensesmodel->setHeaderData(5, Qt::Horizontal, "Category");
    expensesmodel->setHeaderData(6, Qt::Horizontal, "Payment Method");
    expensesmodel->setHeaderData(7, Qt::Horizontal, "Receipt");

    expensesmodel->setColColors(1,QColor(182, 215, 168, 255)); // set 'Amount' column color
    expensesmodel->setColColors(5,QColor(239, 239, 239, 255)); // set 'Category' column color
    expensesmodel->setColColors(6,QColor(239, 239, 239, 255)); // set 'Payment Method' column color

    ui->expensesTableView->setModel(expensesmodel);
    ui->expensesTableView->setItemDelegate(new QSqlRelationalDelegate(ui->expensesTableView));
    ui->expensesTableView->hideColumn(0); // Don't show id
    ui->expensesTableView->resizeColumnsToContents();

    // ui->expensesTableView->setSortingEnabled(true);

    ui->expensesTableView->setEnabled(true);

    expensesmodel->setReadOnly(7,true);

    // Connect updateslot
    QObject::connect(expensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    QObject::connect(expensesmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(expensesRowHeaderChanged(Qt::Orientation,int,int)));

    // Add context menu
    ui->expensesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->expensesTableView, SIGNAL(customContextMenuRequested(QPoint)),
                     this, SLOT(customMenuRequested(QPoint)));

    return 0;
}

void MainWindow::customMenuRequested(QPoint pos){
    QModelIndex index=ui->expensesTableView->indexAt(pos);
    ui->expensesTableView->selectRow(index.row());

    menu->popup(ui->expensesTableView->viewport()->mapToGlobal(pos));
}

int MainWindow::createEarningsView()
{
    earningsmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    earningsmodel->setTable("earnings");
    earningsmodel->setEditStrategy(MyQSqlRelationalTableModel::OnManualSubmit);
    earningsmodel->setRelation(5, QSqlRelation("categories", "id", "category"));
    earningsmodel->setRelation(6, QSqlRelation("paymentmethods", "id", "payment"));
    earningsmodel->select();

    earningsmodel->setHeaderData(0, Qt::Horizontal, "ID");
    earningsmodel->setHeaderData(1, Qt::Horizontal, "Amount / EUR");
    earningsmodel->setHeaderData(2, Qt::Horizontal, "Date");
    earningsmodel->setHeaderData(3, Qt::Horizontal, "Description");
    earningsmodel->setHeaderData(4, Qt::Horizontal, "What / Where");
    earningsmodel->setHeaderData(5, Qt::Horizontal, "Category");
    earningsmodel->setHeaderData(6, Qt::Horizontal, "Payment Method");

    earningsmodel->setColColors(1,QColor(182, 215, 168, 255)); // set 'Amount' column color
    earningsmodel->setColColors(5,QColor(239, 239, 239, 255)); // set 'Category' column color
    earningsmodel->setColColors(6,QColor(239, 239, 239, 255)); // set 'Payment Method' column color

    ui->earningsTableView->setModel(earningsmodel);
    ui->earningsTableView->setItemDelegate(new QSqlRelationalDelegate(ui->earningsTableView));
    ui->earningsTableView->hideColumn(0); // Don't show id
    ui->earningsTableView->resizeColumnsToContents();

    ui->earningsTableView->setEnabled(true);

    // Connect updateslot
    QObject::connect(earningsmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    QObject::connect(earningsmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(earningsRowHeaderChanged(Qt::Orientation,int,int)));

    return 0;
}

int MainWindow::createMonthlyExpensesView()
{
    monthlyexpensesmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    monthlyexpensesmodel->setTable("monthlyexpenses");
    monthlyexpensesmodel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    monthlyexpensesmodel->setRelation(4, QSqlRelation("categories", "id", "category"));
    monthlyexpensesmodel->setRelation(5, QSqlRelation("paymentmethods", "id", "payment"));
    monthlyexpensesmodel->select();

    monthlyexpensesmodel->setHeaderData(0, Qt::Horizontal, "ID");
    monthlyexpensesmodel->setHeaderData(1, Qt::Horizontal, "Amount / EUR");
    monthlyexpensesmodel->setHeaderData(2, Qt::Horizontal, "Description");
    monthlyexpensesmodel->setHeaderData(3, Qt::Horizontal, "What / Where");
    monthlyexpensesmodel->setHeaderData(4, Qt::Horizontal, "Category");
    monthlyexpensesmodel->setHeaderData(5, Qt::Horizontal, "Payment Method");

    monthlyexpensesmodel->setColColors(1,QColor(182, 215, 168, 255)); // set 'Amount' column color
    monthlyexpensesmodel->setColColors(4,QColor(239, 239, 239, 255)); // set 'Category' column color
    monthlyexpensesmodel->setColColors(5,QColor(239, 239, 239, 255)); // set 'Payment Method' column color


    ui->monthlyExpensesTableView->setModel(monthlyexpensesmodel);
    ui->monthlyExpensesTableView->setItemDelegate(new QSqlRelationalDelegate(ui->monthlyExpensesTableView));
    ui->monthlyExpensesTableView->hideColumn(0); // Don't show id
    ui->monthlyExpensesTableView->resizeColumnsToContents();

    ui->monthlyExpensesTableView->setEnabled(true);

    // Connect updateslot
    QObject::connect(monthlyexpensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    QObject::connect(monthlyexpensesmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(monthlyExpensesRowHeaderChanged(Qt::Orientation,int,int)));

    return 0;
}

int MainWindow::createMonthlyEarningsView()
{
    monthlyearningsmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    monthlyearningsmodel->setTable("monthlyearnings");
    monthlyearningsmodel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    monthlyearningsmodel->setRelation(4, QSqlRelation("categories", "id", "category"));
    monthlyearningsmodel->setRelation(5, QSqlRelation("paymentmethods", "id", "payment"));
    monthlyearningsmodel->select();

    monthlyearningsmodel->setHeaderData(0, Qt::Horizontal, "ID");
    monthlyearningsmodel->setHeaderData(1, Qt::Horizontal, "Amount / EUR");
    monthlyearningsmodel->setHeaderData(2, Qt::Horizontal, "Description");
    monthlyearningsmodel->setHeaderData(3, Qt::Horizontal, "What / Where");
    monthlyearningsmodel->setHeaderData(4, Qt::Horizontal, "Category");
    monthlyearningsmodel->setHeaderData(5, Qt::Horizontal, "Payment Method");

    monthlyearningsmodel->setColColors(1,QColor(182, 215, 168, 255)); // set 'Amount' column color
    monthlyearningsmodel->setColColors(4,QColor(239, 239, 239, 255)); // set 'Category' column color
    monthlyearningsmodel->setColColors(5,QColor(239, 239, 239, 255)); // set 'Payment Method' column color


    ui->monthlyEarningsTableView->setModel(monthlyearningsmodel);
    ui->monthlyEarningsTableView->setItemDelegate(new QSqlRelationalDelegate(ui->monthlyEarningsTableView));
    ui->monthlyEarningsTableView->hideColumn(0); // Don't show id
    ui->monthlyEarningsTableView->resizeColumnsToContents();

    ui->monthlyEarningsTableView->setEnabled(true);

    // Connect updateslot
    QObject::connect(monthlyearningsmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    QObject::connect(monthlyearningsmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(monthlyEarningsRowHeaderChanged(Qt::Orientation,int,int)));

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

int MainWindow::createPaymentsView()
{
    paymentmethodmodel = new MyQSqlRelationalTableModel(this, sqliteDb1->db);
    paymentmethodmodel->setTable("paymentmethods");
    paymentmethodmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    paymentmethodmodel->select();

    paymentmethodmodel->setHeaderData(0, Qt::Horizontal, "ID");
    paymentmethodmodel->setHeaderData(1, Qt::Horizontal, "Category");
    paymentmethodmodel->setHeaderData(2, Qt::Horizontal, "Description");

//    ui->categoriesTableView->setModel(categoriesmodel);
//    ui->categoriesTableView->hideColumn(0); // Don't show id

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

            //Show table for expenses
            createEarningsView();

            // Show table for monthlyexpenses
            createMonthlyExpensesView();

            // Show table for monthlyearnings
            createMonthlyEarningsView();

            // get the table for categories
            createCategoriesView();

            // get the table for categories
            createPaymentsView();

            isOpen=true;
            dbfilename=fileName;

            // Update UI
            this->setWindowTitle("Costs - "+ dbfilename);

            setEnableUIDB(true);

            ui->statusBar->showMessage(tr("Database opened"), STDSTATUSTIME);

            updateCalculations();

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

    // get first payment id
    paymentmethodmodel->select();
    int paymentid = paymentmethodmodel->record(0).value("id").toInt();

    switch(ui->tabWidget->currentIndex())
    {
        case expensesTabID:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("date", QVariant::String));
            rec.append(QSqlField("category", QVariant::Int));
            rec.append(QSqlField("payment", QVariant::Int));
            rec.setValue(0, 0.0);
            rec.setValue(1, curdate);
            rec.setValue(2, catid);
            rec.setValue(3, paymentid);
            row = expensesmodel->rowCount();
            expensesmodel->insertRecord(row,rec);
            break;
        case earningsTabID:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("date", QVariant::String));
            rec.append(QSqlField("category", QVariant::Int));
            rec.append(QSqlField("payment", QVariant::Int));
            rec.setValue(0, 0.0);
            rec.setValue(1, curdate);
            rec.setValue(2, catid);
            rec.setValue(3, paymentid);
            row = earningsmodel->rowCount();
            earningsmodel->insertRecord(row,rec);
            break;
        case monthlyExpensesTabID:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("category", QVariant::Int));
            rec.append(QSqlField("payment", QVariant::Int));
            rec.setValue(0, 0.0);
            rec.setValue(1, catid);
            rec.setValue(2, paymentid);
            row = monthlyexpensesmodel->rowCount();
            monthlyexpensesmodel->insertRecord(row,rec);
            break;
        case monthlyEarningsTabID:
            rec.append(QSqlField("amount", QVariant::Double));
            rec.append(QSqlField("category", QVariant::Int));
            rec.append(QSqlField("payment", QVariant::Int));
            rec.setValue(0, 0.0);
            rec.setValue(1, catid);
            rec.setValue(2, paymentid);
            row = monthlyearningsmodel->rowCount();
            monthlyearningsmodel->insertRecord(row,rec);
            break;
        case categoriesID:
            row = categoriesmodel->rowCount();
            categoriesmodel->insertRows(row,1);
            expensesmodel->relationModel(5)->select();
            break;
    }
    updateCalculations();
}

void MainWindow::uhideAllRows(QTableView *view, QList<qint8> &rowList)
{
    int row;
    while(!rowList.isEmpty()) {
        row = rowList.takeFirst();
        view->showRow(row);
    }
}

void MainWindow::on_actionSave_triggered()
{
    if( expensesmodel->isDirty() )
        submit(expensesmodel);
        uhideAllRows(ui->expensesTableView, expensesHiddenRows);
    if( earningsmodel->isDirty() )
        submit(earningsmodel);
        uhideAllRows(ui->earningsTableView, earningsHiddenRows);
    if( monthlyexpensesmodel->isDirty() )
        submit(monthlyexpensesmodel);
        uhideAllRows(ui->monthlyExpensesTableView, monthlyExpensesHiddenRows);
    if( monthlyearningsmodel->isDirty() )
        submit(monthlyearningsmodel);
        uhideAllRows(ui->monthlyEarningsTableView, monthlyEarningsHiddenRows);
    if( categoriesmodel->isDirty() ) {
        submit(categoriesmodel);
        expensesmodel->relationModel(5)->select();
        earningsmodel->relationModel(5)->select();
        monthlyexpensesmodel->relationModel(4)->select();
        monthlyearningsmodel->relationModel(4)->select();
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
        double total=0, earntotal=0;

        // Get all the data
        while(expensesmodel->canFetchMore())
            expensesmodel->fetchMore();

        int rowcount = expensesmodel->rowCount();

        int pbvalue;

        QDate curdate;
        QDate firstdate = QDate::fromString("2300-12-31","yyyy-MM-dd");
        QDate lastdate = QDate::fromString("1900-01-01","yyyy-MM-dd");

        progressBar->show();

        int expcount=0, earncount=0;
        for (int row=0; row < rowcount; row++)
        {
            //  progressbar stuff
            pbvalue = 100/(double)rowcount*(row+1);
            progressBar->setValue(pbvalue);

            curdate = QDate::fromString(expensesmodel->record(row).value("date").toString(),"yyyy-MM-dd");
            if(curdate<firstdate)
                firstdate = curdate;
//            if(curdate>lastdate)
//                lastdate = curdate;
            if(expensesmodel->record(row).value("amount").toDouble()<0) {
                total += expensesmodel->record(row).value("amount").toDouble();
                expcount++;
            } else {
                earntotal += expensesmodel->record(row).value("amount").toDouble();
                earncount++;
            }
        }

        lastdate = QDate::currentDate();

        int dayspassed = firstdate.daysTo(lastdate)+1;
        calcres.total = total;
        ui->totalCostsLine->setText(QString::number(total, 'f', 2));
        ui->totalEarningsLine->setText(QString::number(earntotal, 'f', 2));
        ui->avgDailyCostsLine->setText(QString::number(total/(double)dayspassed, 'f', 2));
        ui->expectedCostsPerMonthLine->setText(QString::number(total/(double)dayspassed*30.5, 'f', 2));
        ui->expectedTotalCostsLine->setText(QString::number(total/(double)dayspassed*365.0, 'f', 2));
        ui->daysPassedLine->setText(QString::number(dayspassed, 'f', 0));

        progressBar->hide();

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

void MainWindow::deleteEntries(MyQSqlRelationalTableModel *model, QTableView *view)
{
    QItemSelectionModel *selmodel;
    QModelIndex current;
    QModelIndexList selected;

    progressBar->show();
    int pbvalue=0;

    selmodel = view->selectionModel();
    current = selmodel->currentIndex();
    selected = selmodel->selectedIndexes(); // list of "selected" items
    for (int i = 0; i < selected.size(); ++i) {
        //  progressbar stuff
        pbvalue = 100/(double)selected.size()*(i+1);
        progressBar->setValue(pbvalue);

        selected.at(i).row();
        model->removeRows( selected.at(i).row(), 1);
    }

    progressBar->hide();

}

void MainWindow::on_actionDelete_Entry_triggered()
{
    switch(ui->tabWidget->currentIndex())
    {
    case expensesTabID:
        deleteEntries(expensesmodel, ui->expensesTableView);
        break;
    case earningsTabID:
        deleteEntries(earningsmodel, ui->earningsTableView);
        break;
    case monthlyExpensesTabID:
        deleteEntries(monthlyexpensesmodel, ui->monthlyExpensesTableView);
        break;
    case monthlyEarningsTabID:
        deleteEntries(monthlyearningsmodel, ui->monthlyEarningsTableView);
        break;
    case categoriesID:
        deleteEntries(categoriesmodel, ui->categoriesTableView);
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

void MainWindow::on_actionEdit_Categories_triggered()
{
    // Open Category Edit dialog
    CategoryConfigDialog *dialog = new CategoryConfigDialog;
    dialog->createCategoriesView(sqliteDb1);
    dialog->exec();

    // Update database relations
    expensesmodel->relationModel(5)->select();
    earningsmodel->relationModel(5)->select();
    monthlyexpensesmodel->relationModel(4)->select();
    monthlyearningsmodel->relationModel(4)->select();
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
            ui->earningsTableView->setEnabled(false);
            ui->monthlyExpensesTableView->setEnabled(false);
            ui->monthlyEarningsTableView->setEnabled(false);

            isOpen=false;

            setEnableUIDB(false);

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
    expensesmodel->relationModel(6)->select();
    earningsmodel->relationModel(6)->select();
    monthlyexpensesmodel->relationModel(5)->select();
    monthlyearningsmodel->relationModel(5)->select();
}

// CSV handling functions
int MainWindow::getCatId(QString categorystring)
{
    // Search category and return the id
    // Get all the data
    while(categoriesmodel->canFetchMore())
        categoriesmodel->fetchMore();

    int rowcount = categoriesmodel->rowCount();

    int curid;
    QString curcatname;
    int maxid=0;

    for (int row=0; row < rowcount; row++) {
        curid = categoriesmodel->record(row).value("id").toInt();
        if(curid > maxid)
            maxid = curid;
        curcatname = categoriesmodel->record(row).value("category").toString();
        if( QString::compare(curcatname, categorystring, Qt::CaseSensitive) == 0) {
            return curid;
        }
    }

    // We seem not to have found the id, therefore we create a new entry
    QSqlRecord rec;
    rec.append(QSqlField("id", QVariant::Int));
    rec.append(QSqlField("category", QVariant::String));
    rec.append(QSqlField("description", QVariant::String));
    rec.setValue(0, maxid+1);
    rec.setValue(1, categorystring);
    rec.setValue(2, "Imported from CSV");
    int row = categoriesmodel->rowCount();
    categoriesmodel->insertRecord(row,rec);
    if( categoriesmodel->isDirty() ) {
        submit(categoriesmodel);
        expensesmodel->relationModel(5)->select();
        earningsmodel->relationModel(5)->select();
        monthlyexpensesmodel->relationModel(4)->select();
        monthlyearningsmodel->relationModel(4)->select();
    }

    return maxid+1;
}

int MainWindow::getPaymentId(QString paymentstring)
{
    // Search payment and return the id
    // Get all the data
    while(paymentmethodmodel->canFetchMore())
        paymentmethodmodel->fetchMore();

    int rowcount = paymentmethodmodel->rowCount();

    int curid;
    QString curpaymentname;
    int maxid=0;

    for (int row=0; row < rowcount; row++) {
        curid = paymentmethodmodel->record(row).value("id").toInt();
        if(curid > maxid)
            maxid = curid;
        curpaymentname = paymentmethodmodel->record(row).value("payment").toString();
        if( QString::compare(curpaymentname, paymentstring, Qt::CaseSensitive) == 0) {
            return curid;
        }
    }

    // We seem not to have found the id, therefore we create a new entry
    QSqlRecord rec;
    rec.append(QSqlField("id", QVariant::Int));
    rec.append(QSqlField("payment", QVariant::String));
    rec.append(QSqlField("description", QVariant::String));
    rec.setValue(0, maxid+1);
    rec.setValue(1, paymentstring);
    rec.setValue(2, "Imported from CSV");
    int row = paymentmethodmodel->rowCount();
    paymentmethodmodel->insertRecord(row,rec);
    if( paymentmethodmodel->isDirty() ) {
        submit(paymentmethodmodel);
        expensesmodel->relationModel(6)->select();
        earningsmodel->relationModel(6)->select();
        monthlyexpensesmodel->relationModel(5)->select();
        monthlyearningsmodel->relationModel(5)->select();
    }

    return maxid+1;
}

QStringList MainWindow::parseLine(QString line)
{
    QStringList list;
    QString curval;
    QChar curchar, nextchar;

    bool quoted=false;

    int start=0;

    curchar = line[0];

    if(QString::compare(curchar,"\"",Qt::CaseSensitive) == 0 ) {
        quoted = true;
        // skip the "
        start++;
    }

    for (int cc=start; cc<line.length(); cc++) {
        curchar = line[cc];
        nextchar = line[cc+1];
        if( !quoted ) {
            if(QString::compare(curchar,",",Qt::CaseSensitive) == 0 ) {
                if(QString::compare(nextchar,"\"",Qt::CaseSensitive) == 0 ) {
                    quoted = true;
                    // skip the "
                    cc++;
                }
                list.append(curval);
                curval.clear();
                continue; // If we have submitted, we do not add the next char which is ,
            }
            curval.append(curchar);
        } else { // quoted, keep adding until next char is ", then unset quoted
            if(QString::compare(nextchar,"\"",Qt::CaseSensitive) == 0) {
                quoted = false;
                // skip the "
                cc++;
            }
            curval.append(curchar);
        }
    }

    if(quoted)
        qDebug() << "ERROR IN CSV IMPORT, STILL QUOTED";

    return list;
}



void MainWindow::on_actionFrom_CSV_new_triggered()
{
    QMap<int, int> columnMap;
    int lineskip;
    QString dateformat;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import CSV"),"",
                                                    tr("CSV (*.csv)"));

    bool invertValues = false;

    if (!fileName.isEmpty()) {
        CSVImportDialog *dialog = new CSVImportDialog;
        dialog->createCSVImportView(fileName);
        if(dialog->exec()) {
            dialog->returnData(columnMap, lineskip, dateformat, invertValues);

            switch(ui->tabWidget->currentIndex())
            {
            case expensesTabID:
                importCSVFile(expensesmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            case earningsTabID:
                importCSVFile(earningsmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            case monthlyExpensesTabID:
                importCSVFile(monthlyexpensesmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            case monthlyEarningsTabID:
                importCSVFile(monthlyearningsmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            }
        }
        delete dialog;
        dialog=0;
    }
}

void MainWindow::fileToImportDragged(QString fileName)
{
    QMap<int, int> columnMap;
    int lineskip;
    QString dateformat;

    bool invertValues = false;

    if (!fileName.isEmpty()) {
        CSVImportDialog *dialog = new CSVImportDialog;
        dialog->createCSVImportView(fileName);
        if(dialog->exec()) {
            dialog->returnData(columnMap, lineskip, dateformat, invertValues);

            switch(ui->tabWidget->currentIndex())
            {
            case expensesTabID:
                importCSVFile(expensesmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            case earningsTabID:
                importCSVFile(earningsmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            case monthlyExpensesTabID:
                importCSVFile(monthlyexpensesmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            case monthlyEarningsTabID:
                importCSVFile(monthlyearningsmodel, fileName, columnMap, dateformat, invertValues, lineskip);
                break;
            }
        }
        delete dialog;
        dialog=0;
    }
}

int MainWindow::importCSVFile(MyQSqlRelationalTableModel *model, QString fileName, QMap<int, int> map, QString dateformat, bool invertValues, int lineskip)
{
    enum ColMapEnum{
        amountCol,
        dateCol,
        descriptionCol,
        categoryCol,
        paymentCol
    };

    QSqlRecord record;
    QString line;

    // Begin with disabling the updateCalculation slot
    QObject::disconnect(expensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::disconnect(earningsmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::disconnect(monthlyexpensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::disconnect(monthlyearningsmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::disconnect(categoriesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::disconnect(paymentmethodmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    QFile file(fileName);
    if (!fileName.isEmpty()) {
        // read file

        ui->statusBar->showMessage(tr("Importing CSV..."));
        progressBar->show();

        int row;

        //  progressbar stuff
        int pbvalue;
        qint64 filesize = file.size(), cursize = 0;

        // Iterate over lines
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Skip lines
            for(int l=0; l<lineskip; l++) {
                line = file.readLine();
                cursize += line.size();  //  progressBar stuff
            }
            while (!file.atEnd()) {
                line = file.readLine();

                //  progressbar stuff
                cursize += line.size();
                pbvalue = 100/(double)filesize*(cursize);
                progressBar->setValue(pbvalue);

                record.clear();

                processCSVLine(line, map, dateformat, record);

                if(invertValues)
                    record.setValue(0, -record.value(0).toReal());

                row = model->rowCount();
                model->insertRecord(row, record);
            }
            file.close();
        }

        progressBar->hide();
        ui->statusBar->showMessage(tr("Import of CSV finished"), STDSTATUSTIME);
    }

    // Enable the updateCalculation slot again
    QObject::connect(expensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::connect(earningsmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::connect(monthlyexpensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::connect(monthlyearningsmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::connect(categoriesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);
    QObject::connect(paymentmethodmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateslot);

    // Update calculations
    updateCalculations();

    ui->expensesTableView->resizeColumnsToContents();

    return 0;
}

int MainWindow::processCSVLine(QString line, QMap<int,int> map, QString dateformat, QSqlRecord &record)
{
    enum ColMapEnum{
        amountCol,
        dateCol,
        descriptionCol,
        whatCol,
        categoryCol,
        paymentCol
    };

    QDate date;
    QString curdate = date.currentDate().toString("yyyy-MM-dd");
    int catid = 1;
    int paymentid = 1;

    QStringList values = parseLine(line);

    record.append(QSqlField("amount", QVariant::Double));
    record.append(QSqlField("date", QVariant::String));
    record.append(QSqlField("description", QVariant::String));
    record.append(QSqlField("what", QVariant::String));
    record.append(QSqlField("category", QVariant::Int));
    record.append(QSqlField("payment", QVariant::Int));

    if( map[amountCol] > 0 )
        record.setValue(0, values.value(map[amountCol]-1).replace(",", ".") );
    if( map[dateCol] > 0 ) {
        curdate = date.fromString(values.value(map[dateCol]-1),dateformat).toString("yyyy-MM-dd");
        record.setValue(1, curdate);
    } else {
        record.setValue(1, curdate);
    }
    if( map[descriptionCol] > 0 )
        record.setValue(2, values.value(map[descriptionCol]-1));
    if( map[whatCol] > 0 )
        record.setValue(3, values.value(map[whatCol]-1));
    if( map[categoryCol] > 0 ) {
        // Finds the category id or creates a new category entry
        catid = getCatId(values.value(map[categoryCol]-1));
        record.setValue(4, catid);
    } else {
        catid = 1;
        record.setValue(4, catid);
    }
    if( map[paymentCol] > 0 ) {
        // Finds the category id or creates a new category entry
        paymentid = getPaymentId(values.value(map[paymentCol]-1));
        record.setValue(5, paymentid);
    } else {
        paymentid = 1;
        record.setValue(5, paymentid);
    }

    return 0;
}

void MainWindow::setEnableUIDB(bool enable)
{
    // Disable database entry actions until database gets opened
    ui->actionNew_Entry->setEnabled(enable);
    ui->actionDelete_Entry->setEnabled(enable);
    ui->actionSave->setEnabled(enable);
    ui->actionUpdate->setEnabled(enable);
    ui->actionClose_Database->setEnabled(enable);
    ui->actionEdit_Categories->setEnabled(enable);
    ui->actionEdit_Payment_Methods->setEnabled(enable);
    ui->actionFrom_CSV_new->setEnabled(enable);

    // Disable tableViews until database gets opened
    ui->expensesTableView->setEnabled(enable);
    ui->monthlyExpensesTableView->setEnabled(enable);
    ui->earningsTableView->setEnabled(enable);
    ui->monthlyEarningsTableView->setEnabled(enable);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::unsetSortChecked() {
    ui->actionDatabase_ID->setChecked(false);
    ui->actionAmount->setChecked(false);
    ui->actionDate->setChecked(false);
    ui->actionDescription->setChecked(false);
    ui->actionWhere->setChecked(false);
    ui->actionCategory->setChecked(false);
    ui->actionPayment_Method->setChecked(false);
}

void MainWindow::on_actionDatabase_ID_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(0, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(0, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionDatabase_ID->setChecked(true);
}

void MainWindow::on_actionAmount_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(1, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(1, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionAmount->setChecked(true);
}

void MainWindow::on_actionDate_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(2, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(2, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionDate->setChecked(true);
}


void MainWindow::on_actionDescription_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(3, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(3, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionDescription->setChecked(true);
}
void MainWindow::on_actionWhere_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(4, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(4, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionWhere->setChecked(true);
}

void MainWindow::on_actionCategory_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(5, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(5, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionCategory->setChecked(true);
}

void MainWindow::on_actionPayment_Method_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() || earningsmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(6, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);
    ui->earningsTableView->sortByColumn(6, Qt::AscendingOrder);
    ui->earningsTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionPayment_Method->setChecked(true);
}

void MainWindow::on_actionReport_Bug_triggered()
{
    QString link = "https://github.com/torlenor/Costs/issues";
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::on_actionCopy_triggered()
{
    // Implement copy
}
