#ifndef AVATARDIALOG_H
#define AVATARDIALOG_H

#include <QDialog>

class QPushButton;

namespace Ui {
class AvatarDialog;
}

class AvatarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AvatarDialog(QWidget *parent = 0, int current = 1);
    ~AvatarDialog();

private:
    Ui::AvatarDialog *ui;
    QPushButton *current;

signals:
    void selectionFinished(int avatar);

public slots:
    void avatarClicked();
private slots:
    void on_btn_ok_clicked();
};

#endif // AVATARDIALOG_H
