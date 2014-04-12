#include "avatardialog.h"
#include "ui_avatardialog.h"

#include "./PokemonInfo/pokemoninfo.h"

#include <TeambuilderLibrary/theme.h>


AvatarDialog::AvatarDialog(QWidget *parent, int current) :
    QDialog(parent),
    ui(new Ui::AvatarDialog)
{
    ui->setupUi(this);

    connect(ui->btn_cancel, SIGNAL(clicked()), SLOT(close()));

    QGridLayout *layout = new QGridLayout;
    QWidget *w = new QWidget;

    QDesktopWidget *mydesk = QApplication::desktop();

    setGeometry(32, 64, mydesk->width() - 64, mydesk->height() - 128);

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
    b->setChecked(true);

    if (current != b) {
        current->setChecked(false);
        current = b;
    }
}

void AvatarDialog::on_btn_ok_clicked()
{
    emit selectionFinished(current->property("avatar").toInt());
    close();
}
