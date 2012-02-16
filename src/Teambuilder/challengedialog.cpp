#include "challengedialog.h"
#include "ui_challengedialog.h"

ChallengeDIalog::ChallengeDIalog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChallengeDIalog)
{
    ui->setupUi(this);
}

ChallengeDIalog::~ChallengeDIalog()
{
    delete ui;
}
