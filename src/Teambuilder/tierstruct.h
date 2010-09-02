#ifndef TIERSTRUCT_H
#define TIERSTRUCT_H

#include <QtCore>
#include <QtGui>

struct TierNode
{
    QList<TierNode *> subNodes;
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
    QList<QAction*> buildMenu(QMenu *menu, QObject *c);
    static TierNode* moveInTree(TierNode *lastNode, int levelDiff);
};

#endif // TIERSTRUCT_H
