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

void MyQSqlRelationalTableModel::setReadOnly(int col, bool readonly)
{
    // Sets the specified column col readonly
    isReadOnly[col] = readonly;
}

void MyQSqlRelationalTableModel::setColColors(int col, QColor color)
{
    // Sets the background color of the specified column col
    colColors[col] = color;
}

QVariant MyQSqlRelationalTableModel::data ( const QModelIndex & index, int role ) const
{
    // If the role for which the data is requested is Qt::BackgroundColorRole, give back our
    // defined color for specific columns
    if(role==Qt::BackgroundColorRole)
    {
        QColor rowcol = colColors.value(index.column(), QColor(Qt::white));
        return QVariant(rowcol);
    }
    // If not, give back the initial object stored in data
    return QSqlRelationalTableModel::data(index,role);
}
