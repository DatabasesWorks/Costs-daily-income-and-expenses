#ifndef SQLACCOUNTTABLEMODEL_H
#define SQLACCOUNTTABLEMODEL_H

#include <QAbstractTableModel>

class SqlAccountTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SqlAccountTableModel(QObject * parent = 0);
    ~SqlAccountTableModel();

    Qt::ItemFlags flags(const QModelIndex & index) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole);

    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
    bool insertColumns(int column, int count, const QModelIndex & parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex & parent = QModelIndex());

};

#endif // SQLACCOUNTTABLEMODEL_H
