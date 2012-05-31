#include "Teambuilder/teamimporter.h"
#include <QGridLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>

TeamImporter::TeamImporter(QWidget*parent) : QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QGridLayout *l = new QGridLayout(this);
    l->addWidget(new QLabel(tr("Paste your exported team from Netbattle Supremacy / "
                               "Shoddy Battle.\nYour language needs to be set to English to import English teams.")),0,0,1,2);
    l->addWidget(mycontent = new QPlainTextEdit(),1,0,1,2);
    mycontent->resize(mycontent->width(), 250);

    QPushButton *cancel, *_done;

    cancel = new QPushButton(tr("&Cancel"));
    _done = new QPushButton(tr("&Done"));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(_done, SIGNAL(clicked()), this, SLOT(close()));
    connect(_done, SIGNAL(clicked()), this, SLOT(done()));

    l->addWidget(cancel, 2,0);
    l->addWidget(_done,2,1);
}

void TeamImporter::done()
{
    emit done(mycontent->toPlainText());
}
