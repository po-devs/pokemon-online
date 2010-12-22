#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtGui>

class MainEngine;

class CentralWidgetInterface {
public:
    virtual QMenuBar * createMenuBar(MainEngine *) { return NULL;}
    virtual QSize defaultSize() { return QSize();}
};

#endif // CENTRALWIDGET_H
