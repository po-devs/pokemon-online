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

    ui->type2->setVisible(poke().type2() != Type::Curse);
    ui->genderSprite->setVisible(poke().gender() != Pokemon::Neutral);
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

    ui->itemSprite->setPixmap(ItemInfo::Icon(poke().item()));
}
