#include "csvimportdialog.h"
#include "ui_csvimportdialog.h"

#include <QMap>
#include <QString>
#include <QFile>
#include <QDate>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

CSVImportDialog::CSVImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSVImportDialog)
{
    ui->setupUi(this);

    readSettings();
}

CSVImportDialog::~CSVImportDialog()
{
    writeSettings();
    delete ui;
}

void CSVImportDialog::createCSVImportView(QString filenamein)
{
    QFile file(filenamein);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // read file
        ui->csvTextEdit->setText(file.readAll());
    }

    QTextCursor cursor = ui->csvTextEdit->textCursor();
    QTextCharFormat format;

    cursor.select(QTextCursor::Document);
    format.setForeground( QBrush( QColor( "black" ) ) );
    format.setFontStrikeOut(false);
    cursor.setCharFormat( format );

    for(int i=0; i<ui->lineskipSpinBox->value(); i++){
        format.setForeground( QBrush( QColor( "red" ) ) );
        format.setFontStrikeOut(true);
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, i);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.setCharFormat( format );
    }

}

void CSVImportDialog::returnData(QMap<int, int> &columnMap, int &lineskipret, QString &dateformatret, bool &invert)
{
    columnMap = cmap;
    lineskipret = lineskip;
    dateformatret = ui->dateFormatEdit->text();
    invert = invertValue;
}

void CSVImportDialog::on_importButton_clicked()
{
    QMessageBox msgBox;
    if(ui->amountCheck->checkState()) {
        if(QString(ui->amountEdit->text()).toFloat() > 0) {
            cmap.insert(0,QString(ui->amountEdit->text()).toFloat());
        } else {
            msgBox.setText("Error: Column 'Amount' selected but no column number specified.");
            msgBox.exec();
            return;
        }
    } else {
        cmap.insert(0,-1);
    }
    if(ui->dateCheck->checkState()) {
        if(QString(ui->dateEdit->text()).toFloat() > 0) {
            cmap.insert(1,QString(ui->dateEdit->text()).toFloat());
        } else {
            msgBox.setText("Error: Column 'Date' selected but no column number specified.");
            msgBox.exec();
            return;
        }
    } else {
        cmap.insert(1,-1);
    }
    if(ui->descriptionCheck->checkState()) {
        if(QString(ui->descriptionEdit->text()).toFloat() > 0) {
            cmap.insert(2,QString(ui->descriptionEdit->text()).toFloat());
        } else {
            msgBox.setText("Error: Column 'Description' selected but no column number specified.");
            msgBox.exec();
            return;
        }
    } else {
        cmap.insert(2,-1);
    }
    if(ui->whatCheck->checkState()) {
        if(QString(ui->whatEdit->text()).toFloat() > 0) {
            cmap.insert(3,QString(ui->whatEdit->text()).toFloat());
        } else {
            msgBox.setText("Error: Column 'What/Where' selected but no column number specified.");
            msgBox.exec();
            return;
        }
    } else {
        cmap.insert(3,-1);
    }
    if(ui->categoryCheck->checkState()) {
        if(QString(ui->categoryEdit->text()).toFloat() > 0) {
            cmap.insert(4,QString(ui->categoryEdit->text()).toFloat());
        } else {
            msgBox.setText("Error: Column 'Category' selected but no column number specified.");
            msgBox.exec();
            return;
        }
    } else {
        cmap.insert(4,-1);
    }
    if(ui->paymentCheck->checkState()) {
        if(QString(ui->paymentEdit->text()).toFloat() > 0) {
            cmap.insert(5,QString(ui->paymentEdit->text()).toFloat());
        } else {
            msgBox.setText("Error: Column 'Payment' selected but no column number specified.");
            msgBox.exec();
            return;
        }
    } else {
        cmap.insert(5,-1);
    }

    lineskip = ui->lineskipSpinBox->value();

    invertValue = ui->invertCheck->checkState();

    this->accept();
}

void CSVImportDialog::on_cancelButton_clicked()
{
    this->reject();
}

void CSVImportDialog::on_lineskipSpinBox_valueChanged(int arg1)
{
    QTextCursor cursor = ui->csvTextEdit->textCursor();
    QTextCharFormat format;

    cursor.select(QTextCursor::Document);
    format.setForeground( QBrush( QColor( "black" ) ) );
    format.setFontStrikeOut(false);
    cursor.setCharFormat( format );

    for(int i=0; i<arg1; i++){
        format.setForeground( QBrush( QColor( "red" ) ) );
        format.setFontStrikeOut(true);
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, i);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.setCharFormat( format );
    }
}

void CSVImportDialog::writeSettings()
{
    QSettings settings("Abyle Org", "Costs");

    settings.beginGroup("CSVImportDialog");
    settings.setValue("amountChecked", ui->amountCheck->checkState());
    settings.setValue("dateChecked", ui->dateCheck->checkState());
    settings.setValue("descriptionChecked", ui->descriptionCheck->checkState());
    settings.setValue("whatChecked", ui->whatCheck->checkState());
    settings.setValue("categoryChecked", ui->categoryCheck->checkState());
    settings.setValue("paymentChecked", ui->paymentCheck->checkState());

    settings.setValue("amountValue", ui->amountEdit->text());
    settings.setValue("dateValue", ui->dateEdit->text());
    settings.setValue("descriptionValue", ui->descriptionEdit->text());
    settings.setValue("whatValue", ui->whatEdit->text());
    settings.setValue("categoryValue", ui->categoryEdit->text());
    settings.setValue("paymentValue", ui->paymentEdit->text());

    settings.setValue("lineskip", ui->lineskipSpinBox->value());

    settings.setValue("invertChecked", ui->invertCheck->checkState());

    settings.setValue("dateformat", ui->dateFormatEdit->text());
    settings.endGroup();
}

void CSVImportDialog::readSettings()
{
    QSettings settings("Abyle Org", "Costs");

    settings.beginGroup("CSVImportDialog");
    ui->amountCheck->setChecked(settings.value("amountChecked", true).toBool());
    ui->dateCheck->setChecked(settings.value("dateChecked", true).toBool());
    ui->descriptionCheck->setChecked(settings.value("descriptionChecked", true).toBool());
    ui->whatCheck->setChecked(settings.value("whatChecked", true).toBool());
    ui->categoryCheck->setChecked(settings.value("categoryChecked", true).toBool());
    ui->paymentCheck->setChecked(settings.value("paymentChecked", true).toBool());

    ui->amountEdit->setText(settings.value("amountValue", qint8(1)).toString());
    ui->dateEdit->setText(settings.value("dateValue", qint8(2)).toString());
    ui->descriptionEdit->setText(settings.value("descriptionValue", qint8(3)).toString());
    ui->whatEdit->setText(settings.value("whatValue", qint8(4)).toString());
    ui->categoryEdit->setText(settings.value("categoryValue", qint8(5)).toString());
    ui->paymentEdit->setText(settings.value("paymentValue", qint8(6)).toString());

    ui->lineskipSpinBox->setValue(settings.value("lineskip", qint8(4)).toInt());

    ui->invertCheck->setChecked(settings.value("invertChecked", false).toBool());

    ui->dateFormatEdit->setText(settings.value("dateformat","M/d/yyyy").toString());

    settings.endGroup();
}

void CSVImportDialog::on_defaultsButton_clicked()
{
    ui->amountCheck->setChecked(true);
    ui->dateCheck->setChecked(true);
    ui->descriptionCheck->setChecked(true);
    ui->whatCheck->setChecked(true);
    ui->categoryCheck->setChecked(true);
    ui->paymentCheck->setChecked(true);

    ui->amountEdit->setText("1");
    ui->dateEdit->setText("2");
    ui->descriptionEdit->setText("3");
    ui->whatEdit->setText("4");
    ui->categoryEdit->setText("5");
    ui->paymentEdit->setText("6");

    ui->lineskipSpinBox->setValue(4);

    ui->dateFormatEdit->setText("M/d/yyyy");

}
