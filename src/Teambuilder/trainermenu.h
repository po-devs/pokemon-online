#ifndef TRAINERMENU_H
#define TRAINERMENU_H

#include <QFrame>

namespace Ui {
    class TrainerMenu;
}

class TrainerMenu : public QFrame
{
    Q_OBJECT

public:
    explicit TrainerMenu(QWidget *parent = 0);
    ~TrainerMenu();

private:
    Ui::TrainerMenu *ui;
};

#endif // TRAINERMENU_H
