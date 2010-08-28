#include <QtXml>
#include "tiertree.h"
#include "tier.h"

TierCategory::~TierCategory()
{
    clear();
}

void TierCategory::clear()
{
    foreach(TierCategory *c, subCategories) {
        c->clear();
        delete c;
    }
    foreach(Tier *t, subLeafs) {
        delete t;
    }
    subLeafs.clear();
    subCategories.clear();
    name.clear();
}

void TierCategory::loadFromXml(const QDomElement &elem, TierMachine *boss, bool root)
{
    clear();

    if (!root)
        name = elem.attribute("name");

    this->root = root;

    QDomElement n = elem.firstChildElement();
    while(!n.isNull()) {
        if (n.tagName() == "category") {
            TierCategory *c = new TierCategory();
            c->loadFromXml(n, boss);
            subCategories.push_back(c);
        } else if (n.tagName() == "tier") {
            Tier *t = new Tier(boss);
            t->loadFromXml(n);
            subLeafs.push_back(t);
        }
        n = n.nextSiblingElement();
    }
}

void TierTree::loadFromXml(const QString &xmldata, TierMachine *boss)
{
    QDomDocument doc;
    doc.setContent(xmldata);
    QDomElement docElem = doc.documentElement();
    root.loadFromXml(docElem, boss, true);
}
