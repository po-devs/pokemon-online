#include <QSortFilterProxyModel>
#include <QCompleter>
#include <QLineEdit>
#include <QMessageBox>
#include <QDockWidget>

#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/qclosedockwidget.h"
#include "../Utilities/otherwidgets.h"

#include "TeambuilderLibrary/theme.h"
#include "TeambuilderLibrary/pokeselection.h"

#include "ui_pokeedit.h"
#include "pokeedit.h"
#include "pokemovesmodel.h"
#include "poketablemodel.h"
#include "teambuilderwidget.h"
#include "teambuilder.h"

bool PokeEdit::advancedWindowClosed = false;

PokeEdit::PokeEdit(TeamBuilderWidget *master, PokeTeam *poke, QAbstractItemModel *pokeModel, QAbstractItemModel *itemsModel, QAbstractItemModel *natureModel) :
    ui(new Ui::PokeEdit),
    pokemonModel(pokeModel),
    m_poke(poke), master(master)
{
    ui->setupUi(this);
    ui->itemSprite->raise();
    ui->item->setModel(itemsModel);
    ui->nature->setModel(natureModel);

    ui->levelSettings->setPoke(poke);
    ui->evbox->setPoke(poke);
    ui->ivbox->setPoke(poke);

    if (0) {
        master->getDock(EVDock)->setWidget(ui->evbox);
        master->getDock(IVDock)->setWidget(ui->ivbox);
        master->getDock(LevelDock)->setWidget(ui->levelSettings);
        master->getDock(MoveDock)->setWidget(ui->moveContainer);
    } else {
        QCloseDockWidget *hi = new QCloseDockWidget(tr("Advanced"), this);
        hi->setObjectName("AdvancedTab");
        hi->setWidget(ui->ivbox);
        hi->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
        ui->horizontalMove->addWidget(hi);

        if (advancedWindowClosed) {
            hi->close();
        }

        connect(hi, SIGNAL(closed()), SIGNAL(closeAdvanced()));
//        QDockWidget *hi2 = new QDockWidget(tr("Level"), this);
//        hi2->setWidget(ui->levelSettings);
//        hi2->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
//        ui->horizontalPoke->addWidget(hi2);
    }

    QSortFilterProxyModel *pokeFilter = new QSortFilterProxyModel(this);
    pokeFilter->setFilterRegExp(".");
    pokeFilter->setSourceModel(pokemonModel);

    QCompleter *completer = new QCompleter(pokeFilter, ui->nickname);
    completer->setCompletionColumn(1);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->nickname->setCompleter(completer);
    connect(completer, SIGNAL(activated(QString)), SLOT(setNum(QString)));

    /* 12 characters for the name */
    ui->nickname->setValidator(new QNickValidator(ui->nickname, 12));

    movesModel = new PokeMovesModel(poke->num(), poke->gen(), this);
    QSortFilterProxyModel *filter = new QSortFilterProxyModel(this);
    filter->setSourceModel(movesModel);
    ui->moveChoice->setModel(filter);

#ifdef QT5
    ui->moveChoice->horizontalHeader()->setSectionResizeMode(PokeMovesModel::PP, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setSectionResizeMode(PokeMovesModel::Pow, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setSectionResizeMode(PokeMovesModel::Acc, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setSectionResizeMode(PokeMovesModel::Name, QHeaderView::Fixed);
#else
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::PP, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::Pow, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::Acc, QHeaderView::ResizeToContents);
    ui->moveChoice->horizontalHeader()->setResizeMode(PokeMovesModel::Name, QHeaderView::Fixed);
#endif
    ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Name, 125);
    ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Type, Theme::TypePicture(Type::Normal).width()+5);
    ui->moveChoice->setIconSize(Theme::TypePicture(Type::Normal).size());

    ui->moveChoice->sortByColumn(PokeMovesModel::Name, Qt::AscendingOrder);

    m_moves[0] = ui->move1;
    m_moves[1] = ui->move2;
    m_moves[2] = ui->move3;
    m_moves[3] = ui->move4;

    connect(ui->speciesLabel, SIGNAL(clicked()), SLOT(on_pokemonFrame_clicked()));

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
    connect(ui->levelSettings, SIGNAL(genderUpdated()), this, SLOT(updatePicture()));
    connect(ui->levelSettings, SIGNAL(genderUpdated()), this, SLOT(updateGender()));
    connect(ui->ivbox, SIGNAL(genderUpdated()), SLOT(updateGender()));
    connect(ui->ivbox, SIGNAL(genderUpdated()), ui->levelSettings, SLOT(updateGender()));
    connect(ui->ivbox, SIGNAL(shinyUpdated()), SLOT(updatePicture()));
    connect(ui->happiness, SIGNAL(valueChanged(int)), this, SLOT(changeHappiness(int)));
    connect(ui->nature, SIGNAL(currentIndexChanged(int)), this, SLOT(changeNature(int)));
    connect(ui->item, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeItem(QString)));
    connect(ui->evbox, SIGNAL(natureChanged(int)), this, SLOT(setNature(int)));
    connect(ui->evbox, SIGNAL(natureBoostChanged()), ui->ivbox, SLOT(updateStats()));
    connect(ui->ivbox, SIGNAL(statsUpdated()), ui->evbox, SLOT(updateEVs()));

    updateAll();
}

void PokeEdit::closeAdvancedTab()
{
    findChild<QWidget*>("AdvancedTab")->close();
}

void PokeEdit::showAdvancedTab()
{
    findChild<QWidget*>("AdvancedTab")->show();
}

void PokeEdit::openPokemonSelection()
{
    on_pokemonFrame_clicked();
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
    connect(p, SIGNAL(shinySelected(bool)), SLOT(changeShiny(bool)));
}

void PokeEdit::changeShiny(bool shiny)
{
    if (shiny == poke().shiny()) {
        return;
    }
    if (poke().gen().num == 2) {
        if (shiny) {
            poke().setDV(Speed, 10);
            poke().setDV(Attack, 15);
            poke().setDV(Defense, 10);
            poke().setDV(SpAttack, 10);
        } else {
            poke().setDV(Speed, 15);
            poke().setDV(Attack, 15);
            poke().setDV(SpAttack, 15);
        }
        updateStats();
        ui->ivbox->updateAll();
    } else {
        poke().shiny() = shiny;
    }
    updatePicture();
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

    if (num == Move::SecretSword && poke().num() == Pokemon::Keldeo && PokemonInfo::Released(Pokemon::Keldeo_R, poke().gen())) {
        setNum(Pokemon::Keldeo_R);
        return;
    }

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

void PokeEdit::updatePluginLayout()
{
    QLayoutItem* item;
    while ( ( item = ui->pluginButtons->takeAt( 0 ) ) != NULL )
    {
        delete item->widget();
        delete item;
    }

    master->teambuilder->call("addPokeEditButton(QLayout*,PokeTeam*)", ui->pluginButtons, m_poke);
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

    bool g1 = poke().gen().num == 1;
    ui->happiness->setVisible(!g1);
    ui->item->setVisible(!g1);
    ui->itemLabel->setVisible(!g1);
    ui->happinessLabel->setVisible(!g1);
    ui->genderSprite->setVisible(!g1);

    bool g2 = poke().gen().num <= 2;
    ui->natureLabel->setVisible(!g2);
    ui->nature->setVisible(!g2);

    updatePluginLayout();
}

void PokeEdit::setPoke(PokeTeam *poke)
{
    m_poke = poke;

    ui->evbox->setPoke(poke);
    ui->ivbox->setPoke(poke);
    ui->levelSettings->setPoke(poke);

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

void PokeEdit::setNum(const QString &num)
{
    setNum(PokemonInfo::Number(num));
}

void PokeEdit::setNum(Pokemon::uniqueId num)
{
    if (num == poke().num()) {
        return;
    }

    bool sameForme = num.pokenum == poke().num().pokenum;

    if (!sameForme) {
        poke().reset();
    }

    if (num.pokenum == Pokemon::Keldeo) {
        if (num == Pokemon::Keldeo_R && !poke().hasMove(Move::SecretSword)) {
            try {
                poke().addMove(Move::SecretSword);
            } catch(const QString &) {
                poke().setMove(Move::SecretSword, 0, false);
            }
        } else if (PokemonInfo::Released(Pokemon::Keldeo_R, poke().gen())) {
            poke().removeMove(Move::SecretSword);
        }
    } else if (num.pokenum == Pokemon::Giratina) {
        if (num == Pokemon::Giratina_O && poke().item() != Item::GriseousOrb) {
            poke().item() = Item::GriseousOrb;
        } else if (num == Pokemon::Giratina && poke().item() == Item::GriseousOrb) {
            poke().item() = Item::NoItem;
        }
    } else if (num.pokenum == Pokemon::Arceus && ItemInfo::PlateType(poke().item()) != num.subnum) {
        poke().item() = ItemInfo::PlateForType(num.subnum);
    } else if (num.pokenum == Pokemon::Genesect && ItemInfo::DriveForme(poke().item()) != num.subnum) {
        poke().item() = ItemInfo::DriveForForme(num.subnum);
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
    if (poke().num() == Pokemon::Giratina && itemNum == Item::GriseousOrb && PokemonInfo::Released(Pokemon::Giratina_O, poke().gen())) {
        setNum(Pokemon::Giratina_O); 
    } else if (poke().num() == Pokemon::Giratina_O && itemNum != Item::GriseousOrb) {
        setNum(Pokemon::Giratina); 
    } else if (itemNum == Item::GriseousOrb && poke().gen() <= 4 && poke().num().pokenum != Pokemon::Giratina) {
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
