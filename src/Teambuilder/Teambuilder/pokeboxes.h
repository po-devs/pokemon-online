#ifndef POKEBOXES_H
#define POKEBOXES_H

#include <QFrame>

namespace Ui {
class PokeBoxes;
}

class PokeBoxes : public QFrame
{
    Q_OBJECT
    
public:
    explicit PokeBoxes(QWidget *parent = 0);
    ~PokeBoxes();
    
private:
    Ui::PokeBoxes *ui;
};

#endif // POKEBOXES_H
