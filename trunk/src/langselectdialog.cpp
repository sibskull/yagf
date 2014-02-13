/*
    YAGF - cuneiform and tesseract OCR graphical front-end
    Copyright (C) 2009-2014 Andrei Borovsky <anb@symmetrica.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
