#include <QMenu>
#include <QCompleter>
#include <QSortFilterProxyModel>

#include <PokemonInfo/pokemoninfo.h>

#include "theme.h"
#include "pokeselection.h"
#include "ui_pokeselection.h"
#include "modelenum.h"
#include "advancedsearch.h"

PokeSelection::PokeSelection(Pokemon::uniqueId pokemon, QAbstractItemModel *pokemonModel) :
    ui(new Ui::PokeSelection), search(NULL), newwidth(0)
{
    ui->setupUi(this);

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setFilterRegExp(".");
    proxy->setSourceModel(pokemonModel);

    this->sourceModel = pokemonModel;
    this->proxy = proxy;

    ui->pokemonList->setModel(proxy);

    QCompleter *completer = new QCompleter(proxy, ui->pokeEdit);
    completer->setCompletionColumn(1);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->pokeEdit->setCompleter(completer);

    setNum(pokemon);

    if (getGen() <= 1) {
        ui->shiny->hide();
    } else {
        ui->shiny->show();
    }

    ui->baseStats->setGen(getGen());

    connect(completer, SIGNAL(activated(QModelIndex)), SLOT(setPokemon(QModelIndex)));
    connect(ui->shiny, SIGNAL(toggled(bool)), SLOT(updateSprite()));
    connect(ui->pokemonList, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(setNum(Pokemon::uniqueId)));
    connect(ui->pokemonList, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(updateSprite()));
    connect(ui->pokemonList, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(updateTypes()));
    connect(ui->pokemonList, SIGNAL(pokemonActivated(Pokemon::uniqueId)), SLOT(finish()));
    connect(ui->changeSpecies, SIGNAL(clicked()), SLOT(finish()));
    connect(ui->pokemonFrame, SIGNAL(clicked()), SLOT(toggleSearchWindow()));
}

void PokeSelection::toggleSearchWindow()
{
    if (search) {
        QGridLayout *gl = (QGridLayout*)layout();
        gl->removeWidget(search);
        search->deleteLater();
        search = NULL;

        setFixedWidth(oldwidth);
        move(oldx, y());
    } else {
        //Tricks to get a window at the correct size. Qt is annoying, not allowing resize() to work
        //properly on windows, i have to use setFixedWidth on the top level window :(
        oldwidth = width();
        oldx = x();

        QGridLayout *gl = (QGridLayout*)layout();
        search = new AdvancedSearch(this);
        search->setGen(getGen());

        ui->pokemonList->setFixedWidth(ui->pokemonList->width());
        ui->pokeEdit->setFixedWidth(ui->pokeEdit->width());
        ui->changeSpecies->setFixedWidth(ui->changeSpecies->width());
        search->setResultsWidth(ui->pokemonList->width());

        if (newwidth) {
            setFixedWidth(newwidth);
        }

        gl->addWidget(search, 0, 4, gl->rowCount(), 1);
        search->show();

        if (newwidth == 0) {
            newwidth = width();
        }

        connect(search, SIGNAL(pokemonSelected(Pokemon::uniqueId)), SLOT(setNum(Pokemon::uniqueId)));

        /* Moving the widget if it goes out of bounds */
        QWidget *top = ((QWidget*)parent())->window();

        if (x() + width() > top->x() + top->width()) {
            move(std::max(top->x(), top->x()+(top->width()-width())/2), y());
        }
    }
    /* Because qt5 doesn't know how to do it itself :( */
    update();
}

void PokeSelection::show()
{
    QWidget::show();
    ui->pokemonList->setFocus();
}

void PokeSelection::setPokemon(const QModelIndex &p)
{
    setNum(p.data(CustomModel::PokenumRole).toInt());
}

void PokeSelection::setNum(const Pokemon::uniqueId &num)
{
    if (m_num == num) {
        return;
    }

    ui->pokeEdit->setText(PokemonInfo::Name(num));
    ui->pokemonList->setCurrentIndex(proxy->mapFromSource(sourceModel->index(num.pokenum, 1)));
    ui->pokemonList->scrollTo(ui->pokemonList->currentIndex());
    m_num = num;

    ui->baseStats->setNum(num);

    if (PokemonInfo::HasFormes(num) && PokemonInfo::AFormesShown(num)) {
        QMenu *m = new QMenu(ui->altForme);
        QList<Pokemon::uniqueId> formes = PokemonInfo::Formes(num, getGen());

        QSignalMapper *mapper = new QSignalMapper(m);
        foreach(Pokemon::uniqueId forme, formes) {
            QAction *ac = m->addAction(PokemonInfo::Name(forme), mapper, SLOT(map()));
            ac->setCheckable(true);
            if (forme == num) {
                ac->setChecked(true);
            }
            mapper->setMapping(ac, forme.toPokeRef());
        }
        connect(mapper, SIGNAL(mapped(int)), SLOT(changeForme(int)));

        ui->altForme->setMenu(m);
        ui->altForme->setEnabled(true);
    } else {
        delete ui->altForme->menu();
        ui->altForme->setMenu(0);
        ui->altForme->setDisabled(true);
    }

    updateTypes();
    updateSprite();
}

void PokeSelection::changeForme(int pokeref)
{
    setNum(Pokemon::uniqueId(pokeref));
}

void PokeSelection::finish()
{
    emit pokemonChosen(num());
    emit shinySelected(ui->shiny->isChecked());

    close();
}

Pokemon::gen PokeSelection::getGen()
{
    return ui->pokemonList->currentIndex().data(CustomModel::PokegenRole).value<Pokemon::gen>();
}

void PokeSelection::updateSprite()
{
    Pokemon::uniqueId _num = num();
    bool shiny = ui->shiny->isChecked();

    ui->pokemonSprite->setPixmap(PokemonInfo::Picture(_num, getGen(), Pokemon::Male, shiny));
}

void PokeSelection::updateTypes()
{
    Pokemon::uniqueId _num = num();
    int type1 = PokemonInfo::Type1(_num, getGen());
    int type2 = PokemonInfo::Type2(_num, getGen());

    ui->type1->setPixmap(Theme::TypePicture(type1));

    if (type2 != Pokemon::Curse) {
        ui->type2->setPixmap(Theme::TypePicture(type2));
        ui->type2->setVisible(true);
    } else {
        ui->type2->setVisible(false);
    }
}

PokeSelection::~PokeSelection()
{
    delete ui;
}
