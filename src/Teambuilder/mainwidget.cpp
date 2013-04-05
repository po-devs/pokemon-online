#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "../Shared/config.h"

#include <QRadioButton>

MainWidget::MainWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    ui->versionLabel->setText(QString("<html>Pok&eacute;mon Online Simulator v%1</html>").arg(VERSION));
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
        ui->topLabel->hide();

        QHBoxLayout *layout = dynamic_cast<QHBoxLayout*>(ui->topWidget->layout());
        layout->insertWidget(spots.count()-1, tabNames[spot] = new QRadioButton());
        tabNames[spot]->setFocusPolicy(Qt::NoFocus);

        tabNames[spot]->setProperty("tab-window", spot);
        connect(tabNames[spot], SIGNAL(toggled(bool)), SLOT(changeSpot()));

        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->addWidget(w));
    } else {
        int index = getIndex(spot);

        QWidget *del = ui->stackedWidget->widget(index);
        ui->stackedWidget->removeWidget(del);
        del->deleteLater();
        ui->stackedWidget->insertWidget(index, w);
        ui->stackedWidget->setCurrentIndex(index);
    }

    updateTabNames();
}

void MainWidget::closeTab(int spot)
{
    QHBoxLayout *layout = dynamic_cast<QHBoxLayout*>(ui->topWidget->layout());
    layout->removeWidget(tabNames[spot]);
    delete tabNames.take(spot);

    QWidget *del = ui->stackedWidget->widget(getIndex(spot));
    ui->stackedWidget->removeWidget(del);
    del->deleteLater();

    spots.remove(getIndex(spot), 1);

    updateTabNames();
}

int MainWidget::numberOfTabs() const
{
    return spots.count();
}

void MainWidget::updateTabNames()
{
    tabNames[currentWidget()->property("tab-window").toInt()]->setChecked(true);

    if (spots.count() > 1) {
        for (int i = 0; i < spots.size(); i++) {
            tabNames[spots[i]]->setText(QString("&%1. %2").arg(i+1).arg(ui->stackedWidget->widget(i)->windowTitle()));
            tabNames[spots[i]]->show();
        }
    } else {
        tabNames[spots[0]]->hide();
    }

    topLevelWidget()->setWindowTitle(QString("%1 - Pokemon Online").arg(currentWidget()->windowTitle()));
}

void MainWidget::changeSpot()
{
    QRadioButton *r = dynamic_cast<QRadioButton*>(sender());
    if (r && !r->isChecked()) {
        return;
    }

    int spot = sender()->property("tab-window").toInt();
    ui->stackedWidget->setCurrentIndex(getIndex(spot));

    updateTabNames();
    emit reloadMenuBar();
}

QWidget *MainWidget::currentWidget() const
{
    return ui->stackedWidget->currentWidget();
}
