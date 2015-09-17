#ifndef MYQSQLRELATIONALTABLEMODEL_H
#define MYQSQLRELATIONALTABLEMODEL_H

#include <QSqlRelationalTableModel>
#include <QColor>

class MyQSqlRelationalTableModel : public QSqlRelationalTableModel
{
public:
        MyQSqlRelationalTableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());

        Qt::ItemFlags flags(const QModelIndex & index) const;

        void setReadOnly(int col, bool readonly);
        void setColColors(int col, QColor color);

        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

signals:

public slots:

private:
        QMap<int, bool> isReadOnly;
        QMap<int, QColor> colColors;
};

#endif // MYQSQLRELATIONALTABLEMODEL_H
