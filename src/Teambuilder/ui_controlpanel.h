/********************************************************************************
** Form generated from reading UI file 'controlpanel.ui'
**
** Created: Wed 16. Mar 15:23:07 2011
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROLPANEL_H
#define UI_CONTROLPANEL_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlPanel
{
public:
    QWidget *userInfo;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *userName;
    QPushButton *searchUser;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLabel *status;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QLabel *authority;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_6;
    QLabel *ip;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_7;
    QLabel *lastAp;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_3;
    QSpinBox *time;
    QPushButton *tBan;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *mute;
    QPushButton *kick;
    QPushButton *ban;
    QWidget *userAlias;
    QGridLayout *gridLayout_4;
    QListWidget *aliasList;
    QLineEdit *aliasName;
    QPushButton *searchAlias;
    QWidget *banList;
    QGridLayout *gridLayout;
    QTableWidget *banTable;
    QPushButton *refresh;
    QPushButton *unban;
    QPushButton *banIP;
    QWidget *tab;
    QGridLayout *gridLayout_2;
    QPushButton *tbanIP;
    QPushButton *tunban;
    QPushButton *trefresh;
    QTableWidget *tbanTable;

    void setupUi(QTabWidget *ControlPanel)
    {
        if (ControlPanel->objectName().isEmpty())
            ControlPanel->setObjectName(QString::fromUtf8("ControlPanel"));
        ControlPanel->resize(328, 335);
        ControlPanel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        ControlPanel->setTabShape(QTabWidget::Rounded);
        userInfo = new QWidget();
        userInfo->setObjectName(QString::fromUtf8("userInfo"));
        verticalLayout_2 = new QVBoxLayout(userInfo);
        verticalLayout_2->setContentsMargins(10, 10, 10, 10);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(userInfo);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        userName = new QLineEdit(userInfo);
        userName->setObjectName(QString::fromUtf8("userName"));

        horizontalLayout->addWidget(userName);

        searchUser = new QPushButton(userInfo);
        searchUser->setObjectName(QString::fromUtf8("searchUser"));

        horizontalLayout->addWidget(searchUser);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(userInfo);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        status = new QLabel(userInfo);
        status->setObjectName(QString::fromUtf8("status"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(status->sizePolicy().hasHeightForWidth());
        status->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(status);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_4 = new QLabel(userInfo);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(label_4);

        authority = new QLabel(userInfo);
        authority->setObjectName(QString::fromUtf8("authority"));
        sizePolicy.setHeightForWidth(authority->sizePolicy().hasHeightForWidth());
        authority->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(authority);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_6 = new QLabel(userInfo);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy1.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(label_6);

        ip = new QLabel(userInfo);
        ip->setObjectName(QString::fromUtf8("ip"));
        sizePolicy.setHeightForWidth(ip->sizePolicy().hasHeightForWidth());
        ip->setSizePolicy(sizePolicy);
        ip->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        horizontalLayout_4->addWidget(ip);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_7 = new QLabel(userInfo);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_7->addWidget(label_7);

        lastAp = new QLabel(userInfo);
        lastAp->setObjectName(QString::fromUtf8("lastAp"));
        sizePolicy.setHeightForWidth(lastAp->sizePolicy().hasHeightForWidth());
        lastAp->setSizePolicy(sizePolicy);

        horizontalLayout_7->addWidget(lastAp);


        verticalLayout_2->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_3 = new QLabel(userInfo);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy2);

        horizontalLayout_6->addWidget(label_3);

        time = new QSpinBox(userInfo);
        time->setObjectName(QString::fromUtf8("time"));
        time->setMinimum(1);
        time->setMaximum(1440);

        horizontalLayout_6->addWidget(time);

        tBan = new QPushButton(userInfo);
        tBan->setObjectName(QString::fromUtf8("tBan"));

        horizontalLayout_6->addWidget(tBan);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        mute = new QPushButton(userInfo);
        mute->setObjectName(QString::fromUtf8("mute"));

        horizontalLayout_5->addWidget(mute);

        kick = new QPushButton(userInfo);
        kick->setObjectName(QString::fromUtf8("kick"));

        horizontalLayout_5->addWidget(kick);

        ban = new QPushButton(userInfo);
        ban->setObjectName(QString::fromUtf8("ban"));

        horizontalLayout_5->addWidget(ban);


        verticalLayout_2->addLayout(horizontalLayout_5);

        ControlPanel->addTab(userInfo, QString());
        userAlias = new QWidget();
        userAlias->setObjectName(QString::fromUtf8("userAlias"));
        gridLayout_4 = new QGridLayout(userAlias);
        gridLayout_4->setContentsMargins(10, 10, 10, 10);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        aliasList = new QListWidget(userAlias);
        aliasList->setObjectName(QString::fromUtf8("aliasList"));
        aliasList->setMouseTracking(false);
        aliasList->setEditTriggers(QAbstractItemView::NoEditTriggers);

        gridLayout_4->addWidget(aliasList, 1, 0, 1, 2);

        aliasName = new QLineEdit(userAlias);
        aliasName->setObjectName(QString::fromUtf8("aliasName"));

        gridLayout_4->addWidget(aliasName, 0, 0, 1, 1);

        searchAlias = new QPushButton(userAlias);
        searchAlias->setObjectName(QString::fromUtf8("searchAlias"));

        gridLayout_4->addWidget(searchAlias, 0, 1, 1, 1);

        ControlPanel->addTab(userAlias, QString());
        banList = new QWidget();
        banList->setObjectName(QString::fromUtf8("banList"));
        gridLayout = new QGridLayout(banList);
        gridLayout->setContentsMargins(10, 10, 10, 10);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        banTable = new QTableWidget(banList);
        if (banTable->columnCount() < 2)
            banTable->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        banTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        banTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        banTable->setObjectName(QString::fromUtf8("banTable"));
        banTable->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        banTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        banTable->setAlternatingRowColors(true);
        banTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        banTable->setShowGrid(false);
        banTable->setGridStyle(Qt::SolidLine);
        banTable->setSortingEnabled(true);
        banTable->setCornerButtonEnabled(false);
        banTable->horizontalHeader()->setStretchLastSection(true);
        banTable->verticalHeader()->setVisible(false);
        banTable->verticalHeader()->setDefaultSectionSize(19);

        gridLayout->addWidget(banTable, 0, 0, 1, 3);

        refresh = new QPushButton(banList);
        refresh->setObjectName(QString::fromUtf8("refresh"));

        gridLayout->addWidget(refresh, 1, 0, 1, 1);

        unban = new QPushButton(banList);
        unban->setObjectName(QString::fromUtf8("unban"));

        gridLayout->addWidget(unban, 1, 1, 1, 1);

        banIP = new QPushButton(banList);
        banIP->setObjectName(QString::fromUtf8("banIP"));

        gridLayout->addWidget(banIP, 1, 2, 1, 1);

        ControlPanel->addTab(banList, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        tab->setCursor(QCursor(Qt::ArrowCursor));
        gridLayout_2 = new QGridLayout(tab);
        gridLayout_2->setContentsMargins(10, 10, 10, 10);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        tbanIP = new QPushButton(tab);
        tbanIP->setObjectName(QString::fromUtf8("tbanIP"));

        gridLayout_2->addWidget(tbanIP, 3, 2, 1, 1);

        tunban = new QPushButton(tab);
        tunban->setObjectName(QString::fromUtf8("tunban"));

        gridLayout_2->addWidget(tunban, 3, 1, 1, 1);

        trefresh = new QPushButton(tab);
        trefresh->setObjectName(QString::fromUtf8("trefresh"));

        gridLayout_2->addWidget(trefresh, 3, 0, 1, 1);

        tbanTable = new QTableWidget(tab);
        if (tbanTable->columnCount() < 3)
            tbanTable->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tbanTable->setHorizontalHeaderItem(0, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tbanTable->setHorizontalHeaderItem(1, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tbanTable->setHorizontalHeaderItem(2, __qtablewidgetitem4);
        tbanTable->setObjectName(QString::fromUtf8("tbanTable"));
        tbanTable->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        tbanTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tbanTable->setAlternatingRowColors(true);
        tbanTable->setSelectionMode(QAbstractItemView::SingleSelection);
        tbanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        tbanTable->setShowGrid(false);
        tbanTable->setGridStyle(Qt::SolidLine);
        tbanTable->setSortingEnabled(false);
        tbanTable->setCornerButtonEnabled(false);
        tbanTable->horizontalHeader()->setVisible(false);
        tbanTable->horizontalHeader()->setCascadingSectionResizes(true);
        tbanTable->horizontalHeader()->setDefaultSectionSize(100);
        tbanTable->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        tbanTable->horizontalHeader()->setStretchLastSection(true);
        tbanTable->verticalHeader()->setVisible(false);
        tbanTable->verticalHeader()->setCascadingSectionResizes(false);
        tbanTable->verticalHeader()->setHighlightSections(false);
        tbanTable->verticalHeader()->setProperty("showSortIndicator", QVariant(false));
        tbanTable->verticalHeader()->setStretchLastSection(false);

        gridLayout_2->addWidget(tbanTable, 0, 0, 1, 3);

        ControlPanel->addTab(tab, QString());

        retranslateUi(ControlPanel);

        ControlPanel->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ControlPanel);
    } // setupUi

    void retranslateUi(QTabWidget *ControlPanel)
    {
        ControlPanel->setWindowTitle(QApplication::translate("ControlPanel", "Control Panel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ControlPanel", "Username: ", 0, QApplication::UnicodeUTF8));
        searchUser->setText(QApplication::translate("ControlPanel", "Search", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ControlPanel", "Status: ", 0, QApplication::UnicodeUTF8));
        status->setText(QApplication::translate("ControlPanel", "Online", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ControlPanel", "Authority: ", 0, QApplication::UnicodeUTF8));
        authority->setText(QString());
        label_6->setText(QApplication::translate("ControlPanel", "IP Address: ", 0, QApplication::UnicodeUTF8));
        ip->setText(QString());
        label_7->setText(QApplication::translate("ControlPanel", "Last Appearance:", 0, QApplication::UnicodeUTF8));
        lastAp->setText(QString());
        label_3->setText(QApplication::translate("ControlPanel", "Temporary Ban:", 0, QApplication::UnicodeUTF8));
        time->setSuffix(QString());
        tBan->setText(QApplication::translate("ControlPanel", "Temp Ban", 0, QApplication::UnicodeUTF8));
        mute->setText(QApplication::translate("ControlPanel", "Mute", 0, QApplication::UnicodeUTF8));
        kick->setText(QApplication::translate("ControlPanel", "Kick", 0, QApplication::UnicodeUTF8));
        ban->setText(QApplication::translate("ControlPanel", "Ban", 0, QApplication::UnicodeUTF8));
        ControlPanel->setTabText(ControlPanel->indexOf(userInfo), QApplication::translate("ControlPanel", "User info", 0, QApplication::UnicodeUTF8));
        aliasName->setText(QString());
        searchAlias->setText(QApplication::translate("ControlPanel", "Search", 0, QApplication::UnicodeUTF8));
        ControlPanel->setTabText(ControlPanel->indexOf(userAlias), QApplication::translate("ControlPanel", "User alias", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = banTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("ControlPanel", "Username", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = banTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("ControlPanel", "Banned IP", 0, QApplication::UnicodeUTF8));
        refresh->setText(QApplication::translate("ControlPanel", "Refresh", 0, QApplication::UnicodeUTF8));
        unban->setText(QApplication::translate("ControlPanel", "Unban", 0, QApplication::UnicodeUTF8));
        banIP->setText(QApplication::translate("ControlPanel", "Ban IP ...", 0, QApplication::UnicodeUTF8));
        ControlPanel->setTabText(ControlPanel->indexOf(banList), QApplication::translate("ControlPanel", "Ban list", 0, QApplication::UnicodeUTF8));
        tbanIP->setText(QApplication::translate("ControlPanel", "Ban IP ...", 0, QApplication::UnicodeUTF8));
        tunban->setText(QApplication::translate("ControlPanel", "Unban", 0, QApplication::UnicodeUTF8));
        trefresh->setText(QApplication::translate("ControlPanel", "Refresh", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tbanTable->horizontalHeaderItem(0);
        ___qtablewidgetitem2->setText(QApplication::translate("ControlPanel", "Username", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tbanTable->horizontalHeaderItem(1);
        ___qtablewidgetitem3->setText(QApplication::translate("ControlPanel", "Banned IP", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tbanTable->horizontalHeaderItem(2);
        ___qtablewidgetitem4->setText(QApplication::translate("ControlPanel", "Time left", 0, QApplication::UnicodeUTF8));
        ControlPanel->setTabText(ControlPanel->indexOf(tab), QApplication::translate("ControlPanel", "Temporary bans", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ControlPanel: public Ui_ControlPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLPANEL_H
