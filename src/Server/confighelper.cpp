#include "confighelper.h"
#include "../Utilities/otherwidgets.h"

/* I hate stupid GCC that forces to specify the base class
   of attributes when templates are involved, even though
   it can check if there's a problem or not and shut up
   when there's not.

   What the hell is undeclared when templates are compile-time?
   Do they know what lazy evaluation is?

   You can always bypass that using -fpermissive, but it's going to
   be deprecated -.-

   Visual C++ > GCC on this point */

ConfigForm::ConfigForm(const QString &button1, const QString &button2)
    :button1S(button1), button2S(button2)
{

}

void ConfigForm::addConfigHelper(AbstractConfigHelper *helper)
{
    helpers.push_back(helper);
}

QWidget* ConfigForm::generateConfigWidget()
{
    QWidget *ret = new QWidget();

    QVBoxLayout *v = new QVBoxLayout(ret);

    foreach(AbstractConfigHelper *helper, helpers) {
        v->addWidget(helper->generateConfigWidget());
    }

    if (button1S.isEmpty() && button2S.isEmpty())
        return ret;
    if (button1S.isEmpty()) {
        QPushButton *p = new QPushButton(button2S);
        v->addWidget(p);

        connect(p, SIGNAL(clicked()), SIGNAL(button2()));
        return ret;
    }
    if (button2S.isEmpty()) {
        QPushButton *p = new QPushButton(button1S);
        v->addWidget(p);

        connect(p, SIGNAL(clicked()), SIGNAL(button1()));
        return ret;
    }
    QPushButton *p1 = new QPushButton(button1S);
    QPushButton *p2 = new QPushButton(button2S);

    connect(p1, SIGNAL(clicked()), SIGNAL(button1()));
    connect(p2, SIGNAL(clicked()), SIGNAL(button2()));

    v->addLayout(new QSideBySide(p1, p2));

    return ret;
}

void ConfigForm::applyVals()
{
    foreach(AbstractConfigHelper *helper, helpers) {
        helper->updateVal();
    }
}

AbstractConfigHelper::AbstractConfigHelper(const QString &desc) : description(desc) {

}

QWidget *AbstractConfigHelper::generateConfigWidget() {
    internalWidget = getInternalWidget();
    if (description.isEmpty()) {
        return internalWidget;
    } else {
        QWidget *w = new QWidget();
        w->setLayout(new QSideBySide(new QLabel(description), internalWidget));
        return w;
    }
}

template<class T>
ConfigHelper<T>::ConfigHelper(const QString &desc, T &var) : AbstractConfigHelper(desc), var(var) {

}


template<class T>
ConfigCombo<T>::ConfigCombo(const QString &desc, T &var, const QStringList &labels, const QList<T> &values)
    : ConfigHelper<T>(desc, var), labels(labels), values(values)
{

}

template<class T>
QWidget *ConfigCombo<T>::getInternalWidget()
{
    QComboBox *ret = new QComboBox();

    ret->addItems(labels);

    for (int i = 0; i < values.size(); i++) {
        if (values[i] == ConfigHelper<T>::var) {
            ret->setCurrentIndex(i);
            break;
        }
    }

    QObject::connect(ret, SIGNAL(activated(int)), SLOT(editingDone()));

    return ret;
}

template<class T>
void ConfigCombo<T>::updateVal()
{
    int index = ((QComboBox*)(ConfigHelper<T>::internalWidget))->currentIndex();

    if (index >= 0 && index < values.size()) {
        ConfigHelper<T>::var = values[index];
    }
}

ConfigSpin::ConfigSpin(const QString &desc, int &var, int min, int max)
    : ConfigHelper<int> (desc, var), min(min), max(max)
{

}

QWidget * ConfigSpin::getInternalWidget()
{
    QSpinBox *ret = new QSpinBox();

    ret->setRange(min, max);
    ret->setValue(var);

    return ret;
}

void ConfigSpin::updateVal()
{
    var = ((QSpinBox*)(internalWidget))->value();
}

ConfigLine::ConfigLine(const QString &desc, QString &var)
    : ConfigHelper<QString> (desc, var)
{

}

void ConfigLine::updateVal()
{
    var = ((QLineEdit*)(internalWidget))->text();
}

QWidget *ConfigLine::getInternalWidget()
{
    QLineEdit *ret = new QLineEdit();
    ret->setText(var);

    return ret;
}

ConfigCheck::ConfigCheck(const QString &desc, bool &var)
    : ConfigHelper<bool> ("", var), checkBoxText(desc)
{

}

void ConfigCheck::updateVal()
{
    var = ((QCheckBox*)(internalWidget))->isChecked();
}

QWidget *ConfigCheck::getInternalWidget()
{
    QCheckBox *ret = new QCheckBox(checkBoxText);
    ret->setChecked(var);

    return ret;
}
