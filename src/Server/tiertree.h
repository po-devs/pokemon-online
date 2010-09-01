#ifndef TIERTREE_H
#define TIERTREE_H

#include <QtCore>

class Tier;
class TierMachine;
class QDomElement;
class QTreeWidget;
class QTreeWidgetItem;

struct TierCategory
{
    QList<Tier *> subLeafs;
    QList<TierCategory *> subCategories;
    QString name;
    bool root;

    ~TierCategory();
    void loadFromXml(const QDomElement &xmldata, TierMachine *boss, bool root=false);
    QDomElement &toXml(QDomElement &xml) const;

    QList<Tier *> gatherTiers();
    void cleanCategories();
    void clear();
    void clearWithoutDeleting();
    void kill(Tier *t);
    TierCategory *dataClone() const;

    void serialize(QDataStream &stream, int level = -1);

    Tier *getTier(const QString &name);
    TierCategory *getCategory(const QString &name);


    QTreeWidgetItem *buildGui();
    void buildRootGui(QTreeWidget *tree);
};

class TierTree
{
    friend class TierMachine;
public:
    void loadFromXml(const QString &xmldata, TierMachine *boss);
    QString toXml() const;
    QList<Tier *> gatherTiers();
    /* Removing branches without leafs */
    void cleanCategories();
    /* Building the tier list as sent over network */
    QByteArray buildTierList();
    /* Returns a copy, but that has no effect on ratings, just used to represent and touch
       the structure of tiers without harming the real one */
    TierTree *dataClone() const;

    Tier *getTier(const QString &name);
    TierCategory *getCategory(const QString &name);

    void buildTreeGui(QTreeWidget *tree);
private:
    TierCategory root;
};

#endif // TIERTREE_H
