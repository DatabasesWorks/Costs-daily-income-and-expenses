#ifndef RECEIPTVIEW_H
#define RECEIPTVIEW_H

#include <QWidget>

namespace Ui {
class ReceiptView;
}

class ReceiptView : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiptView(QWidget *parent = 0);
    ~ReceiptView();

private:
    Ui::ReceiptView *ui;
};

#endif // RECEIPTVIEW_H
