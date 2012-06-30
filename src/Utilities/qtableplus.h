#ifndef QTABLEPLUS_H
#define QTABLEPLUS_H

#include <QTableView>

class QTablePlus : public QTableView
{
    Q_OBJECT
public:
    QTablePlus(QWidget *parent=0);
signals:
    void currentCellChanged(QModelIndex);
protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // QTABLEPLUS_H
