#ifndef FORCELOCALEDIALOG_H
#define FORCELOCALEDIALOG_H

#include <QDialog>

namespace Ui {
class ForceLocaleDialog;
}

class ForceLocaleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ForceLocaleDialog(QWidget *parent = 0);
    ~ForceLocaleDialog();
    enum Locales {
        NoLocale,
        RussianLocale,
        DefaultLocale
    } ;
    void setOption(Locales option);
    Locales getOption();
private:
    Ui::ForceLocaleDialog *ui;
};

#endif // FORCELOCALEDIALOG_H
