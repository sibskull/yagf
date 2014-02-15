#ifndef BUSYFORM_H
#define BUSYFORM_H

#include <QSplashScreen>

namespace Ui {
class BusyForm;
}

class BusyForm : public QSplashScreen
{
    Q_OBJECT
    
public:
    explicit BusyForm(QWidget *parent = 0);
    ~BusyForm();
    
private:
    Ui::BusyForm *ui;
};

#endif // BUSYFORM_H
