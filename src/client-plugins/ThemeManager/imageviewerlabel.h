#ifndef IMAGEVIEWERLABEL_H
#define IMAGEVIEWERLABEL_H

#include <QLabel>

class ImageViewerLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageViewerLabel(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *);

signals:
    void leftPressed();
    void rightPressed();

};

#endif // IMAGEVIEWERLABEL_H
