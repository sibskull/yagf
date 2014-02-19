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

#include "qxttiffiohandler.h"
#include <tiff.h>
#include <tiffio.h>
#include <sys/types.h>
#include <unistd.h>
#include <QIODevice>
#include <QImage>
#include <QBuffer>

QXtTiffIOHandler::QXtTiffIOHandler() :
    QImageIOHandler()
{
    dirnum = 0;
    dircount = 0;
    tif = NULL;
}

QXtTiffIOHandler::~QXtTiffIOHandler()
{
    if (tif)
        TIFFClose(tif);
}

static tsize_t readProc(thandle_t handle, tdata_t data, tsize_t size)
{
    QIODevice * device = (QIODevice *) handle;
    quint32 res = device->read((char *) data, size);
    return res;
}

static tsize_t writeProc(thandle_t handle, tdata_t data, tsize_t size)
{
    QIODevice * device = (QIODevice *) handle;
    quint32 res = device->write((char *) data, size);
    return res;
}

static toff_t seekProc(thandle_t handle, toff_t offset, int whence)
{
    QIODevice * device = (QIODevice *) handle;
    quint64 pos = device->pos();
    switch(whence) {
    case SEEK_CUR:
        pos = pos + offset;
        break;
    case SEEK_SET :
        pos = offset;
        break;
    case SEEK_END :
        pos = device->size() + offset;
        break;
    default:
        break;
    }
    if (device->seek(pos))
        return pos;
    return device->pos();
}

static int closeProc(thandle_t handle)
{
    QIODevice * device = (QIODevice *) handle;
    device->close();
    return 0;
}

static toff_t sizeProc(thandle_t handle)
{
    QIODevice * device = (QIODevice *) handle;
    return device->size();
}

bool QXtTiffIOHandler::canRead() const
{
    if (device())
        if (device()->isReadable()) {
            TIFF * t = TIFFClientOpen("dummy", "r", device(), readProc, writeProc, seekProc, closeProc, sizeProc, NULL, NULL);
            if (t) {
                /*dircount = 0;
                do {
                    dircount++;
                } while (TIFFReadDirectory(t));*/
                device()->reset();
                return true;
            }
        }
    return false;
}

bool QXtTiffIOHandler::read(QImage *image)
{
    quint32 w;
    quint32 h;
    char * raster;
    tif = TIFFClientOpen("dummy", "r", device(), readProc, writeProc, seekProc, closeProc, sizeProc, NULL, NULL);
    TIFFSetDirectory(tif, dirnum);
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    raster = (char*) _TIFFmalloc(w * h * sizeof (uint32));
    QBuffer buff;
    if (raster != NULL) {
        if (TIFFReadRGBAImage(tif, w, h, (uint32 *)raster, 0)) {
            QImage tmp(w, h, QImage::Format_ARGB32);
            for (uint32 row = 0; row < h; row++) {
                //strncpy((char*)tmp.scanLine(h - row - 1), &raster[4*w*row], 4*w);
                quint32 * p = (quint32 *) tmp.scanLine(h - row - 1);
                for (quint32 i = 0; i < w; i++)
                    p[i] = ((quint32 *) raster)[w*row + i];
            }
            _TIFFfree(raster);
            tmp.save(&buff, "JPG");
        }
        device()->reset();
        //TIFFClose(tif);
        return image->loadFromData(buff.buffer(), "JPG");
    }
    return false;
}

QVariant QXtTiffIOHandler::option(QImageIOHandler::ImageOption option) const
{
    return QImageIOHandler::option(option);
}

int QXtTiffIOHandler::loopCount() const
{
    return 0;
}

int QXtTiffIOHandler::imageCount() //const
{
    //device()->open(QIODevice::ReadOnly);
    //if (canRead())
    {
        tif = TIFFClientOpen("dummy", "r", device(), readProc, writeProc, seekProc, closeProc, sizeProc, NULL, NULL);
        dircount = 0;
            do {
                dircount++;
            } while (TIFFReadDirectory(tif));
        device()->reset();
        //TIFFClose(tif);
        return dircount;
    }
    return dircount;
}

bool QXtTiffIOHandler::jumpToImage(int imageNumber)
{
    if (imageNumber < imageCount()) {
        dirnum = imageNumber;
        return true;
     }
    return false;
}

bool QXtTiffIOHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
    if (option == QImageIOHandler::Animation) return true;
    return QImageIOHandler::supportsOption(option);
}

void QXtTiffIOHandler::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    QImageIOHandler::setOption(option, value);
}
