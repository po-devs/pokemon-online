#ifndef TRAINERMENU_H
#define TRAINERMENU_H

#include <QFrame>
#include "centralwidget.h"

namespace Ui {
    class TrainerMenu;
}

class TrainerMenu : public QFrame, public CentralWidgetInterface
{
    Q_OBJECT

public:
    explicit TrainerMenu(QWidget *parent = 0);
    ~TrainerMenu();
signals:
    void done();
private slots:
    void on_close_clicked(){emit done();}
private:
    Ui::TrainerMenu *ui;
};

#endif // TRAINERMENU_H
