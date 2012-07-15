#include <QMainWindow>
#include <QDockWidget>
#include "teambuilderwidget.h"

void TeamBuilderWidget::addDock(int index, Qt::DockWidgetArea area, QDockWidget *widget)
{
    window->addDockWidget(area, widget);
    widgets.insert(index, widget);
}

QDockWidget *TeamBuilderWidget::getDock(int id)
{
    return widgets.value(id);
}

void TeamBuilderWidget::hideAll()
{
    foreach(QDockWidget *widget, widgets) {
        widget->hide();
    }
}

void TeamBuilderWidget::showAll()
{
    foreach(QDockWidget *widget, widgets) {
        widget->show();
    }
}
