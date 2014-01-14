#ifndef QIMAGEBUTTONLR_H
#define QIMAGEBUTTONLR_H

#include "otherwidgets.h"

class QImageButtonLR : public QImageButton
{
    Q_OBJECT
public:
    QImageButtonLR(QWidget*w=0);
    QImageButtonLR(const QString &normal, const QString &hovered);
protected:
    void mouseReleaseEvent(QMouseEvent *ev);
signals:
    void leftClick();
    void rightClick();
};

#endif // QIMAGEBUTTONLR_H
