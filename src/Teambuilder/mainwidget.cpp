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

void MainWidget::setWidget(QWidget *w)
{
    ui->stackedWidget->addWidget(w);
    if (ui->stackedWidget->count() > 1) {
        QWidget *toDel = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(toDel);
        toDel->deleteLater();
    }
}

QWidget *MainWidget::currentWidget() const
{
    return ui->stackedWidget->currentWidget();
}
