#ifndef QCLOSEDOCKWIDGET_H
#define QCLOSEDOCKWIDGET_H

#include <QDockWidget>

class QCloseDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    QCloseDockWidget(const QString &title="", QWidget *parent = NULL);
protected:
    void closeEvent(QCloseEvent *event);
signals:
    void closed();
};

#endif // QCLOSEDOCKWIDGET_H
