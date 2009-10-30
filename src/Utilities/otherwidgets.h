#ifndef OTHERWIDGETS_H
#define OTHERWIDGETS_H

#include <QtGui>

class QCompactTable : public QTableWidget
{
    Q_OBJECT
protected:
    int sizeHintForRow ( int row ) const;
public:
    QCompactTable(int row, int column);
};

/* a widget that allows giving a title to another widget */
class QEntitled : public QWidget
{
    Q_OBJECT
private:
    QLabel *m_title;
    QWidget *m_widget;
    QVBoxLayout *m_layout;

public:
    QEntitled(const QString &title = "Title", QWidget *widget = 0);
    void setTitle(const QString &title);
    void setWidget(QWidget *widget);
};

class QImageButton : public QAbstractButton
{
    Q_OBJECT
public:
    QImageButton(const QString &normal, const QString &hovered);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize maximumSize() const;
protected:
    void paintEvent(QPaintEvent *e);
private:
    QPixmap myPic, myHoveredPic;
};

class QImageBackground : public QWidget
{
    Q_OBJECT
public:
    QImageBackground(const QString &imagePath);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize maximumSize() const;
protected:
    void paintEvent(QPaintEvent *e);
private:
    QPixmap myBackground;
};

#endif // OTHERWIDGETS_H
