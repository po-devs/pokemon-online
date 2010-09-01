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
    void updateTier();
    void updateCategory();
private:
    QTreeWidget *m_tree;
    QWidget *configWidget;
    QWidget *internalWidget;

    TierTree *dataTree;

    enum Type {
        TierT,
        CategoryT
    };

    ConfigForm *helper;
    QString currentEdit;

    void clearCurrentEdit();
    /* Data used to configure categories / tiers */
    QString parent;
    QString pokemons, restrPokemons, moves, items;
    TierCategory *currentTierCat;
    Tier *currentTier;
    Type currentType;

    void updateInternalWidget();
    void updateTree();
};


#endif // TIERWINDOW_H
