#include "trainermenu.h"
#include "ui_trainermenu.h"

TrainerMenu::TrainerMenu(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TrainerMenu)
{
    ui->setupUi(this);
}

TrainerMenu::~TrainerMenu()
{
    delete ui;
}
