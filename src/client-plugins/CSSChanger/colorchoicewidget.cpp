#include "colorchoicewidget.h"
#include <QColorDialog>

void ColorChoiceWidget::on_pushButton_clicked()
{
    QColor color = QColorDialog::getColor(this->color);

    if (color.isValid()) {
        setColor(color);
        emit colorChanged(num, color);
    }
}
