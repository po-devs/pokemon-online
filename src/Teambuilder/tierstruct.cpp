#include "tierstruct.h"

TierNode::TierNode(const QString &s) : name(s)
{
    parent = NULL;
}

TierNode::~TierNode()
{
    clean();
}

void TierNode::clean()
{
    foreach(TierNode *n, subNodes) {
        delete n;
    }
    subNodes.clear();
}

void TierNode::buildFromRaw(QByteArray raw)
{
    clean();

    QDataStream stream(&raw, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    int lastLevel = 0;
    uchar currentLevel = 0;
    TierNode *lastNode = this;

    while (!stream.atEnd()) {
        stream >> currentLevel;

        lastNode = moveInTree(lastNode, currentLevel-lastLevel);
        lastLevel = currentLevel;

        QString name;
        stream >> name;

        TierNode *n = new TierNode(name);
        n->parent = lastNode;
        lastNode->subNodes.push_back(n);
    }
}

TierNode * TierNode::moveInTree(TierNode *lastNode, int levelDiff)
{
    qDebug() << "Moving " << levelDiff << " from " << lastNode->name;

    if (levelDiff == 0)
        return lastNode;

    if (levelDiff > 0) {
        if (lastNode->subNodes.empty())
            return lastNode;
        return moveInTree(lastNode->subNodes.last(), levelDiff -1);
    }

    //levelDiff < 0
    if (lastNode->parent == NULL)
        return lastNode;
    return moveInTree(lastNode->parent, levelDiff+1);
}

QHash<QString, QTreeWidgetItem*> TierNode::buildOnTree(QTreeWidget *tree)
{
    tree->clear();

    QHash<QString, QTreeWidgetItem*> ret;

    foreach(TierNode *n, subNodes) {
        ret.unite(n->buildSelf(tree->invisibleRootItem()));
    }

    return ret;
}

QHash<QString, QTreeWidgetItem *> TierNode::buildSelf(QTreeWidgetItem *root)
{
    QHash<QString, QTreeWidgetItem*> ret;

    QTreeWidgetItem *it = new QTreeWidgetItem(QStringList() << name);
    root->addChild(it);

    QFont f = it->font(0);
    f.setPixelSize(14);
    it->setFont(0,f);

    if (!isLeaf()) {
        foreach(TierNode *t, subNodes) {
            ret.unite(t->buildSelf(it));
        }
    } else {
        ret.insert(name, it);
    }

    return ret;
}

QStringList TierNode::getTierList()
{
    QStringList ret;

    foreach(TierNode *n, subNodes) {
        if (n->isLeaf()) {
            ret.append(n->name);
        } else {
            ret.append(n->getTierList());
        }
    }

    return ret;
}

QList<QAction*> TierNode::buildMenu(QMenu *menu, QObject *c)
{
    QList<QAction *> ret;

    foreach(TierNode *t, subNodes) {
        if (t->isLeaf()) {
            ret.push_back(menu->addAction(t->name,c,SLOT(changeTier())));
            ret.back()->setCheckable(true);
        } else {
            QMenu *newMenu = menu->addMenu(t->name);
            ret.append(t->buildMenu(newMenu, c));
        }
    }

    return ret;
}

bool TierNode::isLeaf() const
{
    return subNodes.empty();
}
