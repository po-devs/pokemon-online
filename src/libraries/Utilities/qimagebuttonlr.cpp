#include "qimagebuttonlr.h"
#include <QMouseEvent>

QImageButtonLR::QImageButtonLR(QWidget* w)
    :QImageButton(w)
{}

QImageButtonLR::QImageButtonLR(const QString &normal, const QString &hovered)
    :QImageButton(normal,hovered)
{}

void QImageButtonLR::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
        emit leftClick();
    else if (ev->button() == Qt::RightButton)
        emit rightClick();
}

