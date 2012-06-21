#include "pokeboxes.h"
#include "ui_pokeboxes.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "Teambuilder/teamholder.h"
#include "theme.h"

PokeBoxes::PokeBoxes(QWidget *parent, TeamHolder *nteam) :
    TeamBuilderWidget(parent), m_team(nteam),
    ui(new Ui::PokeBoxes)
{
    ui->setupUi(this);

    ui->pokemonButtons->setTeam(team().team());
    changePoke(&team().team().poke(0));
    updatePoke();

    connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SLOT(changeTeamPoke(int)));
}

PokeBoxes::~PokeBoxes()
{
    delete ui;
}

void PokeBoxes::changePoke(PokeTeam *poke)
{
    this->m_poke = poke;
}

void PokeBoxes::updatePoke()
{
    ui->nickNameLabel->setText(poke().nickname());
    ui->speciesLabel->setText(PokemonInfo::Name(poke().num()));
    ui->pokemonSprite->setPixmap(poke().picture());
    ui->pokemonSprite->setFixedSize(poke().picture().size());
    ui->type1Label->setPixmap(Theme::TypePicture(PokemonInfo::Type1(poke().num(), poke().gen())));
    if(PokemonInfo::Type2(poke().num()) != Type::Curse) {
        ui->type2Label->setVisible(true);
        ui->type2Label->setPixmap(Theme::TypePicture(PokemonInfo::Type2(poke().num(), poke().gen())));
    } else {
        ui->type2Label->setVisible(false);
    }
    ui->natureLabel->setText(QString("%1").arg(NatureInfo::Name(poke().nature())));
    ui->itemSprite->setPixmap(ItemInfo::Icon(poke().item()));
    ui->genderLabel->setPixmap(Theme::GenderPicture(poke().gender()));
    ui->levelLabel->setText(tr("Lv. %1").arg(poke().level()));
    QString movesInfo;
    for(int movesCount = 0; movesCount < 4; movesCount++) {
        if(movesCount == 4) {
            movesInfo.append(QString("%1").arg(MoveInfo::Name(poke().move(movesCount))));
        } else {
            movesInfo.append(QString("%1\n").arg(MoveInfo::Name(poke().move(movesCount))));
        }
    }
    ui->movesLabel->setText(movesInfo);
}

void PokeBoxes::changeTeamPoke(int index)
{
    changePoke(&team().team().poke(index));
    updatePoke();
}
