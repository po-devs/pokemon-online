#ifndef TIERACTIONFACTORY_H
#define TIERACTIONFACTORY_H

#include <QList>

class QAction;
class QMenu;
class QObject;

class TierActionFactory
{
public:
    virtual QList<QAction*> createTierActions(QMenu *m, QObject *o, const char *slot) = 0;
};

#endif // TIERACTIONFACTORY_H
