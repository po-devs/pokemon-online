#include <cassert>
#include <QCompleter>
#include <Utilities/keypresseater.h>
#include <PokemonInfo/pokemoninfo.h>
#include <PokemonInfo/teamholderinterface.h>
#include <TeambuilderLibrary/pokeselection.h>
#include <TeambuilderLibrary/poketablemodel.h>
#include <TeambuilderLibrary/pokemovesmodel.h>
#include <TeambuilderLibrary/theme.h>
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

    QString moveFiles[] = {
        "tm_and_hm_moves.txt",
        "tutor_moves.txt",
        "level_moves.txt",
        "egg_moves.txt",
        "dw_moves.txt",
        "special_moves.txt"
    };

    int learningToId(const QString &method) {
        for (int i = 0; i < sizeof(learningMethods)/sizeof(QString); i++) {
            if (learningMethods[i] == method) {
                return i;
            }
        }
        return -1;
    }

    QSet<int> pokeMoves(Pokemon::uniqueId num, int learning, Pokemon::gen gen = Pokemon::gen()) {
        if (learning == TMLearning) {
            return PokemonInfo::TMMoves(num, gen);
        } else if (learning == TutorLearning) {
            return PokemonInfo::TutorMoves(num, gen);
        } else if (learning == LevelLearning) {
            return PokemonInfo::LevelMoves(num, gen);
        } else if (learning == EggLearning) {
            return PokemonInfo::EggMoves(num, gen);
        } else if (learning == LevelLearning) {
            return PokemonInfo::LevelMoves(num, gen);
        } else if (learning == DWLearning) {
            return PokemonInfo::dreamWorldMoves(num, gen);
        } else {
            assert(learning == SpecialLearning);
            return PokemonInfo::SpecialMoves(num, gen);
        }
    }

    QString numbersToString(const QSet<int> &numbers, const QString &sep=" ") {
        bool first = true;

        QList<int> sorted = numbers.toList();
        qSort(sorted);

        QString ret;
        foreach(int num, numbers) {
            if (first) {
                first = false;
            } else {
                ret += sep;
            }
            ret += QString::number(num);
        }

        return ret;
    }
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

void PokemonEditorDialog::saveChanges()
{
    /* So we have some moves deleted and some added. */
    /*
     * Prologue: update in-memory database
     * First step: Update the corresponding move files for the pokemon.
     * Second step: Update the pre evo moves of all the evos of the pokemon.
     * Third step: Regenerate all_moves for all affected pokemon
     */

    QHash<int, QSet<int> > addedMoves, deletedMoves;
    QHash<int, QSet<int> > finalMoves;

    for (int i = 0; i < ui->addedMoves->rowCount() -1; i++) {
        addedMoves[learningToId(ui->addedMoves->item(i, 1)->text())].insert(MoveInfo::Number(ui->addedMoves->item(i, 0)->text()));
    }
    for (int i = 0; i < ui->deletedMoves->rowCount() -1; i++) {
        deletedMoves[learningToId(ui->deletedMoves->item(i, 1)->text())].insert(MoveInfo::Number(ui->deletedMoves->item(i, 0)->text()));
    }

    QSet<int> keys = addedMoves.keys().toSet().unite(deletedMoves.keys().toSet());

    foreach(int key, keys) {
        finalMoves[key] = pokeMoves(current, key);
        finalMoves[key].unite(addedMoves[key]);
        finalMoves[key].subtract(deletedMoves[key]);
    }

    /* We now have all the relevant move lists in finalMoves. Now to update the files */
    updateFiles(current, finalMoves);
}

void PokemonEditorDialog::updateFiles(Pokemon::uniqueId num, QHash<int, QSet<int> > moves, Pokemon::gen g)
{
    foreach(int key, moves.keys()) {
        QString file = PokemonInfoConfig::dataRepo() + QString("/db/pokes/%1G/Subgen %2/%3").arg(
                        int(g.num)).arg(g.subnum == g.wholeGen ? GenInfo::NumberOfSubgens(g.num) : int(g.subnum)).arg(moveFiles[key]);

        QStringList lines = QString::fromUtf8(getFileContent(file)).split("\n");

        bool found = false;
        for (int i = 0; i < lines.length(); i++) {
            Pokemon::uniqueId id;
            QString data;
            Pokemon::uniqueId::extract(lines[i], id, data);

            if (id != num) {
                continue;
            }

            found = true;

            lines[i] = num.toString() + " " + numbersToString(moves[key]);

            break;
        }

        if (!found) {
            for (int i = 0; i < lines.length(); i++) {
                Pokemon::uniqueId id;
                QString data;
                Pokemon::uniqueId::extract(lines[i], id, data);

                if (id > num) {
                    lines.insert(i, num.toString() + " " + numbersToString(moves[key]));
                    break;
                }
            }
        }

        writeFileContent(file, lines.join("\n").toUtf8());
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
