#ifndef MASSREPLACEWIDGET_H
#define MASSREPLACEWIDGET_H

#include "ui_massreplace.h"

struct Data;
class QCheckBox;

class MassReplaceWidget : public QDialog, public Ui::MassReplaceDialog
{
    Q_OBJECT
public:
    MassReplaceWidget() {setupUi(this);}

    void setup(const QColor &color, Data* d);
    void setupList();
signals:
    void colorChanged(int num, QColor color);
private slots:
    void on_buttonBox_clicked(QAbstractButton*);
    void on_pushButton_clicked();
private:
    Data *data;
    QColor color;
    QVector<QCheckBox*> boxes;
};

#endif // MASSREPLACEWIDGET_H
