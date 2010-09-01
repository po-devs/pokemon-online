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

    void openTierEdit();
public slots:
    editingRequested(QTreeWidgetItem *item);
signals:
    void tiersChanged();
private slots:
    void done();
private:
    QTreeWidget *m_tree;
    QWidget *configWidget;

    TierTree dataTree;

    enum Type {
        Tier,
        Category
    };

    ConfigForm *helper;
    QString currentEdit;

    void clearCurrentEdit();
};


#endif // TIERWINDOW_H
