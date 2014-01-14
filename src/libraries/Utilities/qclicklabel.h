#ifndef QCLICKLABEL_H
#define QCLICKLABEL_H

#include <QLabel>

class QClickLabel : public QLabel
{
    Q_OBJECT
public:
    QClickLabel(QWidget *parent =NULL);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *ev);
};

#endif // QCLICKLABEL_H
