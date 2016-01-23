#ifndef MYSQLRELATIONALDELEGATE_H
#define MYSQLRELATIONALDELEGATE_H

#include <QSqlRelationalDelegate>

class MySqlRelationalDelegate : public QSqlRelationalDelegate
{
 Q_OBJECT
public:
 explicit MySqlRelationalDelegate(QObject *parent = 0);

QWidget *createEditor(QWidget *aParent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
 void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
 void setEditorData(QWidget *editor, const QModelIndex &index) const;

signals:

public slots:

};

#endif // MYSQLRELATIONALDELEGATE_H
