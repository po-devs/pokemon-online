#include "tierwindow.h"
#include "tiermachine.h"

TierWindow::TierWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose,true);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(m_editWindow = new QPlainTextEdit(),0,0,1,2);
    QPushButton *ok;
    layout->addWidget(ok = new QPushButton(tr("&Done")),1,1);

    m_editWindow->setPlainText(TierMachine::obj()->toString());

    connect(ok, SIGNAL(clicked()), SLOT(done()));
    connect(ok, SIGNAL(clicked()), SLOT(close()));
}

void TierWindow::done()
{
    TierMachine::obj()->fromString(m_editWindow->toPlainText());
    TierMachine::obj()->save();

    emit tiersChanged();
}

