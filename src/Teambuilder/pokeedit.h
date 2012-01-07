#ifndef POKEEDIT_H
#define POKEEDIT_H

#include <QWidget>

namespace Ui {
    class PokeEdit;
}

class PokeEdit : public QWidget
{
    Q_OBJECT

public:
    explicit PokeEdit(QWidget *parent = 0);
    ~PokeEdit();

private:
    Ui::PokeEdit *ui;
};

#endif // POKEEDIT_H
