#ifndef BATTLINGOPTIONS_H
#define BATTLINGOPTIONS_H

#include <QtGui>

class BattlingOptionsWindow : public QWidget
{
    Q_OBJECT
public:
    BattlingOptionsWindow();
signals:
    void settingsChanged();
public slots:
    void applyChanges();
private:
    QCheckBox *sameIp;
    QCheckBox *allowCRated;
    QSpinBox *diffIps;
};

#endif // BATTLINGOPTIONS_H
