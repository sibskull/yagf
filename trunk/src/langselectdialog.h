#ifndef LANGSELECTDIALOG_H
#define LANGSELECTDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class LangSelectDialog;
}

class LangSelectDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LangSelectDialog(QWidget *parent = 0);
    ~LangSelectDialog();
    QStringList getRecognitionLanguages() const;
    void setRecognitionLanguages(const QStringList &sl);
protected:
    void accept();
private:
    void fillLangs();
private:
    Ui::LangSelectDialog *ui;
};

#endif // LANGSELECTDIALOG_H
