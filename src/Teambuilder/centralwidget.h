#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtGui>
#ifdef QT5
#include <QtWidgets>
#endif

class MainEngine;

class CentralWidgetInterface {
public:
    virtual QMenuBar * createMenuBar(MainEngine *) { return NULL;}
    virtual QSize defaultSize() const { return QSize();}
};

#endif // CENTRALWIDGET_H
