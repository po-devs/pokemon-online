#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtGui>
#include <QtWidgets>

class MainEngine;

class CentralWidgetInterface {
public:
    virtual QMenuBar * createMenuBar(MainEngine *) { return NULL;}
    virtual QSize defaultSize() const { return QSize();}
};

#endif // CENTRALWIDGET_H
