#ifndef MYQSQLRELATIONALTABLEMODEL_H
#define MYQSQLRELATIONALTABLEMODEL_H

#include <QSqlRelationalTableModel>
#include <QColor>

class MyQSqlRelationalTableModel : public QSqlRelationalTableModel
{
public:
        MyQSqlRelationalTableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());

        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags(const QModelIndex & index) const;

        void setReadOnly(int col, bool readonly);
        void setColColors(int col, QColor color);
        void setNumber(int col, bool isnumber, int prec = 6);

signals:

public slots:

private:
        QMap<int, bool> isReadOnly;
        QMap<int, QColor> colColors;
        QMap<int, bool> isNumber;
        QMap<int, int> numberPrec;
};

#endif // MYQSQLRELATIONALTABLEMODEL_H
