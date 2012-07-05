#include "qclicklabel.h"

QClickLabel::QClickLabel(QWidget *parent)
    :QLabel(parent)
{

}

void QClickLabel::mousePressEvent(QMouseEvent *ev)
{
    emit clicked();
    QLabel::mousePressEvent(ev);
}
