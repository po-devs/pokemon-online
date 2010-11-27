/********************************************************************************
** Form generated from reading UI file 'controlpanel.ui'
**
** Created: Fri Oct 15 17:37:17 2010
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
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
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
    QWidget *layoutWidget1;
    QGridLayout *gridLayout;
    QLineEdit *aliasName;
    QPushButton *searchAlias;
    QListWidget *aliasList;
    QWidget *banList;
    QWidget *layoutWidget2;
    QGridLayout *gridLayout_2;
    QPushButton *refresh;
    QPushButton *unban;
    QPushButton *banIP;
    QTableWidget *banTable;
    QWidget *tab;
    QWidget *layoutWidget_2;
    QGridLayout *gridLayout_3;
    QPushButton *trefresh;
    QPushButton *tunban;
    QPushButton *tbanIP;
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
        layoutWidget = new QWidget(userInfo);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 10, 301, 291));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        userName = new QLineEdit(layoutWidget);
        userName->setObjectName(QString::fromUtf8("userName"));

        horizontalLayout->addWidget(userName);

        searchUser = new QPushButton(layoutWidget);
        searchUser->setObjectName(QString::fromUtf8("searchUser"));

        horizontalLayout->addWidget(searchUser);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        status = new QLabel(layoutWidget);
        status->setObjectName(QString::fromUtf8("status"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(status->sizePolicy().hasHeightForWidth());
        status->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(status);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(label_4);

        authority = new QLabel(layoutWidget);
        authority->setObjectName(QString::fromUtf8("authority"));
        sizePolicy.setHeightForWidth(authority->sizePolicy().hasHeightForWidth());
        authority->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(authority);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy1.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(label_6);

        ip = new QLabel(layoutWidget);
        ip->setObjectName(QString::fromUtf8("ip"));
        sizePolicy.setHeightForWidth(ip->sizePolicy().hasHeightForWidth());
        ip->setSizePolicy(sizePolicy);

        horizontalLayout_4->addWidget(ip);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_7->addWidget(label_7);

        lastAp = new QLabel(layoutWidget);
        lastAp->setObjectName(QString::fromUtf8("lastAp"));
        sizePolicy.setHeightForWidth(lastAp->sizePolicy().hasHeightForWidth());
        lastAp->setSizePolicy(sizePolicy);

        horizontalLayout_7->addWidget(lastAp);


        verticalLayout->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy2);

        horizontalLayout_6->addWidget(label_3);

        time = new QSpinBox(layoutWidget);
        time->setObjectName(QString::fromUtf8("time"));
        time->setMinimum(1);
        time->setMaximum(1440);

        horizontalLayout_6->addWidget(time);

        tBan = new QPushButton(layoutWidget);
        tBan->setObjectName(QString::fromUtf8("tBan"));

        horizontalLayout_6->addWidget(tBan);


        verticalLayout->addLayout(horizontalLayout_6);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        mute = new QPushButton(layoutWidget);
        mute->setObjectName(QString::fromUtf8("mute"));

        horizontalLayout_5->addWidget(mute);

        kick = new QPushButton(layoutWidget);
        kick->setObjectName(QString::fromUtf8("kick"));

        horizontalLayout_5->addWidget(kick);

        ban = new QPushButton(layoutWidget);
        ban->setObjectName(QString::fromUtf8("ban"));

        horizontalLayout_5->addWidget(ban);


        verticalLayout->addLayout(horizontalLayout_5);

        ControlPanel->addTab(userInfo, QString());
        userAlias = new QWidget();
        userAlias->setObjectName(QString::fromUtf8("userAlias"));
        layoutWidget1 = new QWidget(userAlias);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 10, 301, 281));
        gridLayout = new QGridLayout(layoutWidget1);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        aliasName = new QLineEdit(layoutWidget1);
        aliasName->setObjectName(QString::fromUtf8("aliasName"));

        gridLayout->addWidget(aliasName, 0, 0, 1, 1);

        searchAlias = new QPushButton(layoutWidget1);
        searchAlias->setObjectName(QString::fromUtf8("searchAlias"));

        gridLayout->addWidget(searchAlias, 0, 1, 1, 1);

        aliasList = new QListWidget(layoutWidget1);
        aliasList->setObjectName(QString::fromUtf8("aliasList"));
        aliasList->setMouseTracking(false);
        aliasList->setEditTriggers(QAbstractItemView::NoEditTriggers);

        gridLayout->addWidget(aliasList, 1, 0, 1, 2);

        ControlPanel->addTab(userAlias, QString());
        banList = new QWidget();
        banList->setObjectName(QString::fromUtf8("banList"));
        layoutWidget2 = new QWidget(banList);
        layoutWidget2->setObjectName(QString::fromUtf8("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(10, 10, 301, 291));
        gridLayout_2 = new QGridLayout(layoutWidget2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        refresh = new QPushButton(layoutWidget2);
        refresh->setObjectName(QString::fromUtf8("refresh"));

        gridLayout_2->addWidget(refresh, 1, 0, 1, 1);

        unban = new QPushButton(layoutWidget2);
        unban->setObjectName(QString::fromUtf8("unban"));

        gridLayout_2->addWidget(unban, 1, 1, 1, 1);

        banIP = new QPushButton(layoutWidget2);
        banIP->setObjectName(QString::fromUtf8("banIP"));

        gridLayout_2->addWidget(banIP, 1, 2, 1, 1);

        banTable = new QTableWidget(layoutWidget2);
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

        gridLayout_2->addWidget(banTable, 0, 0, 1, 3);

        ControlPanel->addTab(banList, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        tab->setCursor(QCursor(Qt::ArrowCursor));
        layoutWidget_2 = new QWidget(tab);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(10, 10, 301, 291));
        gridLayout_3 = new QGridLayout(layoutWidget_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        trefresh = new QPushButton(layoutWidget_2);
        trefresh->setObjectName(QString::fromUtf8("trefresh"));

        gridLayout_3->addWidget(trefresh, 1, 0, 1, 1);

        tunban = new QPushButton(layoutWidget_2);
        tunban->setObjectName(QString::fromUtf8("tunban"));

        gridLayout_3->addWidget(tunban, 1, 1, 1, 1);

        tbanIP = new QPushButton(layoutWidget_2);
        tbanIP->setObjectName(QString::fromUtf8("tbanIP"));

        gridLayout_3->addWidget(tbanIP, 1, 2, 1, 1);

        tbanTable = new QTableWidget(layoutWidget_2);
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

        gridLayout_3->addWidget(tbanTable, 0, 0, 1, 3);

        ControlPanel->addTab(tab, QString());

        retranslateUi(ControlPanel);

        ControlPanel->setCurrentIndex(2);


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
        refresh->setText(QApplication::translate("ControlPanel", "Refresh", 0, QApplication::UnicodeUTF8));
        unban->setText(QApplication::translate("ControlPanel", "Unban", 0, QApplication::UnicodeUTF8));
        banIP->setText(QApplication::translate("ControlPanel", "Ban IP ...", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = banTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("ControlPanel", "Username", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = banTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("ControlPanel", "Banned IP", 0, QApplication::UnicodeUTF8));
        ControlPanel->setTabText(ControlPanel->indexOf(banList), QApplication::translate("ControlPanel", "Ban list", 0, QApplication::UnicodeUTF8));
        trefresh->setText(QApplication::translate("ControlPanel", "Refresh", 0, QApplication::UnicodeUTF8));
        tunban->setText(QApplication::translate("ControlPanel", "Unban", 0, QApplication::UnicodeUTF8));
        tbanIP->setText(QApplication::translate("ControlPanel", "Ban IP ...", 0, QApplication::UnicodeUTF8));
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
