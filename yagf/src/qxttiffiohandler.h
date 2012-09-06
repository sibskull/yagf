/*
    YAGF - cuneiform and tesseract OCR graphical front-end
    Copyright (C) 2009-2012 Andrei Borovsky <anb@symmetrica.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

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
