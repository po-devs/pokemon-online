#include "massreplacewidget.h"
#include "data.h"
#include <QCheckBox>
#include <QColorDialog>

void MassReplaceWidget::setup(const QColor &color, Data *d)
{
    data = d;
    this->color = color;
    pushButton->setStyleSheet(QString("background: %1;").arg(color.name()));

    setupList();
}

void MassReplaceWidget::setupList()
{
    foreach(QCheckBox* box, boxes) {
        delete box;
    }
    boxes.clear();

    QVector<int> finds = data->findColor(color);

    for (int i = 0; i < finds.size(); i++) {
        QCheckBox *check = new QCheckBox(data->colors[finds[i]].desc);
        check->setChecked(true);
        check->setProperty("associated-position", finds[i]);
        boxes.push_back(check);
        findList->addWidget(check);
    }
}

void MassReplaceWidget::on_buttonBox_clicked(QAbstractButton *b)
{
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(b);

    if (role == QDialogButtonBox::RejectRole) {
        reject();
    } else if (role == QDialogButtonBox::AcceptRole) {
        accept();
    }
}

void MassReplaceWidget::on_pushButton_clicked()
{
    QColor color = QColorDialog::getColor(this->color);

    if (color.isValid() && color != this->color) {
        foreach(QCheckBox *box, boxes) {
            if (box->isChecked()) {
                emit colorChanged(box->property("associated-position").toInt(), color);
            }
        }

        this->color = color;
        pushButton->setStyleSheet(QString("background: %1;").arg(color.name()));

        setupList();
    }
}
