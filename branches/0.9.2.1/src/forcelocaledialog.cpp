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
