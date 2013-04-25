#ifndef TIERWINDOW_H
#define TIERWINDOW_H

class QRadioButton;

#include "tiertree.h"
#include "../PokemonInfo/battlestructs.h"

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
private slots:
    void done();
    void updateTier();
    void updateCategory();
    void addNew();
    void addNewTier(const QString &name);
    void addNewCategory(const QString &name);
    void deleteCurrent();
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
    bool clauses[ChallengeInfo::numberOfClauses];

    void updateInternalWidget();
    void updateTree();
};

class TierWNameW : public QWidget
{
    Q_OBJECT
public:
    TierWNameW();

signals:
    void addNewTier(const QString &name);
    void addNewCategory(const QString &name);

private slots:
    void addClicked();

private:
    QLineEdit *line;
    QRadioButton *bTier, *bCat;
};


#endif // TIERWINDOW_H
