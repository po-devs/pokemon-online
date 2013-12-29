#include <Utilities/coreclasses.h>
#include "tierstruct.h"
#include "tieractionfactory.h"

TierNode::TierNode(const QString &s) : name(s)
{
    parent = NULL;
    pathToTiers = NULL;
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
    delete pathToTiers;
}

void TierNode::buildFromRaw(QByteArray raw)
{
    clean();
    pathToTiers = new QHash<QString, QVector<QString> >();

    DataStream stream(&raw, QIODevice::ReadOnly);

    int lastLevel = 0;
    uchar currentLevel = 0;
    TierNode *lastNode = this;

    QVector<QString> current;

    while (!stream.atEnd()) {
        stream >> currentLevel;

        lastNode = moveInTree(lastNode, currentLevel-lastLevel);
        lastLevel = currentLevel;

        QString name;
        stream >> name;

        current.resize(currentLevel);
        (*pathToTiers)[name] = current;
        current.push_back(name);

        TierNode *n = new TierNode(name);
        n->parent = lastNode;
        lastNode->subNodes.push_back(n);
    }
}

TierNode * TierNode::moveInTree(TierNode *lastNode, int levelDiff)
{
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
    if (!isLeaf())
        f.setPixelSize(14 - (root->text(0).length() >0) );
    else
        f.setPixelSize(12);
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

QTreeWidgetItem *TierNode::addTier(QTreeWidgetItem *category, const QString &tier)
{
    if (pathToTiers == NULL || !pathToTiers->contains(tier)) {
        /* wrongly called */
        return NULL;
    }

    QVector<QString> path = pathToTiers->value(tier);

    TierNode *moving = this;
    while (path.size() > 0) {
        category = moving->addNode(category, path[0]);
        moving = moving->subNode(path[0]);

        if (category == NULL || moving == NULL)
            return NULL;

        path.pop_front();
    }

    return moving->addNode(category, tier);
}

TierNode * TierNode::subNode(const QString name)
{
    foreach(TierNode *t, subNodes) {
        if (t->name == name)
            return t;
    }

    return NULL;
}

/* Adding a tier on an already built tree is tricky. It'd be trickier if there was the possibility to have the need
    to add categories in the proces */
QTreeWidgetItem *TierNode::addNode(QTreeWidgetItem *category, const QString &tier)
{
    QTreeWidgetItem *ret = NULL;

    int i(0), j(0);
    bool cont = true;

    for ( ; i < category->childCount(); i++) {
        for ( ; j < subNodes.count(); j++) {
            if (subNodes.value(j)->name == tier) {
                cont = false;
                break;
            }
            if (subNodes.value(j)->name == category->child(i)->text(0)) {
                break;
            }
        }
        if (!cont)
            break;
    }

    if (cont) {
        for (; j < subNodes.count(); j++) {
            if (subNodes.value(j)->name == tier) {
                cont = false;
                break;
            }
        }
    }

    if (cont)
        return NULL;

    if (category->childCount() > i && category->child(i)->text(0) == tier) {
        return category->child(i);
    }

    ret = new QTreeWidgetItem(QStringList() << tier);
    QFont f = ret->font(0);
    if (!subNodes.value(j)->isLeaf())
        f.setPixelSize(14 - (category->text(0).length() >0) );
    else
        f.setPixelSize(12);
    ret->setFont(0,f);

    category->insertChild(i, ret);
    ret->setExpanded(true);

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

QList<QAction*> TierNode::buildMenu(QMenu *menu, QObject *c, TierActionFactory *f)
{
    QList<QAction *> ret;

    foreach(TierNode *t, subNodes) {
        if (t->isLeaf()) {
            if (f) {
                QMenu *newMenu = menu->addMenu(t->name);
                QList<QAction *> as = f->createTierActions(newMenu, c, SLOT(changeTier()));
                foreach(QAction *a, as) {
                    a->setProperty("tier", t->name);
                }
                ret.append(as);
            } else {
                ret.push_back(menu->addAction(t->name,c,SLOT(changeTier())));
                ret.back()->setCheckable(true);
                ret.back()->setProperty("tier", t->name);
            }
        } else {
            QMenu *newMenu = menu->addMenu(t->name);
            ret.append(t->buildMenu(newMenu, c, f));
        }
    }

    return ret;
}

bool TierNode::isLeaf() const
{
    return subNodes.empty();
}
