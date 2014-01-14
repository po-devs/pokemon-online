#include "qclosedockwidget.h"

QCloseDockWidget::QCloseDockWidget(const QString &title, QWidget *parent) : QDockWidget(title, parent)
{

}

void QCloseDockWidget::closeEvent(QCloseEvent *event)
{
    emit closed();

    QDockWidget::closeEvent(event);
}
