#include "qglwarner.h"
#include <QTimer>

QGLWarner::QGLWarner(QWidget *v) : QGLWidget(v)
{

}

void QGLWarner::deleteLater()
{
    emit disappearing();
    QTimer::singleShot(10000, this, SLOT(realDeleteLater()));
}

void QGLWarner::realDeleteLater()
{
    QWidget::deleteLater();
}

QGLWarner::~QGLWarner()
{
    emit disappearing();
}

void QGLWarner::setVisible(bool visible)
{
    if (visible) {
        emit appearing();
    } else {
        emit disappearing();
    }

    QGLWidget::setVisible(visible);
}

void QGLWarner::hide()
{
    emit disappearing();

    QGLWidget::hide();
}

void QGLWarner::hideEvent(QHideEvent *h)
{
    emit disappearing();

    QGLWidget::hideEvent(h);
}

void QGLWarner::show()
{
    emit appearing();

    QGLWidget::show();
}

void QGLWarner::showEvent(QShowEvent *h)
{
    emit appearing();

    QGLWidget::showEvent(h);
}
