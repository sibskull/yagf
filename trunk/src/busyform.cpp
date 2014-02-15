#include "busyform.h"
#include "ui_busyform.h"

BusyForm::BusyForm(QWidget *parent) :
    QSplashScreen(parent),
    ui(new Ui::BusyForm)
{
    ui->setupUi(this);
}

BusyForm::~BusyForm()
{
    delete ui;
}
