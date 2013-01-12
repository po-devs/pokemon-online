#include <QEvent>
#include <QScrollBar>

#include "qverticalscrollarea.h"

QVerticalScrollArea::QVerticalScrollArea(QWidget *parent) :
    QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void QVerticalScrollArea::setWidget(QWidget *w)
{
    QScrollArea::setWidget(w);
    w->installEventFilter(this);
}

bool QVerticalScrollArea::eventFilter(QObject *o, QEvent *e)
{
    if(o == widget() && e->type() == QEvent::Resize)
        setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());

    return false;
}
