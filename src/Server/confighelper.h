#ifndef CONFIGHELPER_H
#define CONFIGHELPER_H

#include <QtCore>

/* This file allows to avoid writing bothersome code in order to make GUI config windows.

   Just generate a ConfigForm and add the corresponding ConfigHelper, and the variables
   will be modified internally automatically when the user plays with the GUI.
*/

class AbstractConfigHelper;

struct ConfigForm {
    QList<AbstractConfigHelper *> helpers;
};

class AbstractConfigHelper : public QObject
{
    Q_OBJECT
public:
    AbstractConfigHelper(const QString &desc = "");
    QWidget * generateConfigWidget();
public slots:
    void editingDone();
protected:
    QString description;
    QWidget* internalWidget;
private:
    virtual QWidget *getInternalWidget() = 0;
    virtual void doWhenEditingDone() = 0;
};

template <class T>
class ConfigHelper : public AbstractConfigHelper {
public:
    ConfigHelper(const QString &desc, T &var);
private:
    T &var;
};

/* Creates a combobox.
   You must also specify the values, of type T, that correspond
   to the different labels in the combobox */
template <class T>
class ConfigCombo : public ConfigHelper<T> {
public:
    ConfigCombo(const QString &desc, T &var, const QStringList &labels, const QList<T> &values);
private:
    QStringList labels;
    QList<T> values;

    virtual QWidget *getInternalWidget();
    virtual void doWhenEditingDone();
};

#endif // CONFIGHELPER_H
