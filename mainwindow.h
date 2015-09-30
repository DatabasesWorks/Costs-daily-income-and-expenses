#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QList>
#include <QProgressBar>

#include "myqsqlrelationaltablemodel.h"
#include "databaseapi.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    SqliteDatabase *sqliteDb1;

    struct CalcStruct {   // Declare struct to hold calculated results
        double total;
    };

    enum TabIDs {
        expensesTabID,
        earningsTabID,
        monthlyExpensesTabID,
        monthlyEarningsTabID,
        projectionsID,
        categoriesID
    };

private slots:
    void on_actionAbout_Costs_triggered();
    void on_actionOpen_Database_triggered();
    void on_actionNew_Database_triggered();
    void on_actionNew_Entry_triggered();
    void on_actionSave_triggered();

    void on_actionUpdate_triggered();

    void checkMenubar();
    void updateslot();

    void on_actionDelete_Entry_triggered();

    void on_actionFull_Screen_triggered();

    void on_actionEdit_Categories_triggered();

    void on_actionToggle_Menubar_triggered();

    void on_actionToggle_Toolbar_triggered();

    void on_actionClose_Database_triggered();

    void on_actionEdit_Payment_Methods_triggered();

    void expensesRowHeaderChanged(Qt::Orientation orientation, int first,int last);
    void earningsRowHeaderChanged(Qt::Orientation orientation, int first,int last);
    void monthlyExpensesRowHeaderChanged(Qt::Orientation orientation, int first,int last);
    void monthlyEarningsRowHeaderChanged(Qt::Orientation orientation, int first,int last);

    void on_actionDate_triggered();

    void on_actionDatabase_ID_triggered();

    void on_actionFrom_CSV_new_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionAmount_triggered();

    void on_actionDescription_triggered();

    void on_actionCategory_triggered();

    void on_actionPayment_Method_triggered();

    void on_actionWhere_triggered();

    void on_actionReport_Bug_triggered();

private:
    CalcStruct calcres;

    Ui::MainWindow *ui;

    MyQSqlRelationalTableModel *expensesmodel, *monthlyexpensesmodel, *earningsmodel, *monthlyearningsmodel, *categoriesmodel, *paymentmethodmodel;

    QProgressBar *progressBar;

    bool isOpen;
    QString dbfilename;

    bool isFullscreen;

    bool save();
    bool askClose();

    void setupSignals();

    int openDatabase(QString fileName);

    void setEnableUIDB(bool enable);

    void writeSettings();
    void readSettings();
    void closeEvent(QCloseEvent *event);

    void updateCalculations();

    void submit(MyQSqlRelationalTableModel *model);

    void uhideAllRows(QTableView *view, QList<qint8> &rowList);

    int createExpensesView();
    int createEarningsView();
    int createMonthlyExpensesView();
    int createMonthlyEarningsView();
    int createCategoriesView();
    int createPaymentsView();

    QList<qint8> expensesHiddenRows, monthlyExpensesHiddenRows, earningsHiddenRows, monthlyEarningsHiddenRows;

    void unsetSortChecked();

    void deleteEntries(MyQSqlRelationalTableModel *model, QTableView *view);

    // CSV handling functions
    int importCSVFile(MyQSqlRelationalTableModel *model, QString fileName, QMap<int, int> map, QString dateformat);
    int processCSVLine(QString line, QMap<int,int> map, QString dateformat, QSqlRecord &record);
    QStringList parseLine(QString line);
    int getCatId(QString categorystring);
    int getPaymentId(QString paymentstring);
};

#endif // MAINWINDOW_H
