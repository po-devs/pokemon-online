#ifndef BATTLINGOPTIONS_H
#define BATTLINGOPTIONS_H

#include <QWidget>

class QCheckBox;
class QSpinBox;
class QLabel;

class BattlingOptionsWindow : public QWidget
{
    Q_OBJECT
public:
    BattlingOptionsWindow();
signals:
    void settingsChanged();
public slots:
    void applyChanges();
    void updateLabel();
private:
    QCheckBox *sameIp;
    QCheckBox *allowCRated;
    QSpinBox *diffIps;

    QLabel *desc;
    QSpinBox *months, *percent, *hours, *periods, *max_decay;
    QCheckBox *processOnStartUp;
};

#endif // BATTLINGOPTIONS_H
