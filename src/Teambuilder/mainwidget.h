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
    
    void setWidget(int spot, QWidget *w);
    QWidget *currentWidget() const;
private:
    Ui::MainWidget *ui;

    QVector<int> spots;
    int getIndex(int spot);
};

#endif // MAINWIDGET_H
