#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "../PokemonInfo/pokemonstructs.h"

namespace Ui {
    class MainWindow;
}

enum {
    LevelMoves,
    TutorMoves,
    EggMoves,
    SpecialMoves,
    TMMoves,
    PreEvoMoves,
    DreamWorldMoves
};

struct MoveGen
{
    int gen;
    Pokemon::uniqueId id;
    QSet<int> moves[7];

    void init(int gen, Pokemon::uniqueId id);
};

struct MovesPerPoke
{
    Pokemon::uniqueId id;
    MoveGen gens[5];

    void init(Pokemon::uniqueId id);
};

struct PokeMovesDb
{
    void init();

    QHash<Pokemon::uniqueId, MovesPerPoke> pokes;

    void save();
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void pokemonChosen(QListWidgetItem*);
    void setPokeByNick();
    void save();
    void moveChosen(QListWidgetItem*);
    void moveDeleted(QListWidgetItem*);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    Pokemon::uniqueId currentPoke;

    void switchToPokemon(Pokemon::uniqueId);
    int gen();
    PokeMovesDb database;
    void addMoves(int gen, int cat, QListWidget *container);
};

#endif // MAINWINDOW_H
