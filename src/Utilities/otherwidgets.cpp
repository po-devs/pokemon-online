#include "otherwidgets.h"
#include <QtGui>

QCompactTable::QCompactTable(int row, int column)
        : QTableWidget(row, column)
{
    verticalHeader()->setDefaultSectionSize(22);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(false);
    verticalHeader()->hide();
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QEntitled::QEntitled(const QString &title, QWidget *widget)
{
    m_layout = new QVBoxLayout(this);

    /* The space is there for correct alignment of the title */
    m_title = new QLabel( title);
    m_title->setMaximumHeight(17);

    m_layout->addWidget(m_title, 0, Qt::AlignBottom);
    if (widget)
        m_widget = widget;
    else
        m_widget = new QWidget();
    m_layout->addWidget(m_widget, 100, Qt::AlignTop);
    m_title->setBuddy(m_widget);

    /* Makes the title/items stick together */
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
}

void QEntitled::setWidget(QWidget *widget)
{
    m_layout->removeWidget(m_widget);
    m_widget = widget;
    m_layout->addWidget(m_widget, 0, Qt::AlignTop);
    m_title->setBuddy(m_widget);
}

void QEntitled::setTitle(const QString &title)
{
    m_title->setText(title);
}

QImageButton::QImageButton(const QString &normal, const QString &hovered)
            : myPic(normal), myHoveredPic(hovered), lastUnderMouse(-1)
{
    setFixedSize(myPic.size());

    /* Both are necessary for some styles */
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
}

void QImageButton::changePics(const QString &normal, const QString &hovered)
{
    myPic = QPixmap(normal);
    myHoveredPic = QPixmap(hovered);
    update();
}

QSize QImageButton::sizeHint() const
{
    return myPic.size();
}

QSize QImageButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize QImageButton::maximumSize() const
{
    return sizeHint();
}

void QImageButton::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    if (!underMouse())
        painter.drawPixmap(e->rect(), myPic, e->rect());
    else
        painter.drawPixmap(e->rect(), myHoveredPic, e->rect());

    lastUnderMouse = underMouse();
}

void QImageButton::mouseMoveEvent(QMouseEvent *)
{
    if (int(underMouse()) == lastUnderMouse)
        return;
    update();
}

QImageBackground::QImageBackground(const QString &imagePath)
        : myBackground(imagePath)
{
    if (!myBackground.isNull()) {
        setFixedSize(myBackground.size());
    }
}

void QImageBackground::changePic(const QString &path)
{
    myBackground = QPixmap(path);
    update();
}

QSize QImageBackground::sizeHint() const
{
    return myBackground.size();
}

QSize QImageBackground::minimumSizeHint() const
{
    return sizeHint();
}

QSize QImageBackground::maximumSize() const
{
    return sizeHint();
}

void QImageBackground::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.drawPixmap(e->rect(), myBackground, e->rect());
}

QIdListWidgetItem::QIdListWidgetItem(int id, const QString &text)
	: QListWidgetItem(text), myid(id)
{
}

int QIdListWidgetItem::id() const
{
    return myid;
}

void QIdListWidgetItem::setColor(const QColor &c)
{
    setForeground(QBrush(c));
}

QScrollDownTextEdit::QScrollDownTextEdit()
{
    setReadOnly(true);
    linecount = 0;
}

void QScrollDownTextEdit::insertHtml(const QString &text)
{
    if (linecount >= 2000) {
        clear();
        linecount = 0;
    }

    moveCursor(QTextCursor::End);
    QTextEdit::insertHtml(text);
    QScrollBar * b = verticalScrollBar();
    if(b->isVisible()&&b->value()!= b->maximum())
    {
	b->setValue(b->maximum());
    }

    linecount++;
}

void QScrollDownTextEdit::insertPlainText(const QString &text)
{
    if (linecount >= 1000) {
        clear();
        linecount = 0;
    }

    moveCursor(QTextCursor::End);
    QTextEdit::insertPlainText(text);
    QScrollBar * b = verticalScrollBar();
    if(b->isVisible()&&b->value()!= b->maximum())
    {
	b->setValue(b->maximum());
    }

    linecount++;
}


QNickValidator::QNickValidator(QWidget *parent)
        : QValidator(parent)
{}

bool QNickValidator::isBegEndChar(QChar ch) const
{
    return ch.isLetterOrNumber() || ch.isPunct();
}

void QNickValidator::fixup(QString &input) const
{
    /* The only real case when you need to fix a string that's intermediate
       is to remove the trailing space at the end. */
    if (input.length() > 0 && input[input.length()-1] == ' ') {
        input.resize(input.length()-1);
    }
}

QValidator::State QNickValidator::validate(const QString &input) const
{
    if (input.length() > 20)
    {
        return QValidator::Invalid;
    }
    if (input.length() == 0)
        return QValidator::Intermediate;

    if (!isBegEndChar(input[0])) {
        return QValidator::Invalid;
    }

    bool spaced = false;
    bool punct = false;

    for (int i = 0; i < input.length(); i++) {
        if (input[i] == '%' || input[i] == '*' || input[i] == '<' || input[i] == ':' || input[i] == '(' || input[i] == ')')
            return QValidator::Invalid;
        if (input[i].isPunct()) {
            if (punct == true) {
                //Error: two punctuations are not separated by a letter/number
                return QValidator::Invalid;
            }
            punct = true;
            spaced = false;
        } else if (input[i] == ' ') {
            if (spaced == true) {
                //Error: two spaces are following
                return QValidator::Invalid;
            }
            spaced = true;
        } else if (input[i].isLetterOrNumber()) {
            //we allow another punct & space
            punct = false;
            spaced = false;
        }
    }

    //let's check if there is at least a letter/number & no whitespace at the end
    if (input.length() == 1 && input[0].isPunct()) {
        return QValidator::Intermediate;
    }
    if (!isBegEndChar(input[input.length()-1])) {
        return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}

QValidator::State QNickValidator::validate(QString &input, int &) const
{
    return validate(input);
}

QImageButtonLR::QImageButtonLR(const QString &normal, const QString &hovered)
    :QImageButton(normal,hovered)
{}

void QImageButtonLR::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
        emit leftClick();
    else if (ev->button() == Qt::RightButton)
        emit rightClick();
}
