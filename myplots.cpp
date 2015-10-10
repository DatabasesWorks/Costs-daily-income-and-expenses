#include "myplots.h"

#include <QPainter>
#include <QDebug>
#include <QtMath>

MyPlots::MyPlots(QWidget *parent) : QWidget(parent)
{
    totexpenses=0;
    totincome=0;

    // set white background
    QPalette p(palette());
    p.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(p);
}

void MyPlots::plotExpInc(double exp, double inc)
{
    totexpenses=exp;
    totincome=inc;
}

void MyPlots::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Define new screen dimensions (-5% of real dimensions)
    qreal x0=2.0*width()/20.0, x1=width()-x0;
    qreal y0=height()/20.0, y1=height()-2.0*y0;

    // Draw rectangle filling half the height
    // and 10% of width in the middle of the screen
    qreal h1, h2;
    if( qFabs(totincome) > qFabs(totexpenses) ) {
        h1=y1-y0;
        h2=qFabs(h1*(totexpenses/totincome));
    } else {
        h2=y1-y0;
        h1=qFabs(h2*(totincome/totexpenses));
    }

    double w=(x1-x0)/4.0;

    // Income rect
    QRect r1(x0, y1-h1, w, h1);
    // Expenses rect
    QRect r2(x1-w, y1-h2, w, h2);

    // Income text rect
    QRect r1text(x0, y1, w, 2.0*y0);
    // Expenses text rect
    QRect r2text(x1-w, y1, w, 2.0*y0);

    qreal fsize=height()/600.0*12;

    painter.setFont(QFont("Arial", fsize));
    painter.setPen(Qt::black);
    painter.fillRect(r1, QColor(182, 215, 168, 255));
    painter.drawRect(r1);
    painter.drawText(r1text, Qt::AlignCenter, QString::number(totincome, 'f', 2) + "\nIncome");
    painter.fillRect(r2, QColor(249, 106, 106, 255));
    painter.drawRect(r2);
    painter.drawText(r2text, Qt::AlignCenter, QString::number(totexpenses, 'f', 2) + "\nExpenses");

//    // Draw Top text
//    fsize=height()/600.0*16;
//    QRect toptextrect(0,0,width(),y0);
//    painter.setFont(QFont("Arial", fsize));
//    painter.setPen(Qt::black);
//    painter.drawText(toptextrect, Qt::AlignCenter, "Total income vs. Total expenses");

}
