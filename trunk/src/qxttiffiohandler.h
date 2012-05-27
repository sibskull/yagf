#ifndef QXTTIFFIOHANDLER_H
#define QXTTIFFIOHANDLER_H

#include <QImageIOHandler>
#include <QVariant>
#include <QSize>
#include <QList>
#include "tiffio.h"


class QXtTiffIOHandler : public QImageIOHandler
{
 public:
    explicit QXtTiffIOHandler();
    ~QXtTiffIOHandler();
    bool canRead() const;
    bool read(QImage *image);
    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;
    int	imageCount (); //const;
    bool jumpToImage( int imageNumber );
    int loopCount () const;
private:
    quint16 dirnum;
    quint16 dircount;
    TIFF * tif;
};

#endif // QXTTIFFIOHANDLER_H
