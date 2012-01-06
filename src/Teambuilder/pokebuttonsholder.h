#ifndef POKEBUTTONSHOLDER_H
#define POKEBUTTONSHOLDER_H

#include <QWidget>

namespace Ui {
    class PokeButtonsHolder;
}

class PokeButton;

class PokeButtonsHolder : public QWidget
{
    Q_OBJECT

public:
    explicit PokeButtonsHolder(QWidget *parent = 0);
    ~PokeButtonsHolder();

private:
    Ui::PokeButtonsHolder *ui;
    PokeButton *pokemonButtons[6];
};

#endif // POKEBUTTONSHOLDER_H
