// For clang compatibility
#include <PokemonInfo/geninfo.h>

#include <PokemonInfo/pokemoninfo.h>
#include "advancedsearch.h"
#include "ui_advancedsearch.h"
#include <QStringListModel>
#include <QCompleter>

AdvancedSearch::AdvancedSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdvancedSearch)
{
    ui->setupUi(this);

    connect(ui->search, SIGNAL(clicked()), SLOT(search()));
    connect(ui->results, SIGNAL(activated(QModelIndex)), SLOT(emitNum(QModelIndex)));
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

    ui->ability->setDisabled(gen <= 2);

    //loads the data
    QStringList types;

    for (int i = 0; i < TypeInfo::NumberOfTypes(); i++) {
        types.push_back(TypeInfo::Name(i));
    }

    QStringListModel *tmodel = new QStringListModel(types, this);
    ui->type1->setModel(tmodel);
    ui->type2->setModel(tmodel);

    QStringList abilities;

    for (int i = 0; i < AbilityInfo::NumberOfAbilities(gen.num); i++) {
        abilities.push_back(AbilityInfo::Name(i));
    }

    ui->ability->setModel(new QStringListModel(abilities, this));

    QStringList moves;

    foreach (int i, MoveInfo::Moves(gen)) {
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

    this->gen = gen;
}

void AdvancedSearch::emitNum(const QModelIndex &i)
{
    emit pokemonSelected(PokemonInfo::Number(i.data().toString()));
}

void AdvancedSearch::setResultsWidth(int px)
{
    ui->results->setFixedWidth(px);
}

void AdvancedSearch::search() {
    QVector<int> types;
    QSet<int> moves;
    int ability;
    QVector<QPair<int,int> > equalStats;
    QVector<QPair<int,int> >  minStats;
    QVector<QPair<int,int> >  maxStats;

    if (ui->useType1->isChecked()) {
        types.push_back(ui->type1->currentIndex());
    }
    if (ui->useType2->isChecked()) {
        types.push_back(ui->type2->currentIndex());
    }
    ability = AbilityInfo::Number(ui->ability->currentText());

    QLineEdit *move[] = {ui->move1, ui->move2, ui->move3, ui->move4};
    for(int i = 0; i < 4; i++) {
        moves.insert(MoveInfo::Number(move[i]->text()));
    }
    moves.remove(0);

    QLineEdit *stats[] = {ui->hpedit, ui->atkedit, ui->defedit, ui->spatkedit, ui->spdefedit, ui->speededit};
    QComboBox *statSymbols[] = {ui->hpbox, ui->atkbox, ui->defbox, ui->spatkbox, ui->spdefbox, ui->speedbox};
    for (int i = 0; i < 6; i++) {
        if (statSymbols[i]->currentIndex() == 1) {
            minStats.push_back(QPair<int, int> (i,stats[i]->text().toInt()));
        } else if (statSymbols[i]->currentIndex() == 2) {
            equalStats.push_back(QPair<int, int> (i,stats[i]->text().toInt()));
        } else if (statSymbols[i]->currentIndex() == 3) {
            maxStats.push_back(QPair<int, int> (i,stats[i]->text().toInt()));
        }
    }

    QSet<Pokemon::uniqueId> resultingList;

    for (int i = 1; i < PokemonInfo::TrueCount(); i++) {
        if (!PokemonInfo::Exists(i,gen)) {
            continue;
        }
        foreach(Pokemon::uniqueId num, PokemonInfo::Formes(i, gen)) {
            if (PokemonInfo::IsAesthetic(num)) {
                continue;
            }
            if (moves.size() > 0 && !PokemonInfo::Moves(num, gen).contains(moves)) {
                continue;
            }
            for (int j = 0; j < types.size(); j++) {
                if (PokemonInfo::Type1(num, gen) != types[j] && PokemonInfo::Type2(num, gen) != types[j])
                    goto loopend;
            }

            if (gen >= 3) {
                AbilityGroup ab = PokemonInfo::Abilities(num, gen);
                if (ability != 0 && ab.ab(0) != ability && ab.ab(1) != ability && ab.ab(2) != ability)
                    goto loopend;
            }

            {
                PokeBaseStats b = PokemonInfo::BaseStats(num, gen);
                for (int j = 0; j < equalStats.size(); j++) {
                    if (b.baseStat(equalStats[j].first) != equalStats[j].second)
                        goto loopend;
                }
                for (int j = 0; j < minStats.size(); j++) {
                    if (b.baseStat(minStats[j].first) < minStats[j].second)
                        goto loopend;
                }
                for (int j = 0; j < maxStats.size(); j++) {
                    if (b.baseStat(maxStats[j].first) > maxStats[j].second)
                        goto loopend;
                }
            }

            resultingList.insert(num);
            loopend:;
        }
    }

    ui->results->clear();
    foreach(Pokemon::uniqueId poke, resultingList) {
        ui->results->addItem(PokemonInfo::Name(poke));
    }
    ui->results->sortItems(Qt::AscendingOrder);
}
