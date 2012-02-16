#ifndef CHALLENGEDIALOG_H
#define CHALLENGEDIALOG_H

#include <QDialog>

namespace Ui {
    class ChallengeDIalog;
}

class ChallengeDIalog : public QDialog
{
    Q_OBJECT

public:
    explicit ChallengeDIalog(QWidget *parent = 0);
    ~ChallengeDIalog();

private:
    Ui::ChallengeDIalog *ui;
};

#endif // CHALLENGEDIALOG_H
