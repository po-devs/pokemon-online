#include "advancedsearch.h"
#include "ui_advancedsearch.h"

#include "../PokemonInfo/pokemoninfo.h"
#include <QStringListModel>
#include <QCompleter>

AdvancedSearch::AdvancedSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdvancedSearch)
{
    ui->setupUi(this);
}

AdvancedSearch::~AdvancedSearch()
{
    delete ui;
}

void AdvancedSearch::setGen(const Pokemon::gen &gen)
{
    if (gen.num == 1) {
        ui->spdefbox->hide();
        ui->spdefedit->hide();
        ui->spdeflabel->hide();
        ui->spatklabel->setText(tr("Special"));
    }
    if (gen <= 2) {
        ui->ability->setDisabled(true);
    }

    //loads the data
    QStringList types;

    for (int i = 0; i < TypeInfo::NumberOfTypes(); i++) {
        types.push_back(TypeInfo::Name(i));
    }

    QStringListModel *tmodel = new QStringListModel(types, this);
    ui->type1->setModel(tmodel);
    ui->type2->setModel(tmodel);

    QStringList abilities;

    //Todo : NumberOfAbilities(gen)?
    for (int i = 0; i < AbilityInfo::NumberOfAbilities(); i++) {
        abilities.push_back(AbilityInfo::Name(i));
    }

    ui->ability->setModel(new QStringListModel(abilities, this));

    QStringList moves;

    //Todo : NumberOfMoves(gen)?
    for (int i = 0; i < MoveInfo::NumberOfMoves(); i++) {
        moves.push_back(MoveInfo::Name(i));
    }

    QStringListModel *mmodel = new QStringListModel(moves, this);
    QLineEdit *movesw[4] = {ui->move1, ui->move2, ui->move3, ui->move4 };

    for (int i = 0; i < 4; i++) {
        QCompleter *completer = new QCompleter(movesw[i]);
        completer->setModel(mmodel);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        movesw[i]->setCompleter(completer);
    }
}

void AdvancedSearch::setResultsWidth(int px)
{
    ui->results->setFixedWidth(px);
}
