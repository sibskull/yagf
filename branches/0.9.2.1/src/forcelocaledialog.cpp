/*
    YAGF - cuneiform and tesseract OCR graphical front-end
    Copyright (C) 2009-2013 Andrei Borovsky <anb@symmetrica.net>

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

#include "forcelocaledialog.h"
#include "ui_forcelocaledialog.h"

ForceLocaleDialog::ForceLocaleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ForceLocaleDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Choose UI Language");
    ui->groupBox->setTitle("Choose Language");
    ui->radioButton->setText("English");
    ui->radioButton_2->setText("Russian");
    ui->radioButton_3->setText("Defined by locale");
    ui->label->setText("Restart the program for the changes to take effect");
    ui->buttonBox->buttons()[1]->setText("&Cancel");
    ui->buttonBox->buttons()[0]->setText("&OK");
}

ForceLocaleDialog::~ForceLocaleDialog()
{
    delete ui;
}

void ForceLocaleDialog::setOption(Locales option)
{
    switch(option) {
    case ForceLocaleDialog::NoLocale:
        ui->radioButton->setChecked(true);
        break;
    case ForceLocaleDialog::RussianLocale:
        ui->radioButton_2->setChecked(true);
        break;
    case ForceLocaleDialog::DefaultLocale:
        ui->radioButton_3->setChecked(true);
        break;
    default:
        break;
    }
}

ForceLocaleDialog::Locales ForceLocaleDialog::getOption()
{
    if (ui->radioButton->isChecked())
        return ForceLocaleDialog::NoLocale;
    if (ui->radioButton_2->isChecked())
        return ForceLocaleDialog::RussianLocale;
    return ForceLocaleDialog::DefaultLocale;
}
