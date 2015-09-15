#ifndef MYQSQLRELATIONALTABLEMODEL_H
#define MYQSQLRELATIONALTABLEMODEL_H

#include <QSqlRelationalTableModel>

class MyQSqlRelationalTableModel : public QSqlRelationalTableModel
{
public:
        MyQSqlRelationalTableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());

        Qt::ItemFlags flags(const QModelIndex & index) const;

        void setReadOnly(int column, bool readonly);

signals:

public slots:

private:
        QMap<int, bool> isReadOnly;
};

#endif // MYQSQLRELATIONALTABLEMODEL_H
