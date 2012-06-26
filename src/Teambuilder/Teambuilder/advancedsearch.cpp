#include "advancedsearch.h"
#include "ui_advancedsearch.h"

AdvancedSearch::AdvancedSearch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdvancedSearch)
{
    ui->setupUi(this);
}

AdvancedSearch::~AdvancedSearch()
{
    delete ui;
}

void AdvancedSearch::setResultsWidth(int px)
{
    ui->results->setFixedWidth(px);
}
