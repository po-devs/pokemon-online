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
    void updatePoke(int slot);
    void setTeam(Team &team);
    void setCurrentSlot(int slot);
signals:
    void teamChanged();
    void clicked(int index);
    void doubleClicked(int index);
    void dropEvent(int index, QDropEvent *event);
private:
    Ui::PokeButtonsHolder *ui;
    PokeButton *pokemonButtons[6];
    QButtonGroup *group;

    Team *m_team;
    Team &team() {return *m_team;}
};

#endif // POKEBUTTONSHOLDER_H
