#include "qtableplus.h"

QTablePlus::QTablePlus(QWidget *parent)
    : QTableView(parent)
{

}

void QTablePlus::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);

    emit currentCellChanged(current);
}
