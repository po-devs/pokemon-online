#ifndef EVBOX_H
#define EVBOX_H

#include <QWidget>

namespace Ui {
    class EvBox;
}

class EvBox : public QWidget
{
    Q_OBJECT

public:
    explicit EvBox(QWidget *parent = 0);
    ~EvBox();

private:
    Ui::EvBox *ui;
};

#endif // EVBOX_H
