#ifndef TIERWINDOW_H
#define TIERWINDOW_H

#include <QtGui>
#include "tiertree.h"

class TierWindow : public QWidget
{
    Q_OBJECT
public:
    TierWindow(QWidget *parent = NULL);
signals:
    void tiersChanged();
private slots:
    void done();
private:
    QTreeWidget *m_tree;
    QWidget *configWidget;

    TierTree dataTree;
};


#endif // TIERWINDOW_H
