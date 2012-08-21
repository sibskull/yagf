#ifndef IMAGEBOOSTER_H
#define IMAGEBOOSTER_H

#include <QObject>
#include <QImage>

class ImageBooster : public QObject
{
    Q_OBJECT
public:
    explicit ImageBooster(QObject *parent = 0);
    ~ImageBooster();
    void boost(QImage * image);
signals:

public slots:
private:
    void buildProfile(QImage *image);
    void sharpenEdges(quint32 * r, quint32 * g, quint32 * b, quint32 * br, int w);
private:
    quint32 * profile;
};

#endif // IMAGEBOOSTER_H
