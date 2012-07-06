#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFrame>
#include <QHash>

class QRadioButton;

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
    int numberOfTabs() const;

    void closeTab(int spot);
signals:
    void reloadMenuBar();
public slots:
    void changeSpot();
    void updateTabNames();
private:
    Ui::MainWidget *ui;

    QVector<int> spots;
    QHash<int, QRadioButton*> tabNames;

    int getIndex(int spot);
};

#endif // MAINWIDGET_H
