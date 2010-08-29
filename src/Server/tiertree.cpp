#include <QtXml>
#include "tiertree.h"
#include "tier.h"

TierCategory::~TierCategory()
{
    clear();
}

void TierCategory::kill(Tier *t) {
    subLeafs.removeAll(t);
    delete t;
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
            Tier *t = new Tier(boss, this);
            t->loadFromXml(n);
            subLeafs.push_back(t);
        }
        n = n.nextSiblingElement();
    }
}

QDomElement & TierCategory::toXml(QDomElement &xml) const {
    if (!root) {
        xml.setAttribute("name", name);
    }

    QDomDocument doc;

    foreach (Tier *t, subLeafs) {
        QDomElement elem = doc.createElement("tier");
        t->toXml(elem);
        xml.appendChild(elem);
    }

    foreach (TierCategory *c, subCategories) {
        QDomElement elem = doc.createElement("category");
        c->toXml(elem);
        xml.appendChild(elem);
    }

    return xml;
}

QList<Tier *> TierCategory::gatherTiers()
{
    QList<Tier*> l = subLeafs;
    foreach(TierCategory *c, subCategories) {
        l.append(c->gatherTiers());
    }
    return l;
}

void TierTree::loadFromXml(const QString &xmldata, TierMachine *boss)
{
    QDomDocument doc;
    doc.setContent(xmldata);
    QDomElement docElem = doc.documentElement();
    root.loadFromXml(docElem, boss, true);
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
