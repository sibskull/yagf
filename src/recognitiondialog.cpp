#include "recognitiondialog.h"
#include "ui_recognitiondialog.h"

RecognitionDialog::RecognitionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecognitionDialog)
{
    ui->setupUi(this);
    ui->widget_2->startAnimation();
    connect(ui->pushButton, SIGNAL(clicked()), this, SIGNAL(rejected()));
    connect (this, SIGNAL(rejected()), this, SIGNAL(cancel()), Qt::QueuedConnection);
}

RecognitionDialog::~RecognitionDialog()
{
    delete ui;
}

void RecognitionDialog::blockRecognized(int n)
{
    ui->label_2->setText(trUtf8("block # %1").arg(n));
}
