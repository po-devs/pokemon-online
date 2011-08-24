#ifndef AVATARBOX_H
#define AVATARBOX_H

#include <QLabel>
#include <QPixmap>

/* A box specially made to display 80*80 avatars */
class AvatarBox : public QLabel {
public:
    AvatarBox(const QPixmap &pic=QPixmap());

    void changePic(const QPixmap &pic);
protected:
    QLabel *underLying;
};


#endif // AVATARBOX_H
