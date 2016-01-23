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

#include "generichelper.h"

#include "databaseapi.h"

#include "myqsqlrelationaltablemodel.h"

#include "categoryconfigdialog.h"
#include "paymentmethodsconfigdialog.h"
#include "csvimportdialog.h"
#include "myplots.h"

#define STDSTATUSTIME 5000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setAcceptDrops(true);
    setEnableUIDB(false);
    ui->actionDatabase_ID->setChecked(true);

    setupProgressBarUI();
    setupReceiptViewUI();
    setupPlotUI();
    setupGenericUI();
    setupTableViewContectMenu();
    setupRecentFiles();

    readSettings();

    if(isOpen) {
        isOpen = false;
        openDatabase(dbfilename);
    }

    setupSignals();
    checkMenubar();

    emit doUpdate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupRecentFiles()
{
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    for (int i = 0; i < MaxRecentFiles; ++i) {
        ui->menuFile->insertAction(ui->actionQuit, recentFileActs[i]);
    }
    ui->menuFile->insertSeparator(ui->actionQuit);

    updateRecentFileActions();
}

void MainWindow::setupProgressBarUI()
{
    // Progress bar
    progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    ui->statusBar->addPermanentWidget(progressBar);
    progressBar->setValue(0);
    progressBar->hide();
}

void MainWindow::setupReceiptViewUI()
{
    // Setup stuff for receipt view
    item = new QGraphicsPixmapItem;
    scene = new QGraphicsScene;
    view = new MyGraphicsView(scene);
    view->setGeometry(100, 100, 800, 500);
}

void MainWindow::setupPlotUI()
{
    expincplot = new MyPlots;
}

void MainWindow::setupGenericUI()
{
    earningspalette.setColor(QPalette::Base,QColor(182, 215, 168, 255));
    ui->totalEarningsLine->setPalette(earningspalette);
    expensespalette.setColor(QPalette::Base,QColor(249, 106, 106, 255));
    ui->totalCostsLine->setPalette(expensespalette);

    // Setup Category QTableWidget
    ui->categoryTableWidget->setColumnCount(4);
    ui->categoryTableWidget->setHorizontalHeaderLabels(QStringList() << tr("Category")
                                                                     << tr("Income")
                                                                     << tr("Expenses")
                                                                     << tr("Total"));

    ui->categoryTableWidget->setSortingEnabled(true);
    ui->categoryTableWidget->sortByColumn(0,Qt::AscendingOrder);
}

void MainWindow::setupSignals()
{
    QObject::connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    QObject::connect(ui->tabWidget, &QTabWidget::currentChanged,
                     this, &MainWindow::checkMenubar);

    // Make row adjust to content every time we change the column width
    // to make word warp work
    // QObject::connect(ui->expensesTableView->horizontalHeader(), &QHeaderView::sectionResized,
    // ui->expensesTableView, &QTableView::resizeRowsToContents);

    QObject::connect(this, SIGNAL(doUpdate()) , this, SLOT(updateCalculations()));
    QObject::connect(this, SIGNAL(doUpdateUI()) , this, SLOT(updateCalculationsUI()));
}

void MainWindow::setupTableViewContectMenu()
{
    showReceiptAct = new QAction("Show receipt", this);
    addReceiptAct = new QAction("Add receipt...", this);
    saveReceiptAct = new QAction("Save receipt...", this);
    removeReceiptAct = new QAction("Remove receipt", this);
    QObject::connect(addReceiptAct, SIGNAL(triggered()), this, SLOT(addReceipt()));
    QObject::connect(showReceiptAct, SIGNAL(triggered()), this, SLOT(showReceipt()));
    QObject::connect(saveReceiptAct, SIGNAL(triggered()), this, SLOT(saveReceipt()));
    QObject::connect(removeReceiptAct, SIGNAL(triggered()), this, SLOT(removeReceipt()));

    menu = new QMenu(this);
    menu->addAction(showReceiptAct);
    menu->addSeparator();
    menu->addAction(addReceiptAct);
    menu->addAction(saveReceiptAct);
    menu->addAction(removeReceiptAct);
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

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
 // if some actions should not be usable, like move, this code must be adopted
    if(isOpen)
        event->acceptProposedAction();
    else
        event->ignore();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
 // if some actions should not be usable, like move, this code must be adopted
    if(isOpen)
        event->acceptProposedAction();
    else
        event->ignore();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    if(isOpen)
        event->accept();
    else
        event->ignore();
}

void MainWindow::addReceipt()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;

    selmodel = ui->expensesTableView->selectionModel();
    current = selmodel->currentIndex();
    int id = expensesmodel->record(proxymodel->mapToSource(current).row()).value("id").toInt();

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
    int id = expensesmodel->record(proxymodel->mapToSource(current).row()).value("id").toInt();

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
        view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
        view->update();

        view->hide();
        view->show();
    } else {
        ui->statusBar->showMessage(tr("Selected entry does not have a saved receipt"), STDSTATUSTIME);
    }
}

void MainWindow::saveReceipt()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;

    selmodel = ui->expensesTableView->selectionModel();
    current = selmodel->currentIndex();
    int id = expensesmodel->record(proxymodel->mapToSource(current).row()).value("id").toInt();

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
           QMessageBox::information(this, "Save receipt - Load description", query.lastError().text());
        }
        query.first();
        QString desc = query.value(0).toString();

        QString fileName = QFileDialog::getSaveFileName(this, tr("Add receipt"),"",
                                                        tr("PNG Image (*.png)"));
        QFile file(fileName);
        if (! file.open(QIODevice::WriteOnly) ) return;

        if (pixmap.save(&file, "PNG"))
            ui->statusBar->showMessage(tr("Receipt saved to ") + fileName, STDSTATUSTIME);
        else
            ui->statusBar->showMessage(tr("Error saving receipt to ") + fileName, STDSTATUSTIME);
    }
}

void MainWindow::removeReceipt()
{
    QItemSelectionModel *selmodel;
    QModelIndex current;

    selmodel = ui->expensesTableView->selectionModel();
    current = selmodel->currentIndex();
    int id = expensesmodel->record(proxymodel->mapToSource(current).row()).value("id").toInt();

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

void MainWindow::checkMenubar()
{
    if(isOpen == true) {
        if( ui->tabWidget->currentIndex() == projectionsID
            || ui->tabWidget->currentIndex() == plotsID  ) // used to be projectionsID
        {
            ui->actionDelete_Entry->setEnabled(false);
            ui->actionNew_Entry->setEnabled(false);
            ui->actionGo_to_Bottom->setEnabled(false);
            ui->actionGo_to_Top->setEnabled(false);
        } else {
            ui->actionDelete_Entry->setEnabled(true);
            ui->actionNew_Entry->setEnabled(true);
            ui->actionGo_to_Bottom->setEnabled(true);
            ui->actionGo_to_Top->setEnabled(true);
        }
    }
}

void MainWindow::expensesRowHeaderChanged(Qt::Orientation orientation, int first, int last)
{
    if(orientation == Qt::Vertical) {
        for(int i=first; i<last+1; i++) {
            if( QString::compare(expensesmodel->headerData(i,Qt::Vertical).toString(), "!") == 0 ) {
                ui->expensesTableView->hideRow(i);
                expensesHiddenRows.append(i);
            }
        }
    }
}

void MainWindow::writeSettings()
{
    GenericHelper::setSettingMainWindowSize(size());
    GenericHelper::setSettingMainWindowPos(pos());
    GenericHelper::setSettingCurrentTab(ui->tabWidget->currentIndex());

    GenericHelper::setSettingDatabaseIsOpen(isOpen);
    GenericHelper::setSettingDatabaseFileName(dbfilename);
}

void MainWindow::readSettings()
{
    resize(GenericHelper::getSettingMainWindowSize());
    move(GenericHelper::getSettingMainWindowPos());
    ui->tabWidget->setCurrentIndex(GenericHelper::getSettingCurrentTab());

    isOpen = GenericHelper::getSettingDatabaseIsOpen();
    dbfilename = GenericHelper::getSettingDatabaseFileName();
}

bool MainWindow::askClose()
{
    if (isOpen)
        if (expensesmodel->isDirty() ) {
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
    QMessageBox::about(this, tr("About Costs - ") + QString::number(VERSION_MAJOR,'g',0) + "."
                                                 + QString::number(VERSION_MINOR,'g',0) + "."
                                                 + QString::number(VERSION_PATCH,'g',0) + "."
                                                 + QString::number(VERSION_BUILD,'g',0),
             tr("<b>Costs</b> should help you "
                "manage your income/expenses and is designed to be "
                "easy to use and simple. It is by far not competing with true accounting software.") + "<br><br>" +
                "Costs Version: " + QString::number(VERSION_MAJOR,'g',0) + "."
                                  + QString::number(VERSION_MINOR,'g',0) + "."
                                  + QString::number(VERSION_PATCH,'g',0) + "."
                                  + QString::number(VERSION_BUILD,'g',0)  );
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

    expensesmodel->setColColors(1, QColor(182, 215, 168, 255)); // set 'Amount' column color
    expensesmodel->setColColors(5, QColor(239, 239, 239, 255)); // set 'Category' column color
    expensesmodel->setColColors(6, QColor(239, 239, 239, 255)); // set 'Payment Method' column color

    proxymodel = new QSortFilterProxyModel;
    proxymodel->setSourceModel(expensesmodel);

    ui->expensesTableView->setModel(proxymodel);

    categoryDelegate = new MySqlRelationalDelegate;
    paymentMethodDelegate = new MySqlRelationalDelegate;

    ui->expensesTableView->setItemDelegateForColumn(5, categoryDelegate);
    ui->expensesTableView->setItemDelegateForColumn(6, paymentMethodDelegate);
    ui->expensesTableView->hideColumn(0); // Don't show id
    ui->expensesTableView->resizeColumnsToContents();

    ui->expensesTableView->setEnabled(true);
    ui->expensesTableView->setSortingEnabled(true);

    expensesmodel->setReadOnly(7, true);

    // Set amount column to number type and to 2 decimal places
    expensesmodel->setNumber(1, true, 2);

    // Connect updateCalculations
    QObject::connect(expensesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);

    QObject::connect(expensesmodel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(expensesRowHeaderChanged(Qt::Orientation,int,int)));

    // Add context menu
    ui->expensesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->expensesTableView, SIGNAL(customContextMenuRequested(QPoint)),
                     this, SLOT(customMenuRequested(QPoint)));

    QObject::connect(ui->expensesTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(tabDoubleClicked(QModelIndex)));

    return 0;
}

void MainWindow::tabDoubleClicked(QModelIndex index)
{
    int col = index.column();

    if(col == 7)
        showReceipt();
}

void MainWindow::customMenuRequested(QPoint pos)
{
    QModelIndex index=ui->expensesTableView->indexAt(pos);
    ui->expensesTableView->selectRow(index.row());

    menu->popup(ui->expensesTableView->viewport()->mapToGlobal(pos));
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

    // Connect updateCalculations
    QObject::connect(categoriesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);

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

    // Connect updateCalculations
    QObject::connect(paymentmethodmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);

    return 0;
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openDatabase(action->data().toString());
}

int MainWindow::openDatabase(QString fileName)
{
    bool wasopen=isOpen;
    if(QFile(fileName).exists()){
        // Backup
        if (GenericHelper::getSettingBackupDatabase()) {
            QFileInfo fileInfo(fileName);
            QDateTime curDateTime = QDateTime::currentDateTime();
            QString curDateTimeString = curDateTime.toString("yyyyMMdd_hhmmss");
            QString backupFileBaseName = fileInfo.baseName();
            QString backupFileSuffix = fileInfo.completeSuffix();
            QString backupFilePath = fileInfo.absolutePath();
            QString backupFileName = backupFilePath + QDir::separator() + backupFileBaseName + "_" + curDateTimeString + "." + backupFileSuffix;

            qDebug() << backupFileName;

            if (! GenericHelper::copyFile(fileName, backupFileName, false)) {
                QMessageBox::warning(this, tr("Costs"),
                                     tr("Error creating Backup of Database: %1")
                                     .arg(fileName));
            }
        }

        sqliteDb1 = new SqliteDatabase(fileName);
        isOpen=false;
        //Show table for expenses
        createExpensesView();

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

        emit doUpdate();

        // Scroll to the end
        ui->expensesTableView->resizeColumnsToContents();
        if(ui->expensesTableView->columnWidth(3) > 0.85*this->width()) {
            ui->expensesTableView->setColumnWidth(3, 0.85*this->width() );
        }
        // ui->expensesTableView->resizeRowsToContents();
        // ui->expensesTableView->scrollToTop();
        ui->expensesTableView->scrollToBottom();

        setCurrentFile(fileName);

        return 0;
    } else {
        QMessageBox::warning(this, tr("Costs"),
                             tr("Database cannot be opened: %1")
                             .arg(fileName));
        isOpen = wasopen;
        return 1;
    }

}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }

    updateRecentFileActions();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(GenericHelper::getCompanyName(), GenericHelper::getAppName());
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::on_actionOpen_Database_triggered()
{
    QString oldpath = GenericHelper::getSettingFileDialogPath();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Database"), oldpath,
                                                    tr("SQLite DB (*.db)"));
    if (!fileName.isEmpty()) {
        openDatabase(fileName);
        GenericHelper::setSettingFileDialogPath(QFileInfo(fileName).absolutePath());
    }
}

void MainWindow::on_actionNew_Database_triggered()
{
    QString oldpath = GenericHelper::getSettingFileDialogPath();

    QString fileName = QFileDialog::getSaveFileName(this, tr("New Database"), oldpath,
                                                    tr("SQLite DB (*.db)"));
    QString errorstr;
    if (!fileName.isEmpty()) {
        int ret = SqliteDatabase::CreateDatabase(fileName, errorstr);
        if( ret > 0) {
            QMessageBox::warning(this, tr("Costs"),
                                 tr("Database cannot be created: %1\n\nError: %2")
                                 .arg(fileName).arg(errorstr));
        } else {
            openDatabase(fileName);
        }

        GenericHelper::setSettingFileDialogPath(QFileInfo(fileName).absolutePath());
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

            // Scroll to the end
            ui->expensesTableView->scrollToBottom();
            break;
    }
}

void MainWindow::uhideAllRows(QTableView *view, QList<qint64> &rowList)
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
        uhideAllRows(ui->expensesTableView, expensesHiddenRows);
        submit(expensesmodel);
        // expensesmodel->select();

    emit doUpdate();
}

bool MainWindow::save()
{
    on_actionSave_triggered();

    return 0;
}

void MainWindow::calcCategory()
{
    // Get all the data
    while(categoriesmodel->canFetchMore())
        categoriesmodel->fetchMore();
    while(expensesmodel->canFetchMore())
        expensesmodel->fetchMore();

    int rowcount = categoriesmodel->rowCount();

    // Fill/Update QMap for categories
    QString curcat;
    calcres.categoryids.clear();
    for (int row=0; row < rowcount; row++) {
        curcat = categoriesmodel->record(row).value("category").toString();
        calcres.categoryids[curcat] = row;
        calcres.categorynames[row] = curcat;
    }

    rowcount = expensesmodel->rowCount();

    qreal curvalue;

    calcres.perCategoryIncome.clear();
    calcres.perCategoryExpenses.clear();
    for (int row=0; row < rowcount; row++) {
        curvalue = expensesmodel->record(row).value("amount").toDouble();
        curcat = expensesmodel->record(row).value("category").toString();
        if( curvalue < 0 ) {
            calcres.perCategoryExpenses[calcres.categoryids[curcat]] += curvalue;
        } else {
            calcres.perCategoryIncome[calcres.categoryids[curcat]] += curvalue;
        }
    }
}

void MainWindow::updateCalculations()
{
    if(isOpen) {
        ui->statusBar->showMessage(tr("Calculating..."));
        qreal totalexpenses=0, totalincome=0;

        // Get all the data
        // while(expensesmodel->canFetchMore())
        //    expensesmodel->fetchMore();

        int pbvalue;

        QDate curdate;
        QDate firstdate = QDate::fromString("2300-12-31","yyyy-MM-dd");
        QDate lastdate = QDate::fromString("1900-01-01","yyyy-MM-dd");

        progressBar->show();

        int expcount=0, earncount=0;
        qreal curvalue=0;

        int rowcount = expensesmodel->rowCount();
        qDebug() << rowcount;
        for (int row=0; row < rowcount; row++) {
            //  progressbar stuff
            pbvalue = 50/(double)rowcount*(row+1);
            progressBar->setValue(pbvalue);

            // curvalue = expensesmodel->record(row).value("amount").toDouble();
            curvalue = expensesmodel->data(expensesmodel->index(row, 1), Qt::DisplayRole).toDouble();
            curdate = QDate::fromString(expensesmodel->data(expensesmodel->index(row, 1), Qt::DisplayRole).toString(),"yyyy-MM-dd");
            // curdate = QDate::fromString(expensesmodel->record(row).value("date").toString(),"yyyy-MM-dd");
            if(curdate<firstdate)
                firstdate = curdate;
            if( curvalue < 0 ) {
                totalexpenses += curvalue;
                expcount++;
            } else {
                totalincome += curvalue;
                earncount++;
            }
        }

        lastdate = QDate::currentDate();

        qint64 dayspassed = firstdate.daysTo(lastdate) + 1;

        if(rowcount == 0)
            dayspassed = 0;

        calcres.totalIncome = totalincome;
        calcres.totalExpenses = totalexpenses;
        calcres.totalNet = calcres.totalIncome + calcres.totalExpenses;
        calcres.perDayIncome = totalincome/(double)dayspassed;
        calcres.perDayExpenses = totalexpenses/(double)dayspassed;;
        calcres.perDayNet = calcres.perDayIncome + calcres.perDayExpenses;
        calcres.perMonthIncome = totalincome/(double)dayspassed*30.5;
        calcres.perMonthExpenses = totalexpenses/(double)dayspassed*30.5;
        calcres.perMonthNet = calcres.perMonthIncome + calcres.perMonthExpenses;
        calcres.perYearIncome = totalincome/(double)dayspassed*365.0;
        calcres.perYearExpenses = totalexpenses/(double)dayspassed*365.0;
        calcres.perYearNet = calcres.perYearIncome + calcres.perYearExpenses;
        calcres.perLineIncome = totalincome/earncount;
        calcres.perLineExpenses = totalexpenses/earncount;;
        calcres.daysPassed = dayspassed;

        calcCategory();
        progressBar->setValue(100);

        updateCalculationsUI();

        progressBar->hide();
        ui->statusBar->showMessage(tr("Calculations updated"), STDSTATUSTIME);
    }

    emit doUpdateUI();
}

void MainWindow::updateCalculationsUI()
{
    ui->totalEarningsLine->setText(QString::number(calcres.totalIncome, 'f', 2));
    ui->totalCostsLine->setText(QString::number(calcres.totalExpenses, 'f', 2));

    if( calcres.totalNet < 0 ) {
        ui->netIncomeLine->setPalette(expensespalette);
    } else {
        ui->netIncomeLine->setPalette(earningspalette);
    }
    ui->netIncomeLine->setText(QString::number(calcres.totalNet, 'f', 2));

    ui->avgDailyCostsLine->setText(QString::number(calcres.perDayNet, 'f', 2));
    ui->avgDailyExpensesLine->setText(QString::number(calcres.perDayExpenses, 'f', 2));
    ui->avgDailyEarningsLine->setText(QString::number(calcres.perDayIncome, 'f', 2));

    ui->expectedCostsPerMonthLine->setText(QString::number(calcres.perMonthNet, 'f', 2));

    ui->expectedTotalCostsLine->setText(QString::number(calcres.perYearNet, 'f', 2));

    ui->daysPassedLine->setText(QString::number(calcres.daysPassed, 'f', 0));

    ui->plotWidget->plotExpInc(ui->totalCostsLine->text().toDouble(), ui->totalEarningsLine->text().toDouble());
    ui->plotWidget->update();

    // Update category table
    ui->categoryTableWidget->setSortingEnabled(false);
    ui->categoryTableWidget->setRowCount(0);
    ui->categoryTableWidget->setRowCount(calcres.categoryids.size());
    for(int c=0; c<calcres.categoryids.size(); c++) {
        ui->categoryTableWidget->setItem(c, 0, new QTableWidgetItem(calcres.categorynames[c]));
        ui->categoryTableWidget->setItem(c, 1, new QTableWidgetItem(QString::number(calcres.perCategoryIncome[c],'f',2)));
        ui->categoryTableWidget->setItem(c, 2, new QTableWidgetItem(QString::number(calcres.perCategoryExpenses[c],'f',2)));
        ui->categoryTableWidget->setItem(c, 3, new QTableWidgetItem(QString::number(calcres.perCategoryIncome[c] + calcres.perCategoryExpenses[c],'f',2)));
    }
    ui->categoryTableWidget->setSortingEnabled(true);
}

void MainWindow::on_actionUpdate_triggered()
{
    emit doUpdate();
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

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Costs"),
                 tr("Warning: Do you really want to delete the selected rows?"),
                 QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
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
}

void MainWindow::on_actionDelete_Entry_triggered()
{
    switch(ui->tabWidget->currentIndex())
    {
    case expensesTabID:
        deleteEntries(expensesmodel, ui->expensesTableView);
        break;
    }

    emit doUpdate();
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
    CategoryConfigDialog dialog;
    dialog.createCategoriesView(sqliteDb1);
    dialog.exec();

    // Update database relations
    expensesmodel->relationModel(5)->select();
}

void MainWindow::on_actionEdit_Payment_Methods_triggered()
{
    // Open Payment Methods Edit dialog
    PaymentMethodsConfigDialog dialog;
    dialog.createPaymentsMethodView(sqliteDb1);
    dialog.exec();

    // Update database relations
    expensesmodel->relationModel(6)->select();
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

            isOpen=false;

            setEnableUIDB(false);

            ui->statusBar->showMessage(tr("Database closed"), STDSTATUSTIME);
        } else {
            // do nothing
        }
    }
}

// CSV handling functions
int MainWindow::getCatId(QString categorystring)
{
    // Search category and return the id
    // Get all the data
  //  while(categoriesmodel->canFetchMore())
   //     categoriesmodel->fetchMore();

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
    }

    return maxid+1;
}

int MainWindow::getPaymentId(QString paymentstring)
{
    // Search payment and return the id
    // Get all the data
   // while(paymentmethodmodel->canFetchMore())
    //    paymentmethodmodel->fetchMore();

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
    }

    return maxid+1;
}

QStringList MainWindow::parseLine(QString line, QString separator)
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
            if(QString::compare(curchar, separator, Qt::CaseSensitive) == 0 ) {
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

    // add the last entry if not empty
    if(! curval.isEmpty()) {
        list.append(curval);
        curval.clear();
    }

    if(quoted)
        qDebug() << "ERROR IN CSV IMPORT, STILL QUOTED";

    return list;
}



void MainWindow::on_actionFrom_CSV_new_triggered()
{
    QString oldpath = GenericHelper::getSettingCSVImportDialogPath();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import CSV"), oldpath,
                                                    tr("CSV (*.csv)"));

    if (!fileName.isEmpty()) {
        CSVImportDialog *dialog = new CSVImportDialog;
        CSVImportDialog::CsvImportParams params;

        dialog->createCSVImportView(fileName);
        if(dialog->exec()) {
            dialog->returnData(params);

            switch(ui->tabWidget->currentIndex())
            {
            case expensesTabID:
                importCSVFile(expensesmodel, fileName, params.columnMap, params.dateformat,
                              params.invert, params.lineskip, params.separator, params.locale);
                ui->expensesTableView->scrollToBottom();
                break;
            }
        }
        delete dialog;
        dialog=0;

        GenericHelper::setSettingCSVImportDialogPath(QFileInfo(fileName).absolutePath());
    }
}

void MainWindow::fileToImportDragged(QString fileName)
{
    if (!fileName.isEmpty()) {
        CSVImportDialog *dialog = new CSVImportDialog;
        CSVImportDialog::CsvImportParams params;

        dialog->createCSVImportView(fileName);
        if(dialog->exec()) {
            dialog->returnData(params);

            switch(ui->tabWidget->currentIndex())
            {
            case expensesTabID:
                importCSVFile(expensesmodel, fileName, params.columnMap, params.dateformat,
                              params.invert, params.lineskip, params.separator, params.locale);
                ui->expensesTableView->scrollToBottom();
                break;
            }
        }
        delete dialog;
        dialog=0;
    }
}

int MainWindow::importCSVFile(MyQSqlRelationalTableModel *model, QString fileName, QMap<int, int> map,
                              QString dateformat, bool invertValues, int lineskip, QString separator, QLocale locale)
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
                     this, &MainWindow::updateCalculations);
    QObject::disconnect(categoriesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);
    QObject::disconnect(paymentmethodmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);

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

                processCSVLine(line, map, dateformat, separator, locale, record);

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
                     this, &MainWindow::updateCalculations);
    QObject::connect(categoriesmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);
    QObject::connect(paymentmethodmodel, &QSqlRelationalTableModel::dataChanged,
                     this, &MainWindow::updateCalculations);

    emit doUpdate();

    return 0;
}

qreal MainWindow::parseValueString(QLocale locale, QString valuestring) {
    // Find out (via brute force) what locale is most likely
//    bool commafirst=false;
//    bool dotfirst=false;
//    for(int i=0; i<valuestring.size(); i++) {
//        if (QString::compare(valuestring.at(i), ",") == 0)
//            commafirst=true;
//    }
//    if(!commafirst)
//        return QLocale(QLocale::German).toDouble(valuestring);
//    else
        return locale.toDouble(valuestring);
}

int MainWindow::processCSVLine(QString line, QMap<int,int> map, QString dateformat, QString separator, QLocale locale, QSqlRecord &record)
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

    QStringList values = parseLine(line, separator);

    record.append(QSqlField("amount", QVariant::Double));
    record.append(QSqlField("date", QVariant::String));
    record.append(QSqlField("description", QVariant::String));
    record.append(QSqlField("what", QVariant::String));
    record.append(QSqlField("category", QVariant::Int));
    record.append(QSqlField("payment", QVariant::Int));

    if( map[amountCol] > 0 ) {
        qreal value = parseValueString(locale, values.value(map[amountCol]-1));
        // record.setValue(0, values.value(map[amountCol]-1).replace(",", ".") );
        record.setValue(0, value );
    }
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
    ui->actionGo_to_Bottom->setEnabled(enable);
    ui->actionGo_to_Top->setEnabled(enable);

    // Disable tableViews until database gets opened
    ui->expensesTableView->setEnabled(enable);
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
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(0, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionDatabase_ID->setChecked(true);
}

void MainWindow::on_actionAmount_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(1, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionAmount->setChecked(true);
}

void MainWindow::on_actionDate_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(2, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionDate->setChecked(true);
}

void MainWindow::on_actionDescription_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(3, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionDescription->setChecked(true);
}
void MainWindow::on_actionWhere_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(4, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionWhere->setChecked(true);
}

void MainWindow::on_actionCategory_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(5, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

    unsetSortChecked();
    ui->actionCategory->setChecked(true);
}

void MainWindow::on_actionPayment_Method_triggered()
{
    QMessageBox msgBox;
    if(expensesmodel->isDirty() ) {
        msgBox.setText("BUG: Before you can sort, you have to save first.");
        msgBox.exec();
        unsetSortChecked();
        ui->actionDatabase_ID->setChecked(true);
        return;
    }
    ui->expensesTableView->sortByColumn(6, Qt::AscendingOrder);
    ui->expensesTableView->setSortingEnabled(false);

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
    int t = expensesmodel->rowCount();
    int b = -1;
    int l = expensesmodel->columnCount();
    int r = -1;

    QList<QModelIndex> indexes = ui->expensesTableView->selectionModel()->selection().indexes();
    foreach(QModelIndex index, indexes) {
        t = qMin(t, index.row());
        b = qMax(b, index.row());
        l = qMin(l, index.column());
        r = qMax(r, index.column());
    }

    if ( r <0 ) return;
    if ( b <0 ) return;

    QString textdata;

    QString data = "<!--Start-->\n";
    data += "<table>";

    for (int row=t ;row<=b; row++) {
        data += "<tr>\n";
        for (int col=l ;col<=r; col++) {
            QVariant v = expensesmodel->data(expensesmodel->index(row,col) );

            if ( v.canConvert(QVariant::Double) ) {
                data += "  <td x:num>";
            } else {
                data += "  <td>";
            }

            data += v.toString();
            data += "</td>\n";

            textdata += v.toString();
            textdata += "\t";
        }
        data += "</tr>\n";
        textdata += "\n";
    }

    data += "</table>";
    data += "<!--End-->\n";

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(textdata);
    mimeData->setHtml(data);
    QApplication::clipboard()->setMimeData(mimeData);
}

void MainWindow::on_actionGo_to_Top_triggered()
{
    // Read all the data
//    while(expensesmodel->canFetchMore())
//        expensesmodel->fetchMore();
    ui->expensesTableView->scrollToTop();
}

void MainWindow::on_actionGo_to_Bottom_triggered()
{
    // Read all the data
//    while(expensesmodel->canFetchMore())
//        expensesmodel->fetchMore();
    ui->expensesTableView->scrollToBottom();
}

void MainWindow::on_actionCut_triggered()
{

}

void MainWindow::on_actionPaste_triggered()
{

}

void MainWindow::on_lineEditFilter_textChanged(const QString &arg1)
{
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setFilterRegExp(arg1);
}
