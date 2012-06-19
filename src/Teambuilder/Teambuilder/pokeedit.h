#ifndef POKEEDIT_H
#define POKEEDIT_H

#include <QWidget>

namespace Ui {
    class PokeEdit;
}

namespace Pokemon {
    class uniqueId;
}

class PokeTeam;
class QAbstractItemModel;
class PokeMovesModel;
class QLineEdit;
class QModelIndex;

class PokeEdit : public QWidget
{
    Q_OBJECT

public:
    explicit PokeEdit(PokeTeam *poke, QAbstractItemModel *pokeModel, QAbstractItemModel *itemModel, QAbstractItemModel *natureModel);
    ~PokeEdit();

signals:
    void switchToTrainer();

private slots:
    void on_done_clicked() {emit switchToTrainer();}

public slots:
    void updateStats();
    void updatePicture();
    void updateGender();
    void updateAll();
    void setPoke(PokeTeam *poke);

    void changeHappiness(int newHappiness);
    void changeNature(int newNature);
    void changeItem(const QString &newItem);
    void setNature(int index);
    void setNum(const Pokemon::uniqueId &num);
signals:
    void numChanged();
private slots:
    void changeMove();
    void moveEntered(const QModelIndex&);

    void on_pokemonFrame_clicked();
private:
    Ui::PokeEdit *ui;
    QLineEdit *m_moves[4];
    PokeMovesModel *movesModel;
    QAbstractItemModel *pokemonModel;
    PokeTeam *m_poke;

    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void updateItemSprite(int newItem);
    void setItem(int num);
    void setMove(int slot, int move);
};

#endif // POKEEDIT_H
