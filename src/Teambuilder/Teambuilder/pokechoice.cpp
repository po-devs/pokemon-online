#include "Teambuilder/pokechoice.h"
#include <QHeaderView>
#include "Teambuilder/modelenum.h"
#include "../PokemonInfo/pokemoninfo.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>

QSize TB_PokeChoice::sizeHint() const {
    //Overrides QTableView's size hint which is too big
    return QWidget::sizeHint();
}

TB_PokeChoice::TB_PokeChoice(QWidget *parent)
    : QTableView(parent)
{
    init();
}

TB_PokeChoice::TB_PokeChoice(QAbstractItemModel *model, bool missingno)
{
    init();
    setModel(model);

    if (!missingno) {
        //hideRow(0);
    }
}

void TB_PokeChoice::setModel(QAbstractItemModel *model)
{
    if (selectionModel()) {
        selectionModel()->disconnect(SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this);
    }
    QTableView::setModel(model);

    connect(selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(selectedCell(QModelIndex)));
}

void TB_PokeChoice::init()
{
    verticalHeader()->setDefaultSectionSize(22);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(false);
    verticalHeader()->hide();

    horizontalHeader()->setDefaultSectionSize(40);
    horizontalHeader()->setStretchLastSection(true);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    connect(this, SIGNAL(activated(QModelIndex)), SLOT(activatedCell(QModelIndex)));
}

void TB_PokeChoice::activatedCell(const QModelIndex &index)
{
    int num = model()->data(index, CustomModel::PokenumRole).toInt();

    emit pokemonActivated(Pokemon::uniqueId(num, 0));
}

void TB_PokeChoice::selectedCell(const QModelIndex &index)
{
    int num = model()->data(index, CustomModel::PokenumRole).toInt();

    emit pokemonSelected(Pokemon::uniqueId(num, 0));
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

void TB_PokeChoice::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Space)
    {
        emit activated(currentIndex());
    } else {
        QTableView::keyPressEvent(event);
    }
}
