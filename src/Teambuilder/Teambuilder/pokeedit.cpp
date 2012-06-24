#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QLineEdit>
#include <QMessageBox>

#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "Teambuilder/pokeedit.h"
#include "ui_pokeedit.h"
#include "theme.h"
#include "Teambuilder/pokemovesmodel.h"
#include "Teambuilder/pokeselection.h"
#include "Teambuilder/poketablemodel.h"

PokeEdit::PokeEdit(PokeTeam *poke, QAbstractItemModel *pokeModel, QAbstractItemModel *itemsModel, QAbstractItemModel *natureModel) :
    ui(new Ui::PokeEdit),
    pokemonModel(pokeModel),
    m_poke(poke)
{
    ui->setupUi(this);
    ui->itemSprite->raise();
    ui->item->setModel(itemsModel);
    ui->nature->setModel(natureModel);

    ui->levelSettings->setPoke(poke);
    ui->evbox->setPoke(poke);
    ui->ivbox->setPoke(poke);

    movesModel = new PokeMovesModel(poke->num(), poke->gen(), this);
    QSortFilterProxyModel *filter = new QSortFilterProxyModel(this);
    filter->setSourceModel(movesModel);
    ui->moveChoice->setModel(filter);

    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::PP, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::Pow, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::Acc, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::Name, QHeaderView::Fixed);
    ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Name, 125);
    ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Type, Theme::TypePicture(Type::Normal).width()+5);
    ui->moveChoice->setIconSize(Theme::TypePicture(Type::Normal).size());

    ui->moveChoice->sortByColumn(PokeMovesModel::Name, Qt::AscendingOrder);

    m_moves[0] = ui->move1;
    m_moves[1] = ui->move2;
    m_moves[2] = ui->move3;
    m_moves[3] = ui->move4;

    connect(ui->moveChoice, SIGNAL(activated(QModelIndex)), SLOT(moveEntered(QModelIndex)));

    /* the four move choice items */
    for (int i = 0; i < 4; i++)
    {
        QCompleter *completer = new QCompleter(m_moves[i]);
        completer->setModel(ui->moveChoice->model());
        completer->setCompletionColumn(PokeMovesModel::Name);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setCompletionRole(Qt::DisplayRole);
        m_moves[i]->setCompleter(completer);

        completer->setProperty("move", i);
        m_moves[i]->setProperty("move", i);

        connect(completer, SIGNAL(activated(QString)), SLOT(changeMove()));
        connect(m_moves[i], SIGNAL(returnPressed()), SLOT(changeMove()));
        connect(m_moves[i], SIGNAL(editingFinished()), SLOT(changeMove()));
    }

    connect(ui->levelSettings, SIGNAL(levelUpdated()), this, SLOT(updateStats()));
    connect(ui->levelSettings, SIGNAL(levelUpdated()), ui->ivbox, SLOT(updateStats()));
    connect(ui->levelSettings, SIGNAL(shinyUpdated()), this, SLOT(updatePicture()));
    connect(ui->levelSettings, SIGNAL(genderUpdated()), this, SLOT(updatePicture()));
    connect(ui->levelSettings, SIGNAL(genderUpdated()), this, SLOT(updateGender()));
    connect(ui->ivbox, SIGNAL(genderUpdated()), SLOT(updateGender()));
    connect(ui->ivbox, SIGNAL(genderUpdated()), ui->levelSettings, SLOT(updateGender()));
    connect(ui->ivbox, SIGNAL(shinyUpdated()), SLOT(updatePicture()));
    connect(ui->ivbox, SIGNAL(shinyUpdated()), ui->levelSettings, SLOT(updateShiny()));
    connect(ui->happiness, SIGNAL(valueChanged(int)), this, SLOT(changeHappiness(int)));
    connect(ui->nature, SIGNAL(currentIndexChanged(int)), this, SLOT(changeNature(int)));
    connect(ui->item, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeItem(QString)));
    connect(ui->evbox, SIGNAL(natureChanged(int)), this, SLOT(setNature(int)));
    connect(ui->evbox, SIGNAL(natureBoostChanged()), ui->ivbox, SLOT(updateStats()));
    connect(ui->ivbox, SIGNAL(statsUpdated()), ui->evbox, SLOT(updateEVs()));

    updateAll();
}

void PokeEdit::on_nickname_textChanged(const QString &s)
{
    poke().nickname() = s;

    emit nameChanged();
}

void PokeEdit::on_pokemonFrame_clicked()
{
    PokeTableModel *model = (PokeTableModel*) pokemonModel;
    model->setGen(poke().gen());
    PokeSelection *p = new PokeSelection(poke().num(), pokemonModel);
    p->setParent(this, Qt::Popup);
    QPoint pos = ui->pokemonFrame->mapToGlobal(ui->pokemonFrame->pos());
    p->move(pos.x() + ui->pokemonFrame->width()+10, pos.y()-ui->pokemonFrame->height()/2);
    p->setAttribute(Qt::WA_DeleteOnClose, true);
    p->show();

    connect(p, SIGNAL(pokemonChosen(Pokemon::uniqueId)), SLOT(setNum(Pokemon::uniqueId)));
}

void PokeEdit::changeMove()
{
    int slot = sender()->property("move").toInt();
    int move = MoveInfo::Number(m_moves[slot]->text());

    setMove(slot, move);
}

void PokeEdit::setMove(int slot, int move)
{
    try {
        poke().setMove(move, slot, true);
        m_moves[slot]->setText(MoveInfo::Name(move));

        if (move == Move::Return) {
            ui->happiness->setValue(255);
        } else if (move == Move::Frustration) {
            ui->happiness->setValue(0);
        }
        if (poke().num().pokenum == Pokemon::Keldeo) {
            if (!poke().hasMove(Move::SecretSword)) {
                setNum(Pokemon::Keldeo);
            }
        }
    } catch (const QString &s) {
        QMessageBox::information(NULL, tr("Invalid moveset"), s);
        m_moves[slot]->clear();
    }
}

void PokeEdit::moveEntered(const QModelIndex &index)
{
    int num = index.data(CustomModel::MovenumRole).toInt();

    for (int i = 0; i < 4; i++) {
        if (poke().move(i) == Move::NoMove) {
            setMove(i, num);
            return;
        }
    }

    QMessageBox::information(NULL, tr("Impossible to add move"), tr("No more free moves!"));
}

PokeEdit::~PokeEdit()
{
    delete ui;
}

void PokeEdit::updateAll()
{
    ui->pokemonSprite->setPixmap(poke().picture());
    ui->nickname->setText(poke().nickname());
    ui->type1->setPixmap(Theme::TypePicture(poke().type1()));
    ui->type2->setPixmap(Theme::TypePicture(poke().type2()));
    ui->speciesLabel->setText(PokemonInfo::Name(poke().num()));
    ui->genderSprite->setPixmap(Theme::GenderPicture(poke().gender()));
    ui->happiness->setValue(poke().happiness());
    ui->nature->setCurrentIndex(poke().nature());
    setItem(poke().item());
    ui->levelSettings->updateAll();
    ui->evbox->updateAll();
    ui->ivbox->updateAll();

    movesModel->setPokemon(poke().num(), poke().gen());

    for (int i = 0; i < 4; i++) {
        if (poke().move(i) != 0) {
            m_moves[i]->setText(MoveInfo::Name(poke().move(i)));
        } else {
            m_moves[i]->clear();
        }
    }

    ui->type2->setVisible(poke().type2() != Type::Curse);
    ui->genderSprite->setVisible(poke().gender() != Pokemon::Neutral);

    if (poke().gen().num == 1) {
        ui->happiness->hide();
        ui->item->hide();
        ui->itemLabel->hide();
        ui->happinessLabel->hide();
        ui->genderSprite->hide();
    } else {
        ui->happiness->show();
        ui->item->show();
        ui->itemLabel->show();
        ui->happinessLabel->show();
        ui->genderSprite->show();
    }
    if (poke().gen().num <= 2) {
        ui->natureLabel->hide();
        ui->nature->hide();
    } else {
        ui->natureLabel->show();
        ui->nature->show();
    }
}

void PokeEdit::setPoke(PokeTeam *poke)
{
    m_poke = poke;

    ui->evbox->blockSignals(true);
    ui->evbox->setPoke(poke);
    ui->ivbox->setPoke(poke);
    ui->levelSettings->setPoke(poke);
    ui->evbox->blockSignals(false);

    updateAll();
}

void PokeEdit::updateStats()
{
    ui->evbox->updateAll();
}

void PokeEdit::updatePicture()
{
    ui->pokemonSprite->setPixmap(poke().picture());
}

void PokeEdit::updateGender()
{
    ui->genderSprite->setPixmap(Theme::GenderPicture(poke().gender()));
}

void PokeEdit::updateItemSprite(int newItem)
{
    ui->itemSprite->setPixmap(ItemInfo::Icon(newItem));
}

void PokeEdit::setItem(int itemnum)
{
    QString item = ItemInfo::Name(itemnum);

    /* Searches for the index of the item in the combobox */
    int low = 0;
    int high = ui->item->count();

    while (low != high) {
        if (low + 1 == high) {
            if (ui->item->itemText(low) == item)
                ui->item->setCurrentIndex(low);
            else if (ui->item->itemText(high) == item)
               ui-> item->setCurrentIndex(high);
            break;
        }
        int mid = low + (high-low)/2;
        int x = ui->item->itemText(mid).compare(item);
        if (x == 0) {
            ui->item->setCurrentIndex(mid);
            break;
        } else if (x > 0){
            high = mid;
        } else {
            low = mid;
        }
    }

    updateItemSprite(poke().item());
}

void PokeEdit::changeHappiness(int newHappiness)
{
    poke().happiness() = newHappiness;
}

void PokeEdit::setNum(Pokemon::uniqueId num)
{
    if (num == Pokemon::Keldeo_R && !poke().hasMove(Move::SecretSword)) {
        num = Pokemon::Keldeo;
    }

    if (num == poke().num()) {
        return;
    }

    bool sameForme = num.pokenum == poke().num().pokenum;

    if (!sameForme) {
        poke().reset();
    }

    poke().setNum(num);
    poke().load();

    if (!sameForme) {
        poke().nickname() = PokemonInfo::Name(num);
    }

    if (sameForme) {
        poke().runCheck();
    }

    emit numChanged();
    updateAll();
}

void PokeEdit::setNature(int index)
{
    ui->nature->setCurrentIndex(index);
}

void PokeEdit::changeNature(int newNature)
{
    poke().nature() = newNature;
    updateStats();
}

void PokeEdit::changeItem(const QString &itemName)
{
    int itemNum = ItemInfo::Number(itemName);
    poke().item() = itemNum;
    if (poke().num() == Pokemon::Giratina && itemNum == Item::GriseousOrb) {
        setNum(Pokemon::Giratina_O); 
    } else if (poke().num() == Pokemon::Giratina_O && itemNum != Item::GriseousOrb) {
        setNum(Pokemon::Giratina); 
    } else if (itemNum == Item::GriseousOrb && poke().gen() <= 4) {
        poke().item() = 0;
    }
    if (poke().num().pokenum == Pokemon::Arceus) {
        int subnum = ItemInfo::isPlate(itemNum) ? ItemInfo::PlateType(itemNum) : 0;
        setNum(Pokemon::uniqueId(poke().num().pokenum, subnum));
    }
    if (poke().num().pokenum == Pokemon::Genesect) {
        int subnum = ItemInfo::isDrive(itemNum) ? ItemInfo::DriveForme(itemNum) : 0;
        setNum(Pokemon::uniqueId(poke().num().pokenum, subnum));
    }
    updateItemSprite(poke().item());
    emit itemChanged();
}
