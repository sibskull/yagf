#include "langselectdialog.h"
#include "ui_langselectdialog.h"
#include "settings.h"
#include <QCheckBox>

const int max_lang = 48;

LangSelectDialog::LangSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LangSelectDialog)
{
    ui->setupUi(this);
    fillLangs();
    setRecognitionLanguages(Settings::instance()->getSelectedLanguages());
}

LangSelectDialog::~LangSelectDialog()
{
    delete ui;
}

QStringList LangSelectDialog::getRecognitionLanguages() const
{
    QStringList sl;
    QString wName = "checkBox";
    QCheckBox * cb = findChild<QCheckBox*>(wName);
    if (cb->isChecked())
        sl.append(cb->text());
    for (int i = 1; i < max_lang; i++) {
        wName = QString("checkBox_%1").arg(i+1);
        cb = findChild<QCheckBox*>(wName);
        if (cb->isChecked())
            sl.append(cb->text());
    }
    return sl;
}

void LangSelectDialog::setRecognitionLanguages(const QStringList &sl)
{
    QString wName = "checkBox";
    QCheckBox * cb = findChild<QCheckBox*>(wName);
    if (sl.contains(cb->text()))
            cb->setChecked(true);
    else
        cb->setChecked(false);
    for (int i = 1; i < max_lang; i++) {
        wName = QString("checkBox_%1").arg(i+1);
        cb = findChild<QCheckBox*>(wName);
        if (sl.contains(cb->text()))
            cb->setChecked(true);
        else
            cb->setChecked(false);
    }
}

void LangSelectDialog::accept()
{
    QStringList sl = getRecognitionLanguages();
    Settings::instance()->setSelectedLanguages(sl);
    QDialog::accept();
}

void LangSelectDialog::fillLangs()
{
    QStringList sl = Settings::instance()->fullLanguageNames();
    sl.sort();
    QString wName = "checkBox";
    QCheckBox * cb = findChild<QCheckBox*>(wName);
    cb->setText(sl[0]);
    for (int i = 1; i < sl.count(); i++) {
        wName = QString("checkBox_%1").arg(i+1);
        cb = findChild<QCheckBox*>(wName);
        cb->setText(sl[i]);
    }
    for (int i = sl.count(); i < max_lang; i++) {
        wName = QString("checkBox_%1").arg(i+1);
        cb = findChild<QCheckBox*>(wName);
        cb->hide();
    }
}
