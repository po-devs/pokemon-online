#include "pokechoice.h"
#include <QHeaderView>
#include "modelenum.h"
#include "../../PokemonInfo/pokemoninfo.h"
#include <QMouseEvent>
#include <QApplication>

QSize TB_PokeChoice::sizeHint() const {
    //Overrides QTableView's size hint which is too big
    return QWidget::sizeHint();
}

TB_PokeChoice::TB_PokeChoice(QAbstractItemModel *model, bool missingno)
{
    setObjectName("PokeChoice");

    verticalHeader()->setDefaultSectionSize(22);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(false);
    verticalHeader()->hide();
    horizontalHeader()->hide();

    horizontalHeader()->setDefaultSectionSize(40);
    horizontalHeader()->setStretchLastSection(true);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    setModel(model);

    if (!missingno) {
        //hideRow(0);
    }

    connect(this, SIGNAL(activated(QModelIndex)), SLOT(activatedCell(QModelIndex)));
}

void TB_PokeChoice::activatedCell(const QModelIndex &index)
{
    int num = model()->data(index, CustomModel::PokenumRole).toInt();

    emit pokemonActivated(Pokemon::uniqueId(num, 0));
}

void TB_PokeChoice::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
        dragIndex = indexAt(event->pos());
    }
    QTableView::mousePressEvent(event);
}

void TB_PokeChoice::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos()-startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            startDrag();
        }
    }
    QTableView::mouseMoveEvent(event);
}

void TB_PokeChoice::startDrag()
{
    QMimeData * data = new QMimeData();
    if(dragIndex.isValid())
    {
        QVariant num = dragIndex.data(CustomModel::PokenumRole);
        data->setText(num.toString());
        data->setImageData(dragIndex.data(CustomModel::PokeimageRole).value<QPixmap>());

        QDrag * drag = new QDrag(this);
        drag->setMimeData(data);
        drag->setPixmap(dragIndex.data(CustomModel::PokeimageRole).value<QPixmap>());
        drag->exec(Qt::MoveAction);
    }
}
