#ifndef RANKING_H
#define RANKING_H

#include <QtGui>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

class QCompactTable;

class RankingDialog : public QWidget
{
    Q_OBJECT
public:
    RankingDialog(const QStringList &tiers);
    void init(const QString &name, const QString &tier);
signals:
    void lookForPlayer(const QString &tier, const QString &name);
    void lookForPage(const QString &tier, int page);
public slots:
    void startRanking(int page, int startRank, int total);
    void showRank(const QString &name, int points);
private slots:
    void nextPage();
    void prevPage();
    void searchByName();
    void changePage();
private:
    int curRank;
    int curPage;
    int totalPage;

    QLineEdit *name, *page;
    QLabel *totalPages;
    QCompactTable *players;
    QComboBox *tierSelection;
};

#endif // RANKING_H
