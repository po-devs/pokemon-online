#ifndef TIERTREE_H
#define TIERTREE_H

#include <QtCore>

class Tier;

struct TierCategory
{
    QList<Tier *> subLeafs;
    QList<TierCategory *> subCategories;
    QString name;

    void loadFromXml(const QString &xmldata);
};

class TierTree
{
public:
    TierCategory root;

    void loadFromXml(const QString &xmldata);
};

#endif // TIERTREE_H
