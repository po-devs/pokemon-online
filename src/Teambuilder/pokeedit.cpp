#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "pokeedit.h"
#include "ui_pokeedit.h"
#include "theme.h"

PokeEdit::PokeEdit(PokeTeam *poke, QAbstractItemModel *itemsModel, QAbstractItemModel *natureModel) :
    ui(new Ui::PokeEdit),
    m_poke(poke)
{
    ui->setupUi(this);
    ui->itemSprite->raise();
    ui->item->setModel(itemsModel);
    ui->nature->setModel(natureModel);

    ui->levelSettings->setPoke(poke);
    ui->evbox->setPoke(poke);

    connect(ui->levelSettings, SIGNAL(levelUpdated()), this, SLOT(updateStats()));
    connect(ui->levelSettings, SIGNAL(shinyUpdated()), this, SLOT(updatePicture()));
    connect(ui->levelSettings, SIGNAL(genderUpdated()), this, SLOT(updatePicture()));
    connect(ui->levelSettings, SIGNAL(genderUpdated()), this, SLOT(updateGender()));
    connect(ui->happiness, SIGNAL(valueChanged(int)), this, SLOT(changeHappiness(int)));
    connect(ui->nature, SIGNAL(currentIndexChanged(int)), this, SLOT(changeNature(int)));
    connect(ui->item, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeItem(QString)));
    connect(ui->evbox, SIGNAL(natureChanged(int)), this, SLOT(setNature(int)));

    updateAll();
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

    ui->type2->setVisible(poke().type2() != Type::Curse);
    ui->genderSprite->setVisible(poke().gender() != Pokemon::Neutral);
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
    updateItemSprite(poke().item());
}
