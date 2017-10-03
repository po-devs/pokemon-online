#ifndef COLORCHOICEWIDGET_H
#define COLORCHOICEWIDGET_H

#include "ui_colorchoice.h"

class ColorChoiceWidget : public QWidget, public Ui::Form
{
    Q_OBJECT
public:
    ColorChoiceWidget() {setupUi(this);}

    void setNumber(int num) {
        this->num = num;
        label->setText(QCoreApplication::translate("Form", "Color n\302\260%1:", 0).arg(num+1));
    }
    void setColor(const QColor &color) {
        this->color = color;
        pushButton->setStyleSheet(QString("background: %1;").arg(color.name()));
    }
    void setDesc(const QString &desc) {
        label->setText(desc);
    }

signals:
    void colorChanged(int num, QColor color);
private slots:
    void on_pushButton_clicked();
private:
    QColor color;
    int num;
};

#endif // COLORCHOICEWIDGET_H
