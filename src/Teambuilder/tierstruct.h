#ifndef TIERSTRUCT_H
#define TIERSTRUCT_H

#include <QtCore>
#include <QtGui>

struct TierNode
{
    QList<TierNode *> subNodes;
    /* Reflects the list of categories to go through before accessing a tier */
    QHash<QString, QVector<QString> > *pathToTiers;
    TierNode *parent;
    QString name;

    TierNode(const QString &s = "");
    ~TierNode();
    void clean();
    QStringList getTierList();
    bool isLeaf() const;

    void buildFromRaw(QByteArray raw);
    /* Builds on a tree and returns the tier items */
    QHash<QString, QTreeWidgetItem*> buildOnTree(QTreeWidget *tree);
    QHash<QString, QTreeWidgetItem*> buildSelf(QTreeWidgetItem *parent);
    QTreeWidgetItem *addTier(QTreeWidgetItem *category, const QString &tier);
    QTreeWidgetItem *addNode(QTreeWidgetItem *category, const QString &tier);
    QList<QAction*> buildMenu(QMenu *menu, QObject *c);
    TierNode *subNode(const QString name);
    static TierNode* moveInTree(TierNode *lastNode, int levelDiff);
};

#endif // TIERSTRUCT_H
