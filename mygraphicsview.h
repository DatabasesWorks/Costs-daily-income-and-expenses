#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsScene>

class MyGraphicsView : public QGraphicsView
{
public:
    MyGraphicsView(QGraphicsScene * scene, QWidget * parent = 0);

protected:
    void resizeEvent(QResizeEvent *event)
    {
        fitInView(sceneRect(), Qt::KeepAspectRatio);

        QGraphicsView::resizeEvent(event);
    }

signals:

public slots:
};

#endif // MYGRAPHICSVIEW_H
