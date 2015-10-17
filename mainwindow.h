#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QSqlTableModel>
#include <QList>
#include <QProgressBar>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QMap>

#include "myqsqlrelationaltablemodel.h"
#include "databaseapi.h"
#include "myplots.h"
#include "mygraphicsview.h"

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
        qreal totalIncome; // should be positive +
        qreal totalExpenses; // should be negative -
        qreal totalNet; // (totalincome+totalexpenses)
        qreal perDayIncome; // +
        qreal perDayExpenses; // -
        qreal perDayNet;
        qreal perMonthIncome; // +
        qreal perMonthExpenses; // -
        qreal perMonthNet;
        qreal perYearIncome; // +
        qreal perYearExpenses; // -
        qreal perYearNet;
        qreal perLineIncome; // totalIncome normalized by number of incomes
        qreal perLineExpenses; // totalExpenses normalized by number of expenses
        qint64 daysPassed;

        QMap<QString, qint64> categoryids;
        QMap<qint64, QString> categorynames;
        QMap<qint64, qreal> perCategoryIncome;
        QMap<qint64, qreal> perCategoryExpenses;
    };

    enum TabIDs {
        expensesTabID,
        projectionsID,
        plotsID
    };

protected:
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);

public slots:
    void customMenuRequested(QPoint pos);
    void addReceipt();
    void showReceipt();
    void saveReceipt();
    void removeReceipt();

private slots:
    void on_actionAbout_Costs_triggered();
    void on_actionOpen_Database_triggered();
    void on_actionNew_Database_triggered();
    void on_actionNew_Entry_triggered();
    void on_actionSave_triggered();
    void on_actionUpdate_triggered();
    void on_actionDelete_Entry_triggered();
    void on_actionFull_Screen_triggered();
    void on_actionEdit_Categories_triggered();
    void on_actionToggle_Menubar_triggered();
    void on_actionToggle_Toolbar_triggered();
    void on_actionClose_Database_triggered();
    void on_actionEdit_Payment_Methods_triggered();
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
    void on_actionCopy_triggered();
    void on_actionGo_to_Top_triggered();
    void on_actionGo_to_Bottom_triggered();

    void expensesRowHeaderChanged(Qt::Orientation orientation, int first,int last);
    void checkMenubar();
    void updateslot();

    void openRecentFile();

    void tabDoubleClicked(QModelIndex index);

private:
    CalcStruct calcres;

    Ui::MainWindow *ui;

    MyQSqlRelationalTableModel *expensesmodel, *categoriesmodel, *paymentmethodmodel;

    QProgressBar *progressBar;

    bool isOpen;
    QString dbfilename;

    bool isFullscreen;

    bool save();
    bool askClose();

    void setupSignals();
    void setupTableViewContectMenu();

    int openDatabase(QString fileName);

    void setEnableUIDB(bool enable);

    void writeSettings();
    void readSettings();
    void closeEvent(QCloseEvent *event);

    void calcCategory();
    void updateCalculations();
    void updateCalculationsUI();

    void submit(MyQSqlRelationalTableModel *model);

    void uhideAllRows(QTableView *view, QList<qint64> &rowList);

    int createExpensesView();
    int createCategoriesView();
    int createPaymentsView();

    void unsetSortChecked();

    void deleteEntries(MyQSqlRelationalTableModel *model, QTableView *view);

    // CSV handling functions
    int importCSVFile(MyQSqlRelationalTableModel *model, QString fileName, QMap<int, int> map, QString dateformat,
                      bool invertValues, int lineskip, QString separator, QLocale locale);
    int processCSVLine(QString line, QMap<int,int> map, QString dateformat, QString separator, QLocale locale, QSqlRecord &record);
    QStringList parseLine(QString line, QString separator);
    int getCatId(QString categorystring);
    int getPaymentId(QString paymentstring);
    void fileToImportDragged(QString fileName);
    qreal parseValueString(QLocale locale, QString valuestring);

    QList<qint64> expensesHiddenRows;

    MyPlots *expincplot;

    QPalette earningspalette;
    QPalette expensespalette;

    // Context menu for TableView
    QAction *showReceiptAct;
    QAction *addReceiptAct;
    QAction *saveReceiptAct;
    QAction *removeReceiptAct;
    QMenu *menu;

    // Receipt view
    QGraphicsPixmapItem *item;
    QGraphicsScene *scene;
    MyGraphicsView *view;

    // recent file list
    QString strippedName(const QString &fullFileName);
    enum { MaxRecentFiles = 4 };
    QAction *recentFileActs[MaxRecentFiles];
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString curFile;
};

#endif // MAINWINDOW_H
