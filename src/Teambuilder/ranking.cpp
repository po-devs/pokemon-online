#include "ranking.h"
#include "../Utilities/otherwidgets.h"
#include <QHeaderView>

RankingDialog::RankingDialog(const QStringList &tiers)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *row1 = new QHBoxLayout();
    mainLayout->addLayout(row1);

    QPushButton *search;
    row1->addWidget(name = new QLineEdit());
    row1->addWidget(search = new QPushButton(tr("&Search")));
    row1->addWidget(tierSelection = new QComboBox());
    tierSelection->addItems(tiers);

    players = new QCompactTable(0,3);
    players->setAlternatingRowColors(true);
    players->setHorizontalHeaderLabels(QStringList() << tr("Rank") << tr("Player Name") << tr("Points"));
    players->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(players);

    QHBoxLayout *row3 = new QHBoxLayout();
    mainLayout->addLayout(row3);

    QPushButton *prev, *next;
    row3->addWidget(prev = new QPushButton("<<"));
    row3->addWidget(next = new QPushButton(">>"));
    row3->addWidget(page = new QLineEdit(), 0, Qt::AlignRight);
    row3->addWidget(totalPages = new QLabel("/ ---"), 0, Qt::AlignLeft);

    resize(400,500);
    page->setMaximumWidth(40);
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(next, SIGNAL(clicked()), SLOT(nextPage()));
    connect(prev, SIGNAL(clicked()), SLOT(prevPage()));
    connect(tierSelection, SIGNAL(activated(int)), SLOT(searchByName()));
    connect(name, SIGNAL(returnPressed()), SLOT(searchByName()));
    connect(search, SIGNAL(clicked()), SLOT(searchByName()));
    connect(this->page, SIGNAL(returnPressed()), SLOT(changePage()));
}

void RankingDialog::startRanking(int page, int startRank, int total)
{
    this->page->setText(QString("%1").arg(page));
    totalPages->setText(QString("/ %1").arg(total));
    curRank = startRank;
    curPage = page;
    players->setRowCount(0);
}

void RankingDialog::showRank(const QString &name, int points)
{
    players->setRowCount(players->rowCount()+1);
    int r = players->rowCount() -1;

    players->setItem(r, 0, new QTableWidgetItem(QString("%1").arg(curRank)));
    players->setItem(r, 1, new QTableWidgetItem(name));
    players->setItem(r, 2, new QTableWidgetItem(QString("%1").arg(points)));

    if (name == this->name->text().toLower()) {
        QFont f = players->font();
        f.setBold(true);

        players->item(r, 0)->setFont(f);
        players->item(r, 1)->setFont(f);
        players->item(r, 2)->setFont(f);
    }

    curRank++;
}

void RankingDialog::init(const QString &name, const QString &tier)
{
    this->name->setText(name);
    for(int i = 0; i < tierSelection->count(); i++)
    {
        if (tierSelection->itemText(i) == tier) {
            tierSelection->setCurrentIndex(i);
            break;
        }
    }

    emit lookForPlayer(tier, name);
}

void RankingDialog::prevPage()
{
    if (curPage == 1)
        return;
    emit lookForPage(tierSelection->currentText(),curPage-1);
}

void RankingDialog::nextPage()
{
    emit lookForPage(tierSelection->currentText(),curPage+1);
}

void RankingDialog::searchByName()
{
    emit lookForPlayer(tierSelection->currentText(), name->text());
}

void RankingDialog::changePage()
{
    emit lookForPage(tierSelection->currentText(),page->text().toInt());
}
