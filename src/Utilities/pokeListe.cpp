#include "pokeListe.h"
#include <QMouseEvent>
#include <QApplication>

pokeListe::pokeListe(QWidget * parent):QListView(parent)
{
    //parametre(s) interne(s)
    this->setObjectName("pokeListe");
}

pokeListe::~pokeListe()
{
}

void pokeListe::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
    }
    QListView::mousePressEvent(event);
}

void pokeListe::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos()-startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            //this->setCursor(QCursor(QPixmap("pokeball.png")));
            startDrag();
        }
    }
    QListView::mouseMoveEvent(event);
}

void pokeListe::startDrag()
{
    /*QMimeData * data = new QMimeData();
    data->setText(this->text());
    data->setImageData(QPixmap(this->icon().pixmap(32,32)));
    QDrag * drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(this->icon().pixmap(32,32));
    drag->setDragCursor(QPixmap("pokeball.png"),Qt::MoveAction);
    drag->exec(Qt::MoveAction);*/
}
