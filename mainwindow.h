#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
// #include <QSqlRelationalTableModel>

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

    void on_actionQuit_triggered();

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

private:
    CalcStruct calcres;

    Ui::MainWindow *ui;

    MyQSqlRelationalTableModel *expensesmodel, *monthlyexpensesmodel, *categoriesmodel;

    bool isOpen;
    QString dbfilename;

    bool isFullscreen;

    bool save();
    bool askClose();

    void setupSignals();

    int openDatabase(QString fileName);

    void writeSettings();
    void readSettings();
    void closeEvent(QCloseEvent *event);

    void updateCalculations();

    void submit(MyQSqlRelationalTableModel *model);

    void uhideAllRows(QTableView *view);

    int createExpensesView();
    int createMonthlyExpensesView();
    int createCategoriesView();

    void openDatabaseDialog(QString &filename);
};

#endif // MAINWINDOW_H
