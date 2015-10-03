#include "myqsqlrelationaltablemodel.h"

#include <QDebug>
#include <QSqlRecord>
#include <QPixmap>
#include <QFile>

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
        if(index.column() == 1)
            if(record(index.row()).value(1).toReal()<0)
            rowcol = QColor(QColor(249, 106, 106, 255));
        return QVariant(rowcol);
    }

    // Handle the receipt column (shows a nice icon if there is a stored receipt)
    if(index.column() == 7) {
        QString imgFile = ":/16x16/icons/16x16/camera-photo.png";
        if ( !QFile::exists( imgFile ) )
             qDebug() << "File not found";
        QPixmap pixmap( imgFile );
        if(role == Qt::DecorationRole) {
            if(! record(index.row()).value(7).isNull()) {
                return pixmap.scaledToHeight(16);
            } else {
                return QString();
            }
        }
        if(role == Qt::DisplayRole) {
            return QString();
        }
        if(role == Qt::SizeHintRole) {
            return pixmap.size();
        }
    }

    // If nothing else matches give back the initial object stored in data
    return QSqlRelationalTableModel::data(index,role);
}
