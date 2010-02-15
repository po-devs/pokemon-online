#ifndef OTHERWIDGETS_H
#define OTHERWIDGETS_H

#include <QtGui>

/*
    Those are widgets that Qt lacks, and that are to use like Qt Widgets
*/

/* A table as compact as possible (i.e the rows' heights is the least possible) */
class QCompactTable : public QTableWidget
{
    Q_OBJECT
public:
    QCompactTable(int row, int column);
};

/* A widget that allows giving a title to another widget
   The title appears at the top of the widget */
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

/* A button which is actually an image. There are two params:
    -The image that should be displayed when the button is normal
    -The image that should be displayed when the button is hovered */
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
    void mouseMoveEvent(QMouseEvent *e);
private:
    QPixmap myPic, myHoveredPic;
    int lastUnderMouse; // last mouse pos recorded
};

/* A widget that sets its size to the background given in parameter.
   Actually it changes its background to the one given */
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

/* A QListWidgetItem with an id, for convenience */
class QIdListWidgetItem : public QListWidgetItem
{
public:
    QIdListWidgetItem(int id, const QString &text);

    int id() const;
    void setColor(const QColor &c);
private:
    int myid;
};

/* A textedit that scrolls down automatically, unless not down, and that
   always insert the text at the end */
class QScrollDownTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    QScrollDownTextEdit();

    void insertHtml(const QString &text);
    void insertPlainText(const QString &text);

private:
    int linecount;
};

/* validator for the nicks */
class QNickValidator : public QValidator
{
    Q_OBJECT
public:
    QNickValidator(QWidget *parent);

    bool isBegEndChar(QChar ch) const;
    void fixup(QString &input) const;
    State validate(QString &input, int &pos) const;
    State validate(const QString &input) const;
};

#endif // OTHERWIDGETS_H
