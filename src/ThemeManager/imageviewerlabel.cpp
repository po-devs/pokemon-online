#include "imageviewerlabel.h"

#include <QKeyEvent>
#include <QDebug>

ImageViewerLabel::ImageViewerLabel(QWidget *parent) :
    QLabel(parent)
{
}

void ImageViewerLabel::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "keypress" << event->key() << Qt::Key_Left;

    if (event->type() == QEvent::KeyPress) {
        if(event->key() == Qt::Key_Left) {
            emit leftPressed();
            return;
        } else if (event->key() == Qt::Key_Right) {
            emit rightPressed();
            return;
        }
    }
    QLabel::keyPressEvent(event);
}
