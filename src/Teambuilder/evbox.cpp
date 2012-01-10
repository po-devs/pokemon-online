#include "evbox.h"
#include "ui_evbox.h"

EvBox::EvBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EvBox)
{
    ui->setupUi(this);
}

EvBox::~EvBox()
{
    delete ui;
}
