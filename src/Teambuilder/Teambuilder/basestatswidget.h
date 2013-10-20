#ifndef BASESTATSWIDGET_H
#define BASESTATSWIDGET_H

#include <QWidget>

namespace Pokemon {
    class uniqueId;
    class gen;
}

class QProgressBar;

namespace Ui {
class BaseStatsWidget;
}

class BaseStatsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit BaseStatsWidget(QWidget *parent = 0);
    ~BaseStatsWidget();
public slots:
    void setGen(const Pokemon::gen &gen);
    void setNum(const Pokemon::uniqueId &num, const Pokemon::gen &gen);
private:
    Ui::BaseStatsWidget *ui;

    QProgressBar *stats[6];

    Pokemon::gen *curgen;
};

#endif // BASESTATSWIDGET_H
