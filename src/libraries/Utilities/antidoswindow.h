#ifndef ANTIDOSWINDOW_H
#define ANTIDOSWINDOW_H

#include <QWidget>

class QSpinBox;
class QLineEdit;
class QCheckBox;
class QSettings;

class AntiDosWindow : public QWidget
{
    Q_OBJECT
public:
    AntiDosWindow(QSettings &settings);
public slots:
    void apply();
private:
    QSpinBox *max_people_per_ip, *max_commands_per_user, *max_kb_per_user, *max_login_per_ip, *ban_after_x_kicks;
    QLineEdit *trusted_ips, *notificationsChannel;
    QCheckBox *aDosOn;

    QSettings &settings;
};


#endif // ANTIDOSWINDOW_H
