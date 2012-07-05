#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFrame>

namespace Ui {
class MainWidget;
}

class MainWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    
    void setWidget(QWidget *w);
    QWidget *currentWidget() const;
private:
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H
