#include "pmsystem.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/functions.h"

// TO-DO: Begin able to drag a tab outside of it's tab widget to separate the PM. I didn't do it because i
// think it would break part of the new system.

PMSystem::PMSystem(bool withTabs)
{
    QVBoxLayout *myLayout = new QVBoxLayout(this);
    setWindowTitle(tr("Private Messages"));
    setMinimumSize(300, 300);
    myPMs = new QExposedTabWidget();
    myPMs->setTabsClosable(true);
    myPMs->setMovable(true);
    myLayout->addWidget(myPMs);
    setLayout(myLayout);

    tabbedPMs = withTabs;

    connect(myPMs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(myPMs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}

PMSystem::~PMSystem()
{
}

void PMSystem::startPM(PMStruct *newPM)
{
    if(tabbedPMs) {
        myPMs->addTab(newPM, newPM->name());
        connect(newPM, SIGNAL(messageReceived(PMStruct*)), this, SLOT(messageReceived(PMStruct*)));
        checkTabbing();
    } else {
        newPM->show();
    }
    myPMWindows.insert(newPM->id(), newPM);
}

void PMSystem::closeTab(int tabNum)
{
    PMStruct *widget = (PMStruct*) myPMs->widget(tabNum);
    widget->deleteLater();
    myPMs->removeTab(tabNum);
    myPMWindows.remove(widget->id());
    checkTabbing();
}

void PMSystem::checkTabbing()
{
    if(myPMs->count() <= 0) {
        close();
    } else {
        if(myPMs->count() >= 1) {
            show();
        }
    }
}

void PMSystem::tabChanged(int tabNum)
{
    // We're avoiding crash if we don't have any tab on the myPMs.
    if(myPMs->count() <= 0) {
        return;
    }
    PMStruct *widget = (PMStruct*) myPMs->widget(tabNum);
    widget->state = PMStruct::NoMessages;
    myPMs->tabBar()->setTabTextColor(tabNum, myPMs->tabBar()->palette().text().color());
}

void PMSystem::togglePMs(bool toggled)
{
    tabbedPMs = toggled;
    if(tabbedPMs) {
        foreach(PMStruct *pm, myPMWindows) {
            // Otherwise it wont be added to the myPMs :(
            pm->setWindowFlags(Qt::Widget);
            myPMs->addTab(pm, pm->name());
            show();
        }
    } else {
        foreach(PMStruct *pm, myPMWindows) {
            while(myPMs->count() > 0) {
                myPMs->removeTab(0);
            }
            // Otherwise new window wouldn't show :(
            pm->setWindowFlags(Qt::Window);
            pm->show();
            hide();
        }
    }
}

void PMSystem::messageReceived(PMStruct *pm) {
    if(isVisible()) {
        if(pm->state == PMStruct::NewMessage) {
            return;
        }
        if(pm == myPMs->currentWidget()) {
            return;
        }
        pm->state = PMStruct::NewMessage;
        for(int i = 0; i < myPMs->count(); i++) {
            if(myPMs->widget(i) == pm) {
                myPMs->tabBar()->setTabTextColor(i, QColor(Qt::red));
            }
        }
    } else {
        show();
        for(int i = 0; i < myPMs->count(); i++) {
            if(myPMs->widget(i) == pm) {
                myPMs->setCurrentIndex(i);
            }
        }
    }

}

PMStruct::PMStruct(int id, const QString &ownName, const QString &name, const QString &content, bool html)
    : m_ownName(ownName), escape_html(!html)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    this->id() = id;
    changeName(name);

    QGridLayout *l = new QGridLayout(this);
    this->setLayout(l);

    m_mainwindow = new QScrollDownTextBrowser();
    m_textToSend = new QIRCLineEdit();

    l->addWidget(m_mainwindow, 0,0,1,2);
    l->addWidget(m_textToSend, 1,0,1,2);

    m_challenge = new QPushButton(tr("&Challenge"));
    m_send = new QPushButton(tr("&Ignore"));
    m_send->setCheckable(true);

    log = LogManager::obj()->createLog(PMLog, name + " -- " + ownName + " ");
    QSettings s;
    if(s.value("pms_logged").toBool()) {
        log->override = Log::OverrideYes;
    }

    l->addWidget(m_challenge,2,0);
    l->addWidget(m_send,2,1);

    printLine(content, false);

    connect(m_textToSend, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(m_send, SIGNAL(toggled(bool)), this, SLOT(ignore(bool)));
    connect(m_challenge, SIGNAL(clicked()), this, SLOT(challenge()));
}

void PMStruct::changeName(const QString &newname)
{
    this->m_name = newname;
    setWindowTitle(newname);
}

void PMStruct::changeSelf(const QString &newname)
{
    this->m_ownName = newname;
}

void PMStruct::printLine(QString line, bool self)
{
    if (line.trimmed().length() == 0)
        return;

    QSettings s;
    bool tt = s.value("show_timestamps2").toBool();
    QString timeStr = "";

    if (tt)
        timeStr += "(" + QTime::currentTime().toString("hh:mm") + ") ";

    if (escape_html) {
        line = escapeHtml(line);
    }

    if (self) {
        printHtml(toColor(timeStr + "<b>" + escapeHtml(m_ownName) + ": </b>", Qt::darkBlue) + line, false);
    } else {
        printHtml(toColor(timeStr + "<b>" + escapeHtml(name()) + ": </b>", Qt::darkGray) + line, false);
//        if (!QApplication::activeWindow()) {
//            QApplication::alert(this, 10000);
//            //raise();
//        }
    }
}

void PMStruct::printHtml(const QString &htmlCode, bool timestamps)
{
    QSettings s;
    bool tt = s.value("show_timestamps2").toBool();
    QString timeStr = "";

    if (tt && timestamps)
        timeStr += "(" + QTime::currentTime().toString("hh:mm") + ") ";

    m_mainwindow->insertHtml(timeStr + removeTrollCharacters(htmlCode) + "<br />");
    log->pushHtml(timeStr + removeTrollCharacters(htmlCode) + "<br />");
    emit messageReceived(this);
}

void PMStruct::sendMessage()
{
    QString str = m_textToSend->text().trimmed();
    m_textToSend->clear();

    if (str.length() == 0) {
        return;
    }

    if (str.length() > 0) {
        QStringList s = str.split('\n');
        foreach(QString s1, s) {
            if (s1.length() > 0) {
                emit messageEntered(id(), s1);
                printLine(s1, true);
            }
        }
    }
}

void PMStruct::ignore(bool yes)
{
    emit ignore(id(), yes);
}

void PMStruct::challenge()
{
    emit challengeSent(id());
}

void PMStruct::disable()
{
    printHtml("<i>" + tr("The other party left the server, so the window was disabled.") + "</i>");
    m_challenge->setDisabled(true);
    m_send->setDisabled(true);
    m_textToSend->setDisabled(true);
}
void PMStruct::reuse(int id)
{
    if (this->id() == id) return;

    this->id() = id;
    printHtml("<i>" + tr("The player has logged on again") + "</i>");
    m_challenge->setEnabled(true);
    m_send->setEnabled(true);
    m_textToSend->setEnabled(true);
}

void PMStruct::closeEvent(QCloseEvent *event) {
    log->close();
    event->accept();
}
