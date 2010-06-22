#include "sql.h"
#include "sqlconfig.h"
#include "../Utilities/otherwidgets.h"

SQLConfigWindow::SQLConfigWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *v = new QVBoxLayout(this);

    QLabel *desc = new QLabel(tr("<b><span style='color:red'>Don't touch anything if you've no clue what SQL is!</span></b><br /><br />For any change to have effect, you need to restart the server."
                                 "<br />If you change the settings without knowledge of what you are doing, you'll probably end up without any users stored anymore."));
    desc->setWordWrap(true);
    v->addWidget(desc);

    QSettings s;

    b = new QComboBox();
    b->addItem("SQLite");
    b->addItem("PostGreSQL");
    v->addLayout(new QSideBySide(new QLabel(tr("SQL Database type: ")), b));
    if (s.value("sql_driver").toInt() == SQLCreator::PostGreSQL) {
        b->setCurrentIndex(1);
    } else if (s.value("sql_driver").toInt() == SQLCreator::MySQL) {
        b->setCurrentIndex(2);
    }

    name = new QLineEdit();
    name->setText(s.value("sql_db_name").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Database name: ")), name));

    user = new QLineEdit();
    user->setText(s.value("sql_db_user").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("User: ")), user));

    pass = new QLineEdit();
    pass->setText(s.value("sql_db_pass").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Password: ")), pass));

    host = new QLineEdit();
    host->setText(s.value("sql_db_host").toString());
    v->addLayout(new QSideBySide(new QLabel(tr("Host: ")), host));

    port = new QSpinBox();
    port->setRange(0, 65535);
    port->setValue(s.value("sql_db_port").toInt());
    v->addLayout(new QSideBySide(new QLabel(tr("Port: ")), port));

    QPushButton *apply = new QPushButton(tr("&Apply"));
    connect(apply, SIGNAL(clicked()), this, SLOT(apply()));
    v->addWidget(apply);

    connect(b, SIGNAL(activated(int)), SLOT(changeEnabled()));
    changeEnabled();
}

void SQLConfigWindow::changeEnabled()
{
    bool c = (b->currentIndex() == 0);

    user->setDisabled(c);
    port->setDisabled(c);
    host->setDisabled(c);
    pass->setDisabled(c);
}

void SQLConfigWindow::apply()
{
    QSettings s;

    s.setValue("sql_driver", b->currentIndex());
    s.setValue("sql_db_name", name->text());
    s.setValue("sql_db_port", port->value());
    s.setValue("sql_db_user", user->text());
    s.setValue("sql_db_pass", pass->text());
    s.setValue("sql_db_host", host->text());
}
