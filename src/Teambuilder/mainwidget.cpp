#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QShortcut>
#include "../Utilities/qclicklabel.h"

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
        ui->topLabel->hide();

        QHBoxLayout *layout = dynamic_cast<QHBoxLayout*>(ui->topWidget->layout());
        layout->insertWidget(spots.count()-1, tabNames[spot] = new QClickLabel());

        QShortcut *sh = new QShortcut(this);
        sh->setProperty("tab-window", spot);
        shortCuts[spot] = sh;
        connect(sh, SIGNAL(activated()), SLOT(changeSpot()));
        connect(sh, SIGNAL(activatedAmbiguously()), SLOT(changeSpot()));
        tabNames[spot]->setProperty("tab-window", spot);
        connect(tabNames[spot], SIGNAL(clicked()), SLOT(changeSpot()));

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
    shortCuts[spot]->setKey(QKeySequence());
    shortCuts[spot]->deleteLater();

    updateTabNames();
}

int MainWidget::numberOfTabs() const
{
    return spots.count();
}

void MainWidget::updateTabNames()
{
    if (spots.count() > 1) {
        for (int i = 0; i < spots.size(); i++) {
            tabNames[spots[i]]->setEnabled(i == ui->stackedWidget->currentIndex());
            tabNames[spots[i]]->setText(QString("<u>%1</u>. %2").arg(i+1).arg(ui->stackedWidget->widget(i)->windowTitle()));
            shortCuts[spots[i]]->setKey(Qt::ALT+Qt::Key_0+ i + 1);
            tabNames[spots[i]]->show();
        }
    } else {
        tabNames[spots[0]]->hide();
    }

    topLevelWidget()->setWindowTitle(QString("Pokemon Online - %1").arg(currentWidget()->windowTitle()));
}

void MainWidget::changeSpot()
{
    int spot = sender()->property("tab-window").toInt();
    ui->stackedWidget->setCurrentIndex(getIndex(spot));

    updateTabNames();
    emit reloadMenuBar();
}

QWidget *MainWidget::currentWidget() const
{
    return ui->stackedWidget->currentWidget();
}
