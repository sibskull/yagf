#ifndef RECOGNITIONDIALOG_H
#define RECOGNITIONDIALOG_H

#include <QDialog>

namespace Ui {
class RecognitionDialog;
}

class RecognitionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RecognitionDialog(QWidget *parent = 0);
    ~RecognitionDialog();
signals:
    void cancel();
public slots:
    void blockRecognized(int n);
private:
    Ui::RecognitionDialog *ui;
};

#endif // RECOGNITIONDIALOG_H
