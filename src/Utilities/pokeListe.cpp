#include "pokeListe.h"
#include <QMouseEvent>
#include <QApplication>
#include <QTableWidgetItem>
#include "../PokemonInfo/pokemonInfo.h"


pokeListe::pokeListe(int row, int col, QWidget *parent):QTableWidget(row,col,parent),itemForDrag(0)
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
        itemForDrag = this->itemAt(event->pos());
        //itemForDrag = this->currentItem();
    }
    QTableWidget::mousePressEvent(event);
}

void pokeListe::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos()-startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            //this->setCursor(QCursor(QPixmap(":/drag/pokeball")));
            startDrag();
        }
    }
    QTableWidget::mouseMoveEvent(event);
}

void pokeListe::startDrag()
{
    QMimeData * data = new QMimeData();
    if(itemForDrag !=0)
    {
        int r=itemForDrag->row();
        if(itemForDrag->column()!=0)
        {
            itemForDrag = this->item(r,0);
        }
        //QMessageBox::information(0,"start drag",itemForDrag->data(Qt::DisplayRole).toString());
        data->setText(itemForDrag->data(Qt::DisplayRole).toString());
        data->setImageData(PokemonInfo::Picture(itemForDrag->data(Qt::DisplayRole).toInt(0)));
        QDrag * drag = new QDrag(this);
        drag->setMimeData(data);
        drag->setPixmap(PokemonInfo::Picture(itemForDrag->data(Qt::DisplayRole).toInt(0)));
        /*drag->setDragCursor(QPixmap(":/drag/pokeball"),Qt::MoveAction);*/
            /*drag->setPixmap(QPixmap(":/drag/pokeball"));
        drag->setDragCursor(PokemonInfo::Picture(itemForDrag->data(Qt::DisplayRole).toInt(0)),Qt::MoveAction);
        */drag->exec(Qt::MoveAction);
    }
}
