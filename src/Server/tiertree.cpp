#include <QtXml>
#include <QtGui>
#include <algorithm>
#include "tiertree.h"
#include "tier.h"
#include <Utilities/coreclasses.h>

TierCategory::~TierCategory()
{
    clear();
}

void TierCategory::cleanCategories()
{
    foreach(TierCategory *c, firstLevelCategories()) {
        c->cleanCategories();
    }

    foreach(TierCategory *c, firstLevelCategories()) {
        if (c->subNodes.empty()) {
            subNodes.removeOne(c);
            delete c;
        }
    }
}

void TierCategory::serialize(DataStream &stream, int level)
{
    if (!root) {
        TierNode::serialize(stream, level);
    }

    level += 1;

    foreach(TierNode *n, subNodes) {
        n->serialize(stream, level);
    }
}

TierCategory *TierCategory::getParentCategory(TierNode *c)
{
    if (subNodes.contains(c)) {
        return this;
    }

    foreach(TierCategory *tc, firstLevelCategories()) {
        TierCategory *tcr =  tc->getParentCategory(c);

        if (tcr)
            return tcr;
    }

    return NULL;
}

static bool nodeComp(TierNode *n1, TierNode *n2) {
    if (n1->displayOrder < n2->displayOrder)
        return true;
    if (n2->displayOrder > n1->displayOrder)
        return false;
    if (n1->isCategory() && !n2->isCategory())
        return true;
    return false;
}

void TierCategory::reorder()
{
    std::sort(subNodes.begin(), subNodes.end(), nodeComp);

    foreach(TierCategory *c, firstLevelCategories()) {
        c->reorder();
    }
}

void TierCategory::kill(TierNode *t) {
    subNodes.removeAll(t);
    delete t;
}

void TierCategory::clear()
{
    foreach(TierCategory *c, firstLevelCategories()) {
        c->clear();
    }
    foreach(TierNode *t, subNodes) {
        delete t;
    }
    subNodes.clear();
    changeName("");
}

TierNode * TierCategory::getNode(const QString &name)
{
    if (root && name == "") {
        return this;
    }

    foreach(TierNode *n, subNodes) {
        if (n->name() == name)
            return n;
    }

    foreach(TierCategory *tc, firstLevelCategories()) {
        TierNode *n =  tc->getNode(name);

        if (n) {
            return n;
        }
    }

    return NULL;
}

void TierCategory::clearWithoutDeleting()
{
    subNodes.clear();
    changeName("");
}

void TierCategory::loadFromXml(const QDomElement &elem, TierMachine *boss, bool root)
{
    clear();

    if (!root)
        changeName(elem.attribute("name"));

    displayOrder = elem.attribute("displayOrder").toInt();

    this->root = root;

    QDomElement n = elem.firstChildElement();
    while(!n.isNull()) {
        TierNode *newN;
        if (n.tagName() == "category") {
            newN = new TierCategory();
            ((TierCategory*)(newN))->loadFromXml(n, boss);
        } else if (n.tagName() == "tier") {
            newN = new Tier(boss, this);
            newN->loadFromXml(n);
        } else {
            break;
        }
        subNodes.push_back(newN);
        n = n.nextSiblingElement();
    }
}

TierCategory *TierCategory::dataClone() const
{
    TierCategory *c = new TierCategory();
    c->changeName(name());
    c->displayOrder = displayOrder;
    c->root = root;

    foreach(TierNode *n, subNodes) {
        c->subNodes.push_back(n->dataClone());
    }

    return c;
}

void TierCategory::appendChild(TierNode *t)
{
    subNodes.append(t);
}

void TierCategory::removeChild(TierNode *t)
{
    subNodes.removeOne(t);
}

void TierCategory::buildRootGui(QTreeWidget *tree)
{
    foreach(TierNode *c, subNodes) {
        tree->addTopLevelItem(c->buildGui());
    }
}

QTreeWidgetItem *TierCategory::buildGui()  const{
    QTreeWidgetItem *it = TierNode::buildGui();

    foreach(TierNode *c, subNodes) {
        it->addChild(c->buildGui());
    }

    return it;
}

QDomElement & TierCategory::toXml(QDomElement &xml) const {
    if (!root) {
        xml.setAttribute("name", name());
    }

    xml.setAttribute("displayOrder", displayOrder);

    foreach (TierNode *c, subNodes) {
        QDomDocument doc;
        QDomElement elem;

        if (c->isCategory()) {
            elem = doc.createElement("category");
        } else {
            elem = doc.createElement("tier");
        }

        c->toXml(elem);
        xml.appendChild(elem);
    }

    return xml;
}

QList<Tier *> TierCategory::gatherTiers()
{
    QList<Tier*> l;
    foreach(TierNode *t, subNodes) {
        if (t->isTier()) {
            l.append((Tier*)(t));
        } else {
            l.append(((TierCategory*)(t))->gatherTiers());
        }
    }
    return l;
}

QList<Tier *> TierCategory::firstLevelTiers()
{
    QList<Tier *> ret;

    foreach(TierNode *n, subNodes) {
        if (n->isTier()) {
            Tier *t = dynamic_cast<Tier*>(n);
            if (t) {
                ret.push_back(t);
            }
        }
    }

    return ret;
}

QList<TierCategory *> TierCategory::gatherCategories()
{
    QList<TierCategory*> l = firstLevelCategories();
    foreach(TierCategory *c, l) {
        l.append(c->gatherCategories());
    }
    return l;
}

QList<TierCategory *> TierCategory::firstLevelCategories()
{
    QList<TierCategory *> ret;

    foreach(TierNode *n, subNodes) {
        if (n->isCategory()) {
            TierCategory *t = dynamic_cast<TierCategory*>(n);
            if (t) {
                ret.push_back(t);
            }
        }
    }

    return ret;
}

void TierTree::loadFromXml(const QString &xmldata, TierMachine *boss)
{
    QDomDocument doc;
    doc.setContent(xmldata);
    QDomElement docElem = doc.documentElement();
    root.loadFromXml(docElem, boss, true);
    reorder();
}

QString TierTree::toXml() const
{
    QDomDocument doc;

    QDomElement main = doc.createElement("category");

    root.toXml(main);

    doc.appendChild(main);

    return doc.toString();
}

QList<Tier*> TierTree::gatherTiers()
{
    return root.gatherTiers();
}

QByteArray TierTree::buildTierList()
{
    QByteArray toWrite;

    DataStream stream(&toWrite, QIODevice::WriteOnly);

    root.serialize(stream, -1);

    return toWrite;
}

void TierTree::cleanCategories() {
    root.cleanCategories();
}

TierTree *TierTree::dataClone() const
{
    TierTree *t = new TierTree();

    TierCategory *rootCopy = root.dataClone();
    t->root = *rootCopy;

    /* As it was copied in the new root, data is shared, and so we have
       to break the sharing */
    rootCopy->clearWithoutDeleting();
    delete rootCopy;

    return t;
}

TierNode *TierTree::getNode(const QString &name) {
    return root.getNode(name);
}

void TierTree::buildTreeGui(QTreeWidget *tree)
{
    tree->clear();

    root.buildRootGui(tree);
}

QList<TierCategory *> TierTree::gatherCategories()
{
    return root.gatherCategories();
}

TierCategory *TierTree::getParentCategory(TierNode *n)
{
    return root.getParentCategory(n);
}

void TierTree::reorder()
{
    root.reorder();
}
