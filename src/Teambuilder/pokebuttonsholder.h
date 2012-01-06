#ifndef POKEBUTTONSHOLDER_H
#define POKEBUTTONSHOLDER_H

#include <QWidget>

namespace Ui {
    class PokeButtonsHolder;
}

class PokeButton;
class QButtonGroup;
class Team;

class PokeButtonsHolder : public QWidget
{
    Q_OBJECT

public:
    explicit PokeButtonsHolder(QWidget *parent = 0);
    ~PokeButtonsHolder();

    int currentSlot() const;
    void setTeam(Team &team);
private:
    Ui::PokeButtonsHolder *ui;
    PokeButton *pokemonButtons[6];
    QButtonGroup *group;
};

#endif // POKEBUTTONSHOLDER_H
