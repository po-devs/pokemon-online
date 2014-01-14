#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/pokemoninfo.h>

#include <QDragEnterEvent>
#include <QDrag>

#include "Teambuilder/pokebutton.h"
#include "ui_pokebutton.h"

PokeButton::PokeButton(QWidget *parent) :
    QPushButton(parent),
    ui(new Ui::PokeButton),
    num(0)
{
    ui->setupUi(this);
    ui->number->setBuddy(this);
    layout()->setMargin(2);
    lastPress.invalidate();
}

PokeButton::~PokeButton()
{
    delete ui;
}

void PokeButton::setNumber(int x)
{
    num = x;
    ui->number->setText(tr("#&%1").arg(x+1));
    setAccessibleName(tr("Pokemon slot %1").arg(x+1));
}

void PokeButton::setPokemon(PokeTeam &poke)
{
    m_poke = &poke;
    updateAll();
}

void PokeButton::updateAll()
{
    PokeTeam &poke = *m_poke;

    ui->level->setText(tr("Lv. %1").arg(poke.level()));
    ui->item->setPixmap(ItemInfo::Icon(poke.item()));
    ui->species->setText(PokemonInfo::Name(poke.num()));
    ui->nickname->setText(poke.nickname().isEmpty() ? ui->species->text() : poke.nickname());
    ui->sprite->setPixmap(poke.picture());
}

void PokeButton::mouseMoveEvent(QMouseEvent * event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos()-startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            startDrag();
        }
    }
    QPushButton::mouseMoveEvent(event);
}

void PokeButton::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
    }
    QPushButton::mousePressEvent(event);

    if (lastPress.isValid() && lastPress.elapsed() < QApplication::doubleClickInterval()) {
        lastPress.invalidate();
        emit doubleClicked();
    } else {
        lastPress.restart();
    }
}


void PokeButton::dragEnterEvent(QDragEnterEvent * event)
{
    event->setDropAction(Qt::MoveAction);
    event->accept();
}

void PokeButton::dropEvent(QDropEvent * event)
{
    if(event->source()->metaObject()->className()== metaObject()->className()) {
        PokeButton *other = dynamic_cast<PokeButton*>(event->source());
        if (!other) {
            return;
        }
        std::swap(poke(), other->poke());

        setPokemon(poke());
        other->setPokemon(other->poke());

        event->accept();

        emit pokemonOrderChanged(other->num, num);
    } else {
        emit dropEventReceived(num, event);
    }
}

void PokeButton::keyReleaseEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return || k->key() == Qt::Key_Space) {
        emit doubleClicked();
    } else {
        QPushButton::keyReleaseEvent(k);
    }
}

void PokeButton::startDrag()
{
    QMimeData * data = new QMimeData();
    data->setText(ui->species->text());
    data->setImageData(poke().picture());
    data->setData("TeamSlot", QByteArray::number(num));
    QDrag * drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(*ui->sprite->pixmap());
    drag->exec(Qt::MoveAction);
    setChecked(true);
}
