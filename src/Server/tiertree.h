#ifndef TIERTREE_H
#define TIERTREE_H

#include <QtCore>

class Tier;
class TierMachine;
class QDomElement;

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
    void clear();
    void kill(Tier *t);
};

class TierTree
{
public:
    TierCategory root;

    void loadFromXml(const QString &xmldata, TierMachine *boss);
    QString toXml() const;
    QList<Tier *> gatherTiers();
};

#endif // TIERTREE_H
