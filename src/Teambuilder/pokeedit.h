#ifndef POKEEDIT_H
#define POKEEDIT_H

#include <QWidget>

namespace Ui {
    class PokeEdit;
}

class PokeTeam;
class QAbstractItemModel;

class PokeEdit : public QWidget
{
    Q_OBJECT

public:
    explicit PokeEdit(PokeTeam *poke, QAbstractItemModel *itemModel, QAbstractItemModel *natureModel);
    ~PokeEdit();
signals:
    void switchToTrainer();

private slots:
    void on_done_clicked() {emit switchToTrainer();}

public slots:
    void changeHappiness(int newHappiness);
    void changeNature(int newNature);

private:
    Ui::PokeEdit *ui;
    PokeTeam *m_poke;

    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void updateAll();
    void setItem(int num);
};

#endif // POKEEDIT_H
