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
    QImageButton(const QString &normal, const QString &hovered, const QString &checked ="");
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize maximumSize() const;

    void changePics(const QString &normal, const QString &hovered, const QString &checked = "");
protected:
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
private:
    QPixmap myPic, myHoveredPic, myCheckedPic;
    int lastUnderMouse; // last mouse pos recorded
    bool pressed;

    enum State {
        Normal,
        Hovered,
        Checked
    };
    int lastState;
};

/* A QListWidgetItem with an id, for convenience */

class QIdTreeWidgetItem : public QTreeWidgetItem
{
public:
    QIdTreeWidgetItem(int id, const QStringList &text);

    int id() const;
    void setColor(const QColor &c);
private:
    int myid;
    int mylevel;
    bool operator<(const QTreeWidgetItem &other)const {
        int column = treeWidget()->sortColumn();
        return text(column).toLower() < other.text(column).toLower();
     }
};


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

    void setAutoClear(bool a) {
        autoClear = a;
    }
    void insertHtml(const QString &text);
    void insertPlainText(const QString &text);

private:
    int linecount;
    bool autoClear;
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
/* I have no idea if this will work, but I'm trying :p*/
class QImageButtonLR : public QImageButton
{
    Q_OBJECT
public:
    QImageButtonLR(const QString &normal, const QString &hovered);
protected:
    void mouseReleaseEvent(QMouseEvent *ev);
signals:
    void leftClick();
    void rightClick();
};

/* A Progress bar that emits a signal when clicked on */
class QClickPBar : public QProgressBar
{
    Q_OBJECT
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *);
};

/* A dummy widget that accepts keyboard events */
class QDummyGrabber : public QPushButton
{
    Q_OBJECT
public:
    QDummyGrabber();
};

class QSideBySide : public QHBoxLayout
{
public:
    QSideBySide(QWidget *a, QWidget *b);
};

#endif // OTHERWIDGETS_H
