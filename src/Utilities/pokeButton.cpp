#include "pokeButton.h"
#include <QMouseEvent>
#include <QApplication>
#include <QMimeData>
#include <QIcon>
#include <QImage>
#include <QtDebug>
#include "../Utilities/pokeListe.h"
#include <QMessageBox>

pokeButton::pokeButton(QWidget * parent):QPushButton(parent),index(0)
{
    //parametres internes
    this->setObjectName("pokeButton");
    this->setAcceptDrops(true);
}

pokeButton::pokeButton(QString text,QWidget * parent):QPushButton(text,parent),
index(QString(text.at(text.indexOf("&")+1)).toInt(0,10))
{
    //parametres internes
    this->setObjectName("pokeButton");
    this->setAcceptDrops(true);
}

pokeButton::pokeButton(QIcon icon,QString text,QWidget * parent):QPushButton(icon,text,parent),
index(QString(text.at(text.indexOf("&")+1)).toInt(0,10))
{
    //parametres internes
    this->setObjectName("pokeButton");
    this->setAcceptDrops(true);
}

pokeButton::~pokeButton()
{
}

//fonctions protected
void pokeButton::mouseMoveEvent(QMouseEvent * event)
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
    QPushButton::mouseMoveEvent(event);
}

void pokeButton::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
    }
    QPushButton::mousePressEvent(event);
}

void pokeButton::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        QRect rect_button = this->rect();
        if(rect_button.contains(event->posF().toPoint())&&(startPos-event->pos()).manhattanLength()<QApplication::startDragDistance())
        {
            emit clicked();
        }
    }
    QPushButton::mouseReleaseEvent(event);
}

void pokeButton::dragEnterEvent(QDragEnterEvent * event)
{
    if(event->source()->objectName()=="pokeButton")
    {
        pokeButton * source = qobject_cast<pokeButton *>(event->source());
        if(source && source != this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
    else if(event->source()->objectName()=="pokeListe")
    {
        pokeListe * source = qobject_cast<pokeListe *>(event->source());
        if(source)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
}

void pokeButton::dragMoveEvent(QDragMoveEvent * event)
{
    if(event->source()->objectName()=="pokeButton")
    {
        pokeButton * source = qobject_cast<pokeButton *>(event->source());
        if(source && source != this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
    else if(event->source()->objectName()=="pokeListe")
    {
        pokeListe * source = qobject_cast<pokeListe *>(event->source());
        if(source)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
}

void pokeButton::dropEvent(QDropEvent * event)
{
    if(event->source()->objectName()=="pokeButton")
    {
        pokeButton * source = qobject_cast<pokeButton *>(event->source());
        if(source && source != this)
        {
            if(qvariant_cast<QImage>(event->mimeData()->imageData())!=QVariant())
            {
                QImage image=qvariant_cast<QImage>(event->mimeData()->imageData());
                source->setIcon(this->icon());
                QString ns_text = this->text().remove(tr("(&%1)").arg(this->index));
                ns_text =ns_text+tr("(&%1)").arg(source->index);
                QString n_text = event->mimeData()->text().remove(tr("(&%1)").arg(source->index));
                n_text = n_text+tr("(&%1)").arg(this->index);
                source->setText(ns_text);
                this->setIcon(QPixmap::fromImage(image));
                this->setText(n_text);
                QPair<int,int> echange;
                echange.first = source->index;
                echange.second = this->index;
                qDebug() << "echange:" << echange;
                source->setChecked(false);
                emit changePokemonOrder(echange);
            }
        }
    }
    else if(event->source()->objectName()=="pokeListe")
    {
        pokeListe * source = qobject_cast<pokeListe *>(event->source());
        if(source && this->isChecked() == true )
        {
            const QMimeData * data = event->mimeData();
            //QMessageBox::information(0,"",QString("%1\n"+data->text()).arg(this->index));
            emit changePokemonBase(this->index,data->text().toInt(0,10));
        }
    }
}

//fonction private
void pokeButton::startDrag()
{
    QMimeData * data = new QMimeData();
    data->setText(this->text());
    data->setImageData(QPixmap(this->icon().pixmap(32,32)));
    QDrag * drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(this->icon().pixmap(32,32));
    drag->setDragCursor(QPixmap("pokeball.png"),Qt::MoveAction);
    drag->exec(Qt::MoveAction);
}
