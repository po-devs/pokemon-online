#ifndef TIERWINDOW_H
#define TIERWINDOW_H

#include <QtGui>
#include "tiertree.h"

class ConfigForm;

class TierWindow : public QWidget
{
    Q_OBJECT
public:
    TierWindow(QWidget *parent = NULL);
    ~TierWindow();

    void openTierEdit(Tier *t);
    void openCategoryEdit(TierCategory *c);
public slots:
    void editingRequested(QTreeWidgetItem *item);
signals:
    void tiersChanged();
private slots:
    void done();
private:
    QTreeWidget *m_tree;
    QWidget *configWidget;

    TierTree *dataTree;

    enum Type {
        TierT,
        CategoryT
    };

    ConfigForm *helper;
    QString currentEdit;

    void clearCurrentEdit();
};


#endif // TIERWINDOW_H
