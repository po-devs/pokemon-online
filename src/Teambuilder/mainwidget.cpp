#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
}

MainWidget::~MainWidget()
{
    delete ui;
}

int MainWidget::getIndex(int spot)
{
    return qFind(spots, spot) - spots.begin();
}

void MainWidget::setWidget(int spot, QWidget *w)
{
    if (!spots.contains(spot)) {
        spots.push_back(spot);

        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->addWidget(w));
    } else {
        int index = getIndex(spot);

        QWidget *del = ui->stackedWidget->widget(index);
        ui->stackedWidget->removeWidget(del);
        del->deleteLater();
        ui->stackedWidget->insertWidget(index, w);
        ui->stackedWidget->setCurrentIndex(index);
    }
}

QWidget *MainWidget::currentWidget() const
{
    return ui->stackedWidget->currentWidget();
}
