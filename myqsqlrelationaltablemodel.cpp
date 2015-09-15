#include "myqsqlrelationaltablemodel.h"

#include <QDebug>

MyQSqlRelationalTableModel::MyQSqlRelationalTableModel(QObject * parent, QSqlDatabase db) : QSqlRelationalTableModel(parent, db)
{

}

Qt::ItemFlags MyQSqlRelationalTableModel::flags(const QModelIndex & index) const {
    bool rowisreadonly = isReadOnly.value(index.column(), false);

    if ( rowisreadonly ) {
        return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    } else {
        return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    }
}

void MyQSqlRelationalTableModel::setReadOnly(int column, bool readonly)
{
    isReadOnly[column] = readonly;
}
