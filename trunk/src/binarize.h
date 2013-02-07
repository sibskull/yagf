#ifndef BINARIZE_H
#define BINARIZE_H

#include <QObject>

class QImage;

class Binarize : public QObject
{
    Q_OBJECT
public:
    explicit Binarize(QObject *parent = 0);
    static void otsu(const QImage &input, QImage &output, int left = 0, int top = 0, int width = 0, int height = 0);
signals:
    
public slots:
    
};

#endif // BINARIZE_H
