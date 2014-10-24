#include <QDesktopWidget>
#include "./PokemonInfo/pokemoninfo.h"

#include <TeambuilderLibrary/theme.h>


#include "avatardialog.h"
#include "ui_avatardialog.h"


AvatarDialog::AvatarDialog(QWidget *parent, int current) :
    QDialog(parent),
    ui(new Ui::AvatarDialog)
{
    ui->setupUi(this);

    connect(ui->btn_cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout *layout = new QGridLayout;
    QWidget *w = new QWidget;

    int sxpad = 64;
    int xpad = (parent->width() > 900 + sxpad * 2 ? (parent->width() - 900) / 2 : sxpad);
    int ypad = 64;

    QPoint g = parent->mapToGlobal(QPoint(0, 0));
    QRect rect = QRect(g.x() + xpad, g.y() + ypad, parent->width() - xpad * 2, parent->height() - ypad * 2);

    setGeometry(rect);

    int perRow = qMax(geometry().width() / 96 - 2, 1);

    for (int i = 0; i < 300; i++) {
        QPushButton *b = new QPushButton(QIcon(Theme::TrainerSprite(i + 1)), "");

        b->setProperty("avatar", i + 1);
        b->setIconSize(QSize(96, 96));
        b->setMinimumSize(QSize(96, 96));
        b->setMaximumSize(QSize(96, 96));
        b->setCheckable(true);

        connect(b, SIGNAL(clicked()), SLOT(avatarClicked()));

        layout->addWidget(b, floor(i / perRow), i % perRow);

        if (i + 1 == current) {
            this->current = b;
            b->setChecked(true);
        }
    }

    w->setObjectName("avatarWidget");
    w->setLayout(layout);
    ui->scrollArea_avatars->setWidget(w);
    ui->scrollArea_avatars->ensureWidgetVisible(this->current, 50, -96);
}

AvatarDialog::~AvatarDialog()
{
    delete ui;
}

void AvatarDialog::avatarClicked()
{
    QPushButton *b = dynamic_cast<QPushButton *>(sender());

    if (current != b) {
        b->setChecked(true);
        current->setChecked(false);
        current = b;
    } else {
        on_btn_ok_clicked();
    }
}

void AvatarDialog::on_btn_ok_clicked()
{
    emit selectionFinished(current->property("avatar").toInt());
    close();
}
