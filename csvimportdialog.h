#ifndef CSVIMPORTDIALOG_H
#define CSVIMPORTDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSqlRecord>

#include "myqsqlrelationaltablemodel.h"

namespace Ui {
class CSVImportDialog;
}

class CSVImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CSVImportDialog(QWidget *parent = 0);
    ~CSVImportDialog();

    void createCSVImportView(QString filenamein);
    void returnData(QMap<int, int> &columnMap, int &lineskipret, QString &dateformatret);

private slots:
    void on_importButton_clicked();

    void on_cancelButton_clicked();

    void on_lineskipSpinBox_valueChanged(int arg1);

    void on_defaultsButton_clicked();

private:
    Ui::CSVImportDialog *ui;

    QMap<int, int> cmap;

    int processCSVLine(QString line, QSqlRecord &record);
    QStringList parseLine(QString line);

    MyQSqlRelationalTableModel *importmodel;

    int lineskip;

    void writeSettings();
    void readSettings();

};

#endif // CSVIMPORTDIALOG_H
