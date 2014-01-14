#include <QtCore>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <TeambuilderLibrary//themeaccessor.h>

#include "colorchoicewidget.h"
#include "massreplacewidget.h"
#include "csswidget.h"

CssWidget::CssWidget(ThemeAccessor* theme) : theme(theme) {
    setupUi(this);

    path = theme->path("default.css");
    QFile stylesheet(path);
    stylesheet.open(QIODevice::ReadOnly);
    QString sheet = QString::fromUtf8(stylesheet.readAll());

    data.stylesheet = sheet;

    QRegExp regExp("(#[A-Fa-f0-9]{6})\\b");
    QRegExp comment("/\\*([^*]|\\*(?!/))*\\*/");
    QRegExp misc("/\\*|\\*/");

    int pos = 0;

    while ((pos = regExp.indexIn(sheet, pos)) != -1) {

        int acc = sheet.lastIndexOf("{", pos);
        if (acc != -1) {
            int acc2 = sheet.lastIndexOf("}", acc);

            if (acc2 == -1) {
                acc2 = 0;
            } else {
                acc2++;
            }
            QString desc = sheet.mid(acc2, acc-acc2).replace(comment, "").replace(misc, "").trimmed();

            data.colors << Data::PosValue(pos, QColor(regExp.cap(1)), desc);
        } else {
            data.colors << Data::PosValue(pos, QColor(regExp.cap(1)));
        }

        pos += regExp.matchedLength();
    }

    setupList();
}

void CssWidget::setupList()
{
    QLayoutItem *child;
    while ((child = colorsList->takeAt(0)) != 0) {
      delete child;
    }

    for (int i = 0; i < data.colors.size(); i++) {
        ColorChoiceWidget *w = new ColorChoiceWidget();
        w->setNumber(i);
        w->setColor(data.colors[i].value);
        if (!data.colors[i].desc.isEmpty()) {
            w->setDesc(data.colors[i].desc);
        }
        colorsList->addWidget(w);

        connect(w, SIGNAL(colorChanged(int,QColor)), SLOT(onColorChanged(int,QColor)));
    }
}

void CssWidget::on_tabWidget_currentChanged(QWidget *w)
{
    if (w == individualColors) {
        setupList();
    } else if (w == massReplace) {
        setupGrid();
    }
}

void CssWidget::setupGrid()
{
    QLayoutItem *child;
    while ((child = colorGrid->takeAt(0)) != 0) {
      delete child;
    }

    QSet<QString> colors;

    int count = 0;
    for (int i = 0; i < data.colors.size(); i++) {
        QColor color = data.colors[i].value;
        if (colors.contains(color.name())) {
            continue;
        }
        colors.insert(color.name());
        QPushButton *button = new QPushButton();
        button->setStyleSheet(QString("background: %1;").arg(color.name()));
        button->setProperty("associated-color", color.name());
        connect(button, SIGNAL(clicked()), SLOT(openMassColor()));
        colorGrid->addWidget(button, count/4, count%4);
        count ++;
    }
}

void CssWidget::openMassColor()
{
    QColor color(sender()->property("associated-color").toString());

    MassReplaceWidget *w = new MassReplaceWidget();
    w->setParent(this);
    w->setup(color, &data);
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->setWindowFlags(Qt::Dialog);
    w->show();

    connect(w, SIGNAL(colorChanged(int,QColor)), SLOT(onColorChanged(int,QColor)));
    connect(w, SIGNAL(accepted()), SLOT(onApply()));
    connect(w, SIGNAL(accepted()), SLOT(updateGrid()));
}

void CssWidget::updateGrid()
{
    if (tabWidget->currentWidget() == massReplace) {
        setupGrid();
    }
}

QVector<int> Data::findColor(const QColor &c)
{
    QVector<int> ret;
    for (int i = 0; i < colors.size(); i++) {
        if (colors[i].value == c) {
            ret.push_back(i);
        }
    }

    return ret;
}

void CssWidget::onColorChanged(int num, QColor color)
{
    data.colors[num].value = color;
    data.stylesheet.replace(data.colors[num].pos, color.name().length(), color.name());
}

void CssWidget::onApply()
{
    qApp->setStyleSheet(data.stylesheet);
}

void CssWidget::onAccept()
{
    onApply();
    QFile out(path);
    out.open(QIODevice::WriteOnly);
    out.write(data.stylesheet.toUtf8());
    out.close();;
}

void CssWidget::on_buttonBox_clicked(QAbstractButton *b)
{
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(b);

    if (role == QDialogButtonBox::RejectRole) {
        reject();
    } else if (role == QDialogButtonBox::ApplyRole) {
        onApply();
    } else if (role == QDialogButtonBox::AcceptRole) {
        onAccept();
        accept();
    }
}
