#ifndef OTHERWIDGETS_H
#define OTHERWIDGETS_H

#include <QTableWidget>
#include <QList>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QMenu>
#include <QLineEdit>
#include <QMimeData>
#include <QLabel>

class QVBoxLayout;
class QCompleter;
/*
    Those are widgets that Qt lacks, and that are to use like Qt Widgets
*/

/* A table as compact as possible (i.e the rows' heights is the least possible) */
class QCompactTable : public QTableWidget
{
    Q_OBJECT
public:
    QCompactTable(int row, int column);
    static void makeCompact(QTableView *view);
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
    QImageButton(QWidget *w=0);
    QImageButton(const QString &normal, const QString &hovered, const QString &checked ="");
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize maximumSize() const;

    void changePics(const QString &normal, const QString &hovered, const QString &checked = "");
    void changePics(const QPixmap &normal, const QPixmap &hovered, const QPixmap &checked);
protected:
    void paintEvent(QPaintEvent *e);
    bool event(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
private:
    QPixmap myPic, myHoveredPic, myCheckedPic;
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
    QIdListWidgetItem(int id, const QIcon &icon, const QString &text);
    bool operator<(const QListWidgetItem & item) const;
    int id() const;
    void setId(int id);
    void setColor(const QColor &c);
private:
    int myid;
};
/* A textbrowser that scrolls down automatically, unless not down, and that
   always insert the text at the end */
/* validator for the nicks */
class QNickValidator : public QValidator
{
    Q_OBJECT
public:
    QNickValidator(QWidget *parent);
    QNickValidator(QWidget *parent, uint charmax);

    bool isBegEndChar(QChar ch) const;
    void fixup(QString &input) const;
    State validate(QString &input, int &pos) const;
    State validate(const QString &input) const;
private:
    int charMax;
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

class QExposedTabWidget : public QTabWidget
{
public:
    QTabBar * tabBar() { return QTabWidget::tabBar(); }
};

/* A new Button with pressed pic*/
class QImageButtonP : public QAbstractButton
{
    Q_OBJECT
public:
    QImageButtonP(const QString &normal, const QString &hovered, const QString &pressed, const QString &checked ="");
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize maximumSize() const;

    void changePics(const QString &normal, const QString &hovered, const QString &pressed, const QString &checked = "");
protected:
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
private:
    QPixmap myPic, myHoveredPic, myCheckedPic, myPressedPic;
    int lastUnderMouse; // last mouse pos recorded
    bool bpressed;

    enum State {
        Normal,
        Hovered,
        Checked,
        Pressed
    };
    int lastState;
};

/* A new LineEdit that like IRC chat*/
class QIRCLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    QIRCLineEdit();

    void setPlayers(QAbstractItemModel *players);
    void clear();
private:
    bool event(QEvent *);
    void keyPressEvent(QKeyEvent *);
    //QList<QString> m_Inputlist1;//Stores the inputed strings,up list.
    //QList<QString> m_Inputlist2;//Stores the inputed strings,down list.
    QList<QString> m_Inputlist;
    quint16 listindex;
    QCompleter *completer;
    int completeIndex;
    QString beginning;
    //QString m_Currentline;//Stores a copy of the current text in the LineEdit.
};

class QMimeOnDesktop : public QMimeData
{
public:
    void setExtension(const QString &ext);
    void setData(const QByteArray &data);
protected:
    QVariant retrieveData ( const QString & mimeType, QVariant::Type type ) const;
private:
    QString ext;
    QByteArray data;
    QString path;
};

class QDraggableLabel : public QLabel
{
private:
    void mousePressEvent(QMouseEvent *event);
};

class QDragReactiveTabWidget : public QTabWidget
{
public:
    QDragReactiveTabWidget(QWidget *parent=NULL);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
};

#endif // OTHERWIDGETS_H
