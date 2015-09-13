#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>

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

private slots:
    void on_actionAbout_Costs_triggered();
    void on_actionOpen_Database_triggered();
    void on_actionNew_Database_triggered();
    void on_actionNew_Entry_triggered();
    void on_actionSave_triggered();

    void on_actionUpdate_triggered();

private:
    Ui::MainWindow *ui;

    SqliteDatabase *sqliteDb1;
    QSqlTableModel *categoriesmodel;
    QSqlRelationalTableModel *expensesmodel, *monthlyexpensesmodel;

    bool isOpen;
    QString dbfilename;

    int openDatabase(QString fileName);

    void writeSettings();
    void readSettings();
    void closeEvent(QCloseEvent *event);

    void updateCalculations();
};

#endif // MAINWINDOW_H
