#ifndef PDFTHREAD_H
#define PDFTHREAD_H

#include <QThread>
#include <QStringList>

class PDFExtractor;
class PDFThread : public QThread
{
    Q_OBJECT
public:
    explicit PDFThread(PDFExtractor *parent = 0);
    void run();
    void setProcess(const QString &cmd, const QStringList &args);

public slots:
    void politeStop();
private:
    bool done;
    QString command;
    QStringList arguments;
    PDFExtractor * mparent;

};

#endif // PDFTHREAD_H
