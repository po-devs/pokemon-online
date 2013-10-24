#ifndef POKEDEX_H
#define POKEDEX_H

#include <QWidget>

namespace Ui {
class Pokedex;
}

class Pokedex : public QWidget
{
    Q_OBJECT

public:
    explicit Pokedex(QWidget *parent = 0);
    ~Pokedex();

private:
    Ui::Pokedex *ui;
};

#endif // POKEDEX_H
