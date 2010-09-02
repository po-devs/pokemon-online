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

#if defined(WIN32) || defined(WIN64)
/* On linux the function p.mask() screws up,
   so a new one is made. On windows the standard one is fine */
static QBitmap mask(const QPixmap &p)
{
#if !defined(WIN32) && !defined(WIN64)
    /*
     *  This code is for when borders aren't alpha'd properly (all the move types battle buttons needs to have their border alpha'd)
     *  Of course the problem only occurs on linux
     */

    QImage image = p.toImage();

    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            image.setPixel(i,j, qAlpha(image.pixel(i,j)) <= 0x0f ? 0x0 : 0xffffffff);
        }
    }

    return QBitmap::fromData(p.size(), image.bits());

    /*
      Use this code when you've removed any non-alpha border from all images called by qimagebutton (including battle buttons, challenge window and menu, and pokedex arrows)
      return QBitmap::fromData(p.size(), p.toImage().bits());
    */
#else
    return p.mask();
#endif
}
#endif

QImageButton::QImageButton(const QString &normal, const QString &hovered, const QString &checked)
            : myPic(normal), myHoveredPic(hovered), lastUnderMouse(-1), pressed(false)
{
    setFixedSize(myPic.size());
#if defined(WIN32) || defined(WIN64)
    setMask(::mask(myPic));
#endif
    lastState = Normal;

    /* Both are necessary for some styles */
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    if (checked != "")
        myCheckedPic = QPixmap(checked);
}

void QImageButton::changePics(const QString &normal, const QString &hovered, const QString &checked)
{
    myPic = QPixmap(normal);
    myHoveredPic = QPixmap(hovered);
    if (checked != "")
        myCheckedPic = QPixmap(checked);

#if defined(WIN32) || defined(WIN64)
    setMask(lastState == Checked ? ::mask(myCheckedPic) : (lastState == Normal ? ::mask(myPic) : ::mask(myHoveredPic)));
#endif

    update();
}

void QImageButton::mousePressEvent(QMouseEvent *e)
{
    pressed = true;
    QAbstractButton::mousePressEvent(e);
}

void QImageButton::mouseReleaseEvent(QMouseEvent *e)
{
    pressed = false;
    QAbstractButton::mouseReleaseEvent(e);
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

    int newState;
    if ((this->isChecked() || pressed) && !myCheckedPic.isNull()) {
        newState = Checked;
        painter.drawPixmap(e->rect(), myCheckedPic, e->rect());
    } else {
        if (!underMouse()) {
            newState = Normal;
            painter.drawPixmap(e->rect(), myPic, e->rect());
        }
        else {
            newState = Hovered;
            painter.drawPixmap(e->rect(), myHoveredPic, e->rect());
        }
    }

    if (newState != lastState) {
        lastState = newState;
#if defined(WIN32) || defined(WIN64)
        setMask(lastState == Checked ? ::mask(myCheckedPic) : (lastState == Normal ? ::mask(myPic) : ::mask(myHoveredPic)));
#endif
    }

    lastUnderMouse = underMouse();
}

void QImageButton::mouseMoveEvent(QMouseEvent *)
{
    if (int(underMouse()) == lastUnderMouse)
        return;
    update();
}

QIdTreeWidgetItem::QIdTreeWidgetItem(int id, const QStringList &text)
        : QTreeWidgetItem(0), myid(id)
{
    for (int i = 0; i < text.size(); i++)
        setText(i,text[i]);
}

int QIdTreeWidgetItem::id() const
{
    return myid;
}

void QIdTreeWidgetItem::setColor(const QColor &c)
{
    setForeground(0,QBrush(c));
}

QIdListWidgetItem::QIdListWidgetItem(int id, const QString &text)
        : QListWidgetItem(text), myid(id)
{
}

QIdListWidgetItem::QIdListWidgetItem(int id, const QIcon &icon, const QString &text)
        : QListWidgetItem(icon, text), myid(id)
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

QScrollDownTextBrowser::QScrollDownTextBrowser()
{
    autoClear = true;
    setReadOnly(true);
    setOpenExternalLinks(true);
    linecount = 0;
}

void QScrollDownTextBrowser::insertHtml(const QString &text)
{
    QScrollBar * b = verticalScrollBar();
    if (linecount >= 2000 && autoClear) {
        keepLines(1000);
        moveCursor(QTextCursor::End);
        linecount = 1000;
        QTextBrowser::insertHtml(text);
        b->setValue(b->maximum());
        return;
    }

    int f = b->value();
    int e = b->maximum();
    moveCursor(QTextCursor::End);
    QTextBrowser::insertHtml(text);

    if(b->value() != e)
    {
        b->setValue(f);
    }
    else
    {
        b->setValue(b->maximum());
        moveCursor(QTextCursor::End);
    }
    linecount++;
}

void QScrollDownTextBrowser::keepLines(int numberOfLines)
{
    setReadOnly(false);
    moveCursor(QTextCursor::Start);
    moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);

    while (numberOfLines > 0) {
        --numberOfLines;
        moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
    }

    textCursor().removeSelectedText();
    setReadOnly(true);
}

void QScrollDownTextBrowser::insertPlainText(const QString &text)
{
    QScrollBar * b = verticalScrollBar();
    if (linecount >= 2000 && autoClear) {
        keepLines(1000);
        moveCursor(QTextCursor::End);
        linecount = 1000;
        QTextBrowser::insertPlainText(text);
        b->setValue(b->maximum());
        return;
    }

    int f = b->value();
    int e = b->maximum();

    moveCursor(QTextCursor::End);
    QTextBrowser::insertPlainText(text);

    if(b->value() != e)
    {
        b->setValue(f);
    }
    else
    {
        b->setValue(b->maximum());
        moveCursor(QTextCursor::End);
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
        if (input[i] == '\n' || input[i] == '%' || input[i] == '*' || input[i] == '<' || input[i] == ':' || input[i] == '(' || input[i] == ')')
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

        if (input[i].category() >= QChar::Other_Control && input[i].category() <= QChar::Other_NotAssigned) {
            return QValidator::Intermediate;
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

void QClickPBar::mousePressEvent(QMouseEvent *)
{
    emit clicked();
}

QDummyGrabber::QDummyGrabber()
{
    setFixedSize(0,0);
}

QSideBySide::QSideBySide(QWidget *a, QWidget *b)
{
    addWidget(a);
    addWidget(b);
}

/////////////////////////////////////
/*new button with pressed pic*/
QImageButtonP::QImageButtonP(const QString &normal, const QString &hovered, const QString &pressed, const QString &checked)
            : myPic(normal), myHoveredPic(hovered), myPressedPic(pressed), lastUnderMouse(-1), bpressed(false)
{
    setFixedSize(myPic.size());
#if defined(WIN32) || defined(WIN64)
    setMask(::mask(myPic));
#endif
    lastState = Normal;

    /* Both are necessary for some styles */
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    if (checked != "")
        myCheckedPic = QPixmap(checked);
}

void QImageButtonP::changePics(const QString &normal, const QString &hovered,const QString &pressed, const QString &checked)
{
    myPic = QPixmap(normal);
    myHoveredPic = QPixmap(hovered);
    myPressedPic = QPixmap(pressed);
    if (checked != "")
        myCheckedPic = QPixmap(checked);

#if defined(WIN32) || defined(WIN64)
    setMask(lastState == Checked ? ::mask(myCheckedPic) : (lastState == Normal ? ::mask(myPic) : (lastState == Pressed ? ::mask(myPressedPic) : ::mask(myHoveredPic))));
#endif

    update();
}

void QImageButtonP::mousePressEvent(QMouseEvent *e)
{
    bpressed = true;
    QAbstractButton::mousePressEvent(e);
}

void QImageButtonP::mouseReleaseEvent(QMouseEvent *e)
{
    bpressed = false;
    QAbstractButton::mouseReleaseEvent(e);
}

QSize QImageButtonP::sizeHint() const
{
    return myPic.size();
}

QSize QImageButtonP::minimumSizeHint() const
{
    return sizeHint();
}

QSize QImageButtonP::maximumSize() const
{
    return sizeHint();
}

void QImageButtonP::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    int newState;
    if ((this->isChecked()) && !myCheckedPic.isNull()) {
        newState = Checked;
        painter.drawPixmap(e->rect(), myCheckedPic, e->rect());
    }else if(this->isDown () && !myPressedPic.isNull()) {
        newState = Pressed;
        painter.drawPixmap(e->rect(), myPressedPic, e->rect());
    }else {
        if (!underMouse()) {
            newState = Normal;
            painter.drawPixmap(e->rect(), myPic, e->rect());
        }
        else {
            newState = Hovered;
            painter.drawPixmap(e->rect(), myHoveredPic, e->rect());
        }
    }

    if (newState != lastState) {
        lastState = newState;
#if defined(WIN32) || defined(WIN64)
        setMask(lastState == Checked ? ::mask(myCheckedPic) : (lastState == Normal ? ::mask(myPic) : (lastState == Pressed ? ::mask(myPressedPic) : ::mask(myHoveredPic))));
#endif
    }

    lastUnderMouse = underMouse();
}

void QImageButtonP::mouseMoveEvent(QMouseEvent *)
{
    if (int(underMouse()) == lastUnderMouse)
        return;
    update();
}

///////////////////////////////////
/*new LineEdit like IRC*/

QIRCLineEdit::QIRCLineEdit()
{
    connect(this,SIGNAL(textEdited(QString)),this,SLOT(myTextEdited()));
    listindex=0;
}

void QIRCLineEdit::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Up) {
        if(text()==""){
            if(m_Inputlist.empty()){

            }else{
                listindex--;
                setText(m_Inputlist[listindex]);
            }
        }else if(text()!=""){
            //m_Inputlist2.append(text());
            if(listindex<=m_Inputlist.count()){
                //m_Inputlist.append(text());
                if(listindex==0){
                    setText(m_Inputlist[0]);
                }else{
                    if(listindex==m_Inputlist.count()) m_Inputlist.append(text());
                    listindex--;
                    setText(m_Inputlist[listindex]);
                }
            }else if(listindex>m_Inputlist.count()){
                if(!m_Inputlist.empty()){
                    m_Inputlist.append(text());
                    setText(m_Inputlist[listindex--]);
                }
            }
        }
    }else if(e->key() == Qt::Key_Down) {
        if(text()==""){
            if(m_Inputlist.empty()){

            }else{
                listindex++;
                setText(m_Inputlist[listindex]);
            }
        }else if(text()!=""){
            if(listindex+1<m_Inputlist.count()){
                listindex++;
                setText(m_Inputlist[listindex]);
            }else if(listindex>=m_Inputlist.count()){

            }

        }
    }else{
        QLineEdit::keyPressEvent(e);
    }

}
void QIRCLineEdit::myTextEdited()
{
    if(m_Inputlist.count()==0){
        listindex=1;
    }else{
        listindex=m_Inputlist.count();
    }
}

void QIRCLineEdit::myclear()
{
    if(text()!=""){
        m_Inputlist.append(text());
        if(m_Inputlist.count()==0){
            listindex=1;
        }else{
            listindex=m_Inputlist.count();
        }
    }
    QLineEdit::clear();
}
