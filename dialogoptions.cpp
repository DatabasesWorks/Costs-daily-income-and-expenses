#include "dialogoptions.h"
#include "ui_dialogoptions.h"

#include <QFileDialog>

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);
}

DialogOptions::~DialogOptions()
{
    delete ui;
}

//void DialogOptions::setDialogShown()
//{
//    dialogShown = true;
//    this->refreshUiData();
//}

//bool DialogOptions::getDialogShown()
//{
//    this->refreshUiData();
//    return dialogShown;
//}

//void DialogOptions::refreshUiData()
//{
//    this->ui->checkBoxCloseToTray->setChecked(genericHelper::getCloseToTray());

//    restoreGeometry(genericHelper::getGeometry("options").toByteArray());
//}

//void DialogOptions::on_pushButtonOk_clicked()
//{
//    if (this->ui->checkBoxUpdateCheck->checkState() == Qt::Checked) {

//        genericHelper::setCheckUpdate(true);
//    } else {
//        genericHelper::setCheckUpdate(false);
//    }

//    this->hide();
//    genericHelper::saveGeometry("options",saveGeometry());

//    emit settingsSaved();
//}

//void DialogOptions::on_pushButtonCancel_clicked()
//{
//    this->hide();
//    genericHelper::saveGeometry("options", saveGeometry());
//}
