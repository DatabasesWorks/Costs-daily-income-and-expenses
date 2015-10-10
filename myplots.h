#ifndef MYPLOTS_H
#define MYPLOTS_H

#include <QObject>
#include <QWidget>

class MyPlots : public QWidget
{
    Q_OBJECT
public:
    explicit MyPlots(QWidget *parent = 0);
    void plotExpInc(double exp, double inc);

protected:
    void paintEvent(QPaintEvent *);

signals:

public slots:

private:
    qreal totexpenses;
    qreal totincome;
};

#endif // MYPLOTS_H
