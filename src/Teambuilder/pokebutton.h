#ifndef POKEBUTTON_H
#define POKEBUTTON_H

#include <QPushButton>

namespace Ui {
    class PokeButton;
}

class PokeButton : public QPushButton
{
    Q_OBJECT

public:
    explicit PokeButton(QWidget *parent = 0);
    ~PokeButton();

    void setNumber(int x);
private:
    Ui::PokeButton *ui;
};

#endif // POKEBUTTON_H
