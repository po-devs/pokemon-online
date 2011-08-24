#include "avatarbox.h"

AvatarBox::AvatarBox(const QPixmap &pic)
{
    setObjectName("AvatarBox");
    setFixedSize(82,82);

    underLying = new QLabel(this);
    underLying->setPixmap(pic);
    underLying->move(1,1);
}

void AvatarBox::changePic(const QPixmap &pic)
{
    underLying->setPixmap(pic);
    underLying->setFixedSize(pic.size());
    underLying->move( (82 - pic.width())/2, 81-pic.height());
}
