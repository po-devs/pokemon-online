#include <QCompleter>
#include "Utilities/keypresseater.h"
#include "PokemonInfo/pokemoninfo.h"
#include "PokemonInfo/teamholderinterface.h"
#include "TeambuilderLibrary/pokeselection.h"
#include "TeambuilderLibrary/poketablemodel.h"
#include "TeambuilderLibrary/pokemovesmodel.h"
#include "TeambuilderLibrary/theme.h"
#include "Teambuilder/engineinterface.h"
#include "pokemoneditordialog.h"
#include "ui_pokemoneditordialog.h"


namespace {
    enum LearningMethod {
        TMLearning,
        TutorLearning,
        LevelLearning,
        EggLearning,
        DWLearning,
        SpecialLearning,
        LastLearning
    };

    QString learningMethods[] = {
        "TM/HM",
        "Tutor",
        "Level",
        "Breeding",
        "Dreamworld",
        "Special"
    };
}

PokemonEditorDialog::PokemonEditorDialog(MainEngineInterface *client) :
    ui(new Ui::PokemonEditorDialog), pokeModel(nullptr), movesModel(nullptr)
{
    ui->setupUi(this);

    for (int i = 0; i < LastLearning; i++) {
        ui->learningMethod->addItem(learningMethods[i]);
    }

    QCompleter *completer = new QCompleter(MoveInfo::Names(),this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->newMove->setCompleter(completer);
    ui->newMove->installEventFilter(new KeyPressEater(this)); //Prevents Return from triggering the dialog

    setPokemon(client->trainerTeam()->team().poke(0).num());

    connect(ui->deletedMoves, SIGNAL(cellDoubleClicked(int,int)), SLOT(removeRow(int)));
    connect(ui->addedMoves, SIGNAL(cellDoubleClicked(int,int)), SLOT(removeRow(int)));
    connect(completer, SIGNAL(activated(QString)), SLOT(addMove()));
    connect(ui->newMove, SIGNAL(returnPressed()), SLOT(addMove()));
}

PokemonEditorDialog::~PokemonEditorDialog()
{
    delete ui;
}

void PokemonEditorDialog::setPokemon(Pokemon::uniqueId id)
{
    current = id;

    ui->pokemonSprite->setPixmap(PokemonInfo::Picture(current));

    if (!movesModel) {
        movesModel = new PokeMovesModel(id);
        movesModel->setParent(this);

        ui->moveChoice->setModel(movesModel);

        /* Snippet stolen from src/Teambuilder/Teambuilder/pokeedit.cpp */
        ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Name, 125);
        ui->moveChoice->horizontalHeader()->resizeSection(PokeMovesModel::Type, Theme::TypePicture(Type::Normal).width()+5);
        ui->moveChoice->setIconSize(Theme::TypePicture(Type::Normal).size());

        ui->moveChoice->sortByColumn(PokeMovesModel::Name, Qt::AscendingOrder);
        /* End snippet */

        connect(ui->moveChoice, SIGNAL(activated(QModelIndex)), SLOT(moveEntered(QModelIndex)));
    } else {
        movesModel->setPokemon(id);
    }
}

void PokemonEditorDialog::addMove()
{
    int num = MoveInfo::Number(ui->newMove->text());

    if (num == 0) {
        return;
    }

    QString move = MoveInfo::Name(num);

    ui->addedMoves->insertRow(ui->addedMoves->rowCount());
    ui->addedMoves->setItem(ui->addedMoves->rowCount()-1, 0, new QTableWidgetItem(move));
    ui->addedMoves->setItem(ui->addedMoves->rowCount()-1, 1, new QTableWidgetItem(ui->learningMethod->currentText()));

    for (int i = 0; i < ui->addedMoves->rowCount() -1; i++) {
        if (ui->addedMoves->item(i, 0)->text() == ui->addedMoves->item(ui->addedMoves->rowCount()-1, 0)->text()
                && ui->addedMoves->item(i, 1)->text() == ui->addedMoves->item(ui->addedMoves->rowCount()-1, 1)->text())
        {
            ui->addedMoves->removeRow(ui->addedMoves->rowCount()-1);
            return;
        }
    }
}

void PokemonEditorDialog::on_pokemonFrame_clicked()
{
    if (!pokeModel) {
        pokeModel = new PokeTableModel();
        pokeModel->setParent(this);
    }

    PokeSelection *p= new PokeSelection(current, pokeModel);
    p->setParent(this, Qt::Popup);
    QPoint pos = ui->pokemonFrame->mapToGlobal(ui->pokemonFrame->pos());
    p->move(pos.x() + ui->pokemonFrame->width()+10, pos.y()-ui->pokemonFrame->height()/2);
    p->setAttribute(Qt::WA_DeleteOnClose, true);
    p->show();

    connect(p, SIGNAL(pokemonChosen(Pokemon::uniqueId)), SLOT(setPokemon(Pokemon::uniqueId)));
}

void PokemonEditorDialog::moveEntered(const QModelIndex &index)
{
    Pokemon::uniqueId num = current;
    int move = index.data(CustomModel::MovenumRole).toInt();

    int learningMethod = -1;
    Pokemon::gen gen;

    if (PokemonInfo::TMMoves(num, gen).contains(move)) {
        learningMethod = TMLearning;
    } else if (PokemonInfo::TutorMoves(num, gen).contains(move)) {
        learningMethod = TutorLearning;
    } else if (PokemonInfo::LevelMoves(num, gen).contains(move)) {
        learningMethod = LevelLearning;
    } else if (PokemonInfo::EggMoves(num, gen).contains(move)) {
        learningMethod = EggLearning;
    } else if (gen.num == 5 && PokemonInfo::dreamWorldMoves(num, gen).contains(move)) {
        learningMethod = DWLearning;
    } else if (PokemonInfo::SpecialMoves(num, gen).contains(move)) {
        learningMethod = SpecialLearning;
    }

    if (learningMethod == -1) {
        return;
    }

    ui->deletedMoves->insertRow(ui->deletedMoves->rowCount());
    ui->deletedMoves->setItem(ui->deletedMoves->rowCount()-1, 0, new QTableWidgetItem(MoveInfo::Name(move)));
    ui->deletedMoves->setItem(ui->deletedMoves->rowCount()-1, 1, new QTableWidgetItem(learningMethods[learningMethod]));
}

void PokemonEditorDialog::removeRow(int row)
{
    dynamic_cast<QTableWidget*>(sender())->removeRow(row);
}
