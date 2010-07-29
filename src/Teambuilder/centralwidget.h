#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtGui>

class MainEngine;

class CentralWidgetInterface {
public:
    virtual QMenuBar * createMenuBar(MainEngine *) { return NULL;}
};

#endif // CENTRALWIDGET_H
