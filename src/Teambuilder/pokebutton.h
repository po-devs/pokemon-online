#ifndef POKEBUTTON_H
#define POKEBUTTON_H

#include <QPushButton>

namespace Ui {
    class PokeButton;
}

class PokeTeam;

class PokeButton : public QPushButton
{
    Q_OBJECT

public:
    explicit PokeButton(QWidget *parent = 0);
    ~PokeButton();

    void setNumber(int x);
    void setPokemon(PokeTeam &poke);
signals:
    void pokemonOrderChanged(int start, int end);
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void startDrag();
private:
    Ui::PokeButton *ui;
    QPoint startPos;

    PokeTeam *m_poke;
    int num;
    PokeTeam &poke() {return *m_poke;}
};

#endif // POKEBUTTON_H
