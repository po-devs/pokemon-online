#include "pokebody.h"
#include "pokebodywidget.h"
#include "advanced.h"
#include "pokemovesmodel.h"
#include "../../PokemonInfo/pokemoninfo.h"


TB_PokemonBody::TB_PokemonBody(PokeTeam *_poke, int num) : widget(NULL)
{
    m_poke = _poke;
    m_num = num;

    setProperty("num", num);

    movesModel = new PokeMovesModel(poke()->num(), gen(), this);
}

void TB_PokemonBody::setWidget(PokeBodyWidget *widget)
{
    this->widget = widget;
    widget->setMovesModel(movesModel);
    widget->loadPokemon(*poke());
    widget->setWidgetNum(num());

    connect(widget, SIGNAL(pokemonChosen(Pokemon::uniqueId)), SLOT(setNum(Pokemon::uniqueId)));
    connect(widget, SIGNAL(nickChosen(QString)), SLOT(setNick(QString)));
    connect(widget, SIGNAL(natureChanged(int)), SLOT(setNature(int)));
    connect(widget, SIGNAL(natureChanged(int)), SIGNAL(natureChanged()));
    connect(widget, SIGNAL(moveChosen(int)), SLOT(setMove(int)));
    connect(widget, SIGNAL(moveChosen(int,int)), SLOT(setMove(int,int)));
    connect(widget, SIGNAL(itemChanged(int)), SLOT(setItem(int)));
    connect(widget, SIGNAL(EVChanged(int)), SIGNAL(EVChanged(int)));
}

bool TB_PokemonBody::hasWidget() const
{
    return widget != NULL;
}

QWidget *TB_PokemonBody::getWidget()
{
    return widget;
}

int TB_PokemonBody::gen()
{
    return poke()->gen().num;
}

void TB_PokemonBody::connectWithAdvanced(TB_Advanced *ptr)
{
    connect(ptr, SIGNAL(abilityChanged()), this, SLOT(updateAbility()));
    connect(ptr, SIGNAL(levelChanged()), this, SLOT(updateLevel()));
    connect(ptr, SIGNAL(imageChanged()), this, SLOT(updateImage()));
    connect(ptr, SIGNAL(genderChanged()), this, SLOT(updateGender()));
    connect(ptr, SIGNAL(genderChanged()), this, SLOT(updateImage()));
    connect(ptr, SIGNAL(statChanged()), this, SLOT(updateEVs()));
    connect(ptr, SIGNAL(pokeFormeChanged(Pokemon::uniqueId)), this, SLOT(changeForme(Pokemon::uniqueId)), Qt::QueuedConnection);
    connect(this, SIGNAL(EVChanged(int)), ptr, SLOT(updateStat(int)));
    connect(this, SIGNAL(natureChanged()), ptr, SLOT(updateStats()));
    connect(this, SIGNAL(pokeImageChanged()), ptr, SLOT(updatePokeImage()));
    connect(ptr, SIGNAL(levelChanged()), this, SIGNAL(levelChanged()));
}

void TB_PokemonBody::updateLevel()
{
    widget->setLevel(poke()->level());
}

void TB_PokemonBody::updateEVs()
{
    widget->updateEVs();
}

void TB_PokemonBody::updateImage()
{
    widget->setPicture(poke()->picture());
}

void TB_PokemonBody::updateGender()
{
    widget->setGender(poke()->gender());
}

void TB_PokemonBody::setNum(Pokemon::uniqueId pokenum)
{
    setNum(pokenum, true);
}

void TB_PokemonBody::setNum(Pokemon::uniqueId pokenum, bool resetEverything)
{
    if (pokenum == poke()->num())
        return;

    if (resetEverything) {
        poke()->reset();
    }

    poke()->setNum(pokenum);
    poke()->load();
    poke()->runCheck();

    updateNum();
}

void TB_PokemonBody::updateNum()
{
    movesModel->setPokemon(poke()->num(), gen());

    if (widget) {
        widget->loadPokemon(*poke());
    }

    emit pokeChanged(poke()->num());
}

void TB_PokemonBody::updateItem()
{
    if (widget) {
        widget->setItem(poke()->item());
    }
}

void TB_PokemonBody::changeForme(Pokemon::uniqueId pokenum)
{
    setNum(pokenum, false);
}

void TB_PokemonBody::updateAbility()
{
    /* Abilities may ruin move combinations, for example dream world abilities (implemented)
       or 4th gen abilities with 3rd gen moves on first stage evos (non implemented) */
    poke()->runCheck();
    updateMoves();
}

void TB_PokemonBody::updateMoves()
{
    if (widget) {
        for (int i = 0; i < 4; i++)
        {
            widget->setMove(i, poke()->move(i));
        }
    }
}

PokeTeam * TB_PokemonBody::poke()
{
    return m_poke;
}

void TB_PokemonBody::setNick(const QString &nick)
{
    poke()->nickname() = nick;
    emit nicknameChanged(nick);
}

void TB_PokemonBody::changeGeneration(int gen)
{
    poke()->setGen(gen);
    poke()->load();
    poke()->runCheck();

    if (widget) {
        widget->changeGen(gen);
    }

    updateNum();
}

void TB_PokemonBody::setMove(int moveslot, int movenum)
{
    try {
        poke()->setMove(movenum, moveslot,true);
    }
    catch (QString &expr)
    {
        if (widget) {
            QMessageBox::critical(widget, tr("Error"), expr);
            /* Restoring previous move */
            widget->setMove(moveslot, 0);
        }

        return;
    }
}

void TB_PokemonBody::setMove(int movenum)
{
    try {
        int slot = poke()->addMove(movenum,true);

        if (widget) {
            widget->setMove(slot, movenum);
        }

        if (movenum == Move::Return) {
            poke()->happiness() = 255;
        } else if (movenum == Move::Frustration) {
            poke()->happiness() = 0;
        }
    } catch (QString &expr)
    {
        if (widget) {
            QMessageBox::critical(widget, tr("Error"), expr);
        }
    }
}

void TB_PokemonBody::setItem(int it)
{
    if (it == poke()->item()) {
        return;
    }

    if (PokemonInfo::OriginalForme(poke()->num()) == Pokemon::Arceus) {
        int type = 0;
        if (ItemInfo::isPlate(it)) {
            type = ItemInfo::PlateType(it);
        }

        if (type != poke()->num().subnum) {
            changeForme(Pokemon::uniqueId(poke()->num().pokenum, type));
        }
    }
    if (PokemonInfo::OriginalForme(poke()->num()) == Pokemon::Genesect) {
        int forme = 0;
        if (ItemInfo::isDrive(it)) {
            forme = ItemInfo::DriveForme(it);
        }

        if (forme != poke()->num().subnum) {
            changeForme(Pokemon::uniqueId(poke()->num().pokenum, forme));
        }
    }
    if (it == Item::GriseousOrb && poke()->num().pokenum != Pokemon::Giratina && gen() <= 4) {
        poke()->item() = 0;
        if (widget) {
            widget->setItem(0);
        }
    }
    else {
        poke()->item() = it;
        if (poke()->item() == Item::GriseousOrb) {
            if (poke()->num() == Pokemon::Giratina)
                changeForme(Pokemon::Giratina_O);
        } else if (poke()->num() == Pokemon::Giratina_O) {
            changeForme(Pokemon::Giratina);
        }
    }

    emit itemChanged(it);
}

void TB_PokemonBody::setNature(int nature)
{
    poke()->nature() = nature;
}

