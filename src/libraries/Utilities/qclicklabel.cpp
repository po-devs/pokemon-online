#include "qclicklabel.h"

QClickLabel::QClickLabel(QWidget *parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void QClickLabel::mousePressEvent(QMouseEvent *ev)
{
    emit clicked();
    QLabel::mousePressEvent(ev);
}
