#include "tierwindow.h"
#include "tiermachine.h"

TierWindow::TierWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose,true);

    QGridLayout *layout = new QGridLayout(this);

    dataTree = TierMachine::obj()->getDataTree();

    m_tree = new QTreeWidget();
    layout->addWidget(m_tree, 0, 0, 2, 1);
    m_tree->header()->hide();

    configWidget = new QWidget();
    layout->addWidget(configWidget, 0, 1, 1, 2);

    QPushButton *add, *finish;
    add = new QPushButton("Add New");
    QMenu *m = new QMenu(add);
    m->addAction("Tier");
    m->addAction("Category");
    add->setMenu(m);

    finish = new QPushButton("Finish and Apply");

    layout->addWidget(add,1,1);
    layout->addWidget(finish,1,2);

    connect(finish, SIGNAL(clicked()), SLOT(done()));
    connect(finish, SIGNAL(clicked()), SLOT(close()));
}

TierWindow::~TierWindow() {
    delete dataTree;
}

void TierWindow::done()
{
    TierMachine::obj()->fromString(dataTree->toXml());
    TierMachine::obj()->save();

    emit tiersChanged();
}

