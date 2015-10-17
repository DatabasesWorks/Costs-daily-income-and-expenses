#include "receiptview.h"
#include "ui_receiptview.h"

ReceiptView::ReceiptView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceiptView)
{
    ui->setupUi(this);
}

ReceiptView::~ReceiptView()
{
    delete ui;
}
