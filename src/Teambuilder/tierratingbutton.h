#ifndef TIERRATINGBUTTON_H
#define TIERRATINGBUTTON_H

#include <QPushButton>

namespace Ui {
    class TierRatingButton;
}

class TierRatingButton : public QPushButton
{
    Q_OBJECT

public:
    TierRatingButton(const QString &tier, int rating);
    ~TierRatingButton();

    void setRating(int rating);
    void setTier(const QString &tier);
private:
    Ui::TierRatingButton *ui;
};

#endif // TIERRATINGBUTTON_H
