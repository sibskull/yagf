/*
    YAGF - cuneiform and tesseract OCR graphical front-ends
    Copyright (C) 2009-2014 Andrei Borovsky <anb@symmetrica.net>

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

#include "qipblackandwhiteimage.h"
#include "common.h"
#include "qipgrayscaleimage.h"

static const QRgb black = qRgb(0,0,0);
static const QRgb white = qRgb(255,255,255);



QIPBlackAndWhiteImage::QIPBlackAndWhiteImage() : data(0)
{
    w =0;
    h = 0;
}

QIPBlackAndWhiteImage::QIPBlackAndWhiteImage(quint32 width, quint32 height) : data(new quint8[width*height], deallocator<quint8>)
{
    w = width;
    h = height;
//     quint8 * ptr = new quint8[w*h];
//     data = QSharedPointer<quint8>(ptr);
}

QIPBlackAndWhiteImage::QIPBlackAndWhiteImage(const QIPBlackAndWhiteImage &I) : data(I.data.data(),deallocator<quint8>)
{
    w = I.w;
    h = I.h;
}

quint8 *QIPBlackAndWhiteImage::scanLine(quint32 y) const
{
    return &(data.data()[y*w]);
}

quint8 QIPBlackAndWhiteImage::pixel(quint32 x, quint32 y)
{
    return data.data()[x + y*w];
}

void QIPBlackAndWhiteImage::setPixel(quint32 x, quint32 y, quint8 value)
{
    data.data()[x+y*w] = value;
}

bool QIPBlackAndWhiteImage::compareElements(quint8 **se, quint8 **w, int dimensions) const
{
    quint8 ise[dimensions][dimensions];
    for (int i = 0; i < dimensions; i++)
        for (int j = 0; j < dimensions; j++) // for (int j = 0; j < dimensions-2; j++)
            if (se[i][j] == 0)
                if (w[i][j] != 0)
                    return false;
    return true;
}

IntRect QIPBlackAndWhiteImage::cropInternal(bool upperLeft) const
{
    IntRect result;
    if (upperLeft) {
        for (int y = 0; y < h; y++) {
            quint32 stepCount = 0;
            quint8 * line = scanLine(y);
            for (int x = 1; x < w; x++) {
                if (line[x] != line[x-1])
                    stepCount++;
            }
            if (stepCount > 4) {
                result.y1 = y > 2 ? y - 2 : y;
                result.y1 = y > 4 ? y - 4 : result.y1;
                break;
            }
        }
        for (int x = 0; x < w; x++) {
            quint32  bin[2] = {0,0};
            quint32 min;
            for (int y = 0; y < h; y++) {
                quint8 * line = scanLine(y);
                bin[line[x]]++;
            }
            min = bin[0] < bin[1] ? bin[0] : bin[1];
            if (min > 10) {
                        result.x1 = x >4? x-4:x;
                        break;
            }
        }
        //result.x1 = minL;
    } else {
        for (int y = h-1; y > 0; y--) {
            quint32 stepCount = 0;
            quint8 * line = scanLine(y);
            for (int x = 1; x < w; x++) {
                if (line[x] != line[x-1])
                    stepCount++;
            }
            if (stepCount > 4) {
                result.y2 = h- y > 2 ? y + 2 : y;
                result.y2 = h -y > 4 ? y + 4 : result.y2;
                //result.y2 = 500;
                break;
            }
        }
        for (int x = w-2; x > 0; x--) {
            quint32  bin[2] = {0,0};
            quint32 min;
            for (int y = 0; y < h; y++) {
                quint8 * line = scanLine(y);
                bin[line[x]]++;
            }
            min = bin[0] < bin[1] ? bin[0] : bin[1];
            if (min > 10) {
                        result.x2 = x >4? x-4:x;
                        break;
            }

        }

    }
    return result;
}

void QIPBlackAndWhiteImage::copyInternal(quint8 * data, quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
    int wr = x2 - x1;
    for (int y = y1; y < y2; y++)
        memcpy(&(data[wr*(y-y1)]), &(scanLine(y)[x1]), wr);
}

QImage QIPBlackAndWhiteImage::toImage() const
{
    QImage image(w, h, QImage::Format_ARGB32);
#ifndef IPRIT_MULTITHREADING
    IntRect r = {0,0,image.width(),image.height()};
    toImageInternal(image.scanLine(0),r, image.width());
#endif
#ifdef IPRIT_MULTITHREADING
    IntRect r1 = {0,0,image.width(),image.height()/2};
    IntRect r2 = {0,image.height()/2,image.width(),image.height()};
    QFuture<void> future1 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::toImageInternal,image.scanLine(0),r1, image.width());
    QFuture<void> future2 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::toImageInternal,image.scanLine(0),r2, image.width());
    future1.waitForFinished();
    future2.waitForFinished();
#endif
    return image;
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::copy(quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
    QIPBlackAndWhiteImage result(x2-x1, y2-y1);
#ifndef IPRIT_MULTITHREADING
    copyInternal(result.data.data(), x1, x2, y1, y2);
#endif
#ifdef IPRIT_MULTITHREADING
    copyInternal(result.data.data(), x1, x2, y1, y2);
    QFuture<void> future1 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::copyInternal, result.data.data(), x1, x2, y1, (y1+y2)/2);
    QFuture<void> future2 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::copyInternal, result.data.data(), x1, x2, (y1+y2)/2, y2 - (y1+y2)/2);
    future1.waitForFinished();
    future2.waitForFinished();
#endif

    return result;
}

quint32 QIPBlackAndWhiteImage::width()
{
    return w;
}

quint32 QIPBlackAndWhiteImage::height()
{
    return h;
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::dilate(quint8 *structuringElement, int dimensions) const
{
    QIPBlackAndWhiteImage res(w, h);
    memset((void *)res.data.data(), 1, w*h);
    if ((dimensions >= w)|| (dimensions >= h))
        return res;
    quint8 ** slines = new quint8 * [dimensions];
    quint8 ** dlines = new quint8 * [dimensions];
    quint8 ** elines = new quint8 * [dimensions];
    for (int i = 0; i < dimensions; i++)
        elines[i] = &structuringElement[dimensions*i];
    int dimensions_2 = dimensions/2;
    for (int y = dimensions_2; y < h - dimensions_2; y++) {
        for (int i = 0; i < dimensions; i++) {
            int ind = y + i - dimensions_2;
            slines[i] = scanLine(ind);
            dlines[i] = res.scanLine(ind);
        }
        for (int x = dimensions_2; x < w - dimensions_2; x++) {
            if (slines[dimensions_2][x] == 0) {
                //dlines [dimensions_2][x] = slines[dimensions_2][x];
                int xs = x - dimensions_2;
                for (int i = 0; i < dimensions; i++)
                    for (int j = 0; j < dimensions; j++) {
                        dlines[i][xs+j] &= elines[i][j];
                    }
            }
        }
    }
    delete[] slines;
    delete[] elines;
    delete[] dlines;
    return res;
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::erode(quint8 *structuringElement, int dimensions) const
{
    QIPBlackAndWhiteImage res(w, h);
    memset((void *)res.data.data(), 1, w*h);
    if ((dimensions >= w)|| (dimensions >= h))
        return res;
    quint8 ** slines = new quint8 * [dimensions];
    quint8 ** dlines = new quint8 * [dimensions];
    quint8 ** elines = new quint8 * [dimensions];
    quint8 * sums = new quint8 [dimensions];
    for (int i = 0; i < dimensions; i++)
        elines[i] = &structuringElement[dimensions*i];
    int seSumsTotal = 0;
    for (int i = 0; i < dimensions; i++)
        for (int j = 0; j < dimensions; j++)
            seSumsTotal += elines[i][j];
    int dimensions_2 = dimensions/2;
    for (int y = dimensions_2; y < h - dimensions_2; y++) {
        for (int i = 0; i < dimensions; i++) {
            int ind = y + i - dimensions_2;
            slines[i] = scanLine(ind);
            dlines[i] = res.scanLine(ind);
        }
        //sums[0] = ;
        for (int x = 0; x < dimensions; x++) {
            sums[x] = 0;
            for (int i = 0; i < dimensions; i++)
                sums[x] += slines[i][x];
        }
        int totalSum = 0;
        for (int i = 0; i < dimensions; i++)
            totalSum += sums[i];
        if (totalSum <= seSumsTotal) {
            if (totalSum == 0)
                dlines[dimensions_2][dimensions_2] = 0;
            else {
                if (compareElements(elines, slines, dimensions))
                    dlines[dimensions_2][dimensions_2] = 0;
            }

        }
        for (int x = dimensions_2+1; x < w - dimensions_2; x++) {
           for (int i = 0; i < dimensions; i++)
               slines[i]++;
           //int xf = x + dimensions_2;
           totalSum -= sums[0];
           for (int i = 1; i < dimensions; i++)
               sums[i-1] = sums[i];
           sums[dimensions-1] = 0;
           for (int i = 0; i < dimensions; i++)
               sums[dimensions-1] += slines[i][dimensions-1];
           totalSum += sums[dimensions-1];
           if (totalSum <= seSumsTotal) {
               if (totalSum == 0)
                   dlines[dimensions_2][x] = 0;
               else {
                   if (compareElements(elines, slines, dimensions))
                       dlines[dimensions_2][x] = 0;
               }
           }
        }
    }
    delete[] slines;
    delete[] elines;
    delete[] dlines;
    delete[] sums;
    return res;
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::open(quint8 *structuringElement, int dimensions) const
{
    return erode(structuringElement, dimensions).dilate(structuringElement, dimensions);
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::close(quint8 *structuringElement, int dimensions) const
{
    return dilate(structuringElement, dimensions).erode(structuringElement, dimensions);
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::inverse() const
{
    QIPBlackAndWhiteImage res(w, h);
    for (quint32 y = 0; y < h; y++) {
        quint8 * sline = scanLine(y);
        quint8 * dline = res.scanLine(y);
        for (quint32 x = 0; x < w; x++)
            dline[x] = 1 - sline[x];
    }
    return res;
}

QIPBlackAndWhiteImage QIPBlackAndWhiteImage::crop() const
{
IntRect r1, r2;
#ifndef IPRIT_MULTITHREADING
    r1 = cropInternal(true);
    r2 = cropInternal(false);
#endif
#ifdef IPRIT_MULTITHREADING
    QFuture<IntRect> future1 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::cropInternal, true);
    QFuture<IntRect> future2 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::cropInternal, false);
    future1.waitForFinished();
    future2.waitForFinished();
    r1 = future1.result();
    r2 = future2.result();
#endif
    return copy(r1.x1, r2.x2, r1.y1, r2.y2);
}

QRect QIPBlackAndWhiteImage::cropGrayScaleImage(const QIPGrayscaleImage &image)
{
    IntRect r1, r2;
    #ifndef IPRIT_MULTITHREADING
        r1 = cropInternal(true);
        r2 = cropInternal(false);
    #endif
    #ifdef IPRIT_MULTITHREADING
        QFuture<IntRect> future1 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::cropInternal, true);
        QFuture<IntRect> future2 = QtConcurrent::run(this, &QIPBlackAndWhiteImage::cropInternal, false);
        future1.waitForFinished();
        future2.waitForFinished();
        r1 = future1.result();
        r2 = future2.result();
    #endif
        if ((r2.x2-r1.x1 < 32)&&(r2.y2-r1.y1 < 32))
            return QRect(0,0,0,0);
       // QIPGrayscaleImage tmp = image->copy(r1.x1, r2.x2, r1.y1, r2.y2);
       // delete *image;
       // (*image) = tmp;
        return QRect(r1.x1,r1.y1, r2.x2-r1.x1,r2.y2-r2.y1);
}


bool QIPBlackAndWhiteImage::isNull() const
{
    return w*h == 0;
}

void QIPBlackAndWhiteImage::toImageInternal(uchar *image, const IntRect &rect, int imageWidth) const
{
    int im4 =4*imageWidth;
    for (quint32 y = rect.y1; y < rect.y2; y++) {
        quint8 * line = scanLine(y);
        QRgb * lineOut = (QRgb *)&image[im4*y];
        for (quint32 x = rect.x1; x < rect.x2; x++ )
            lineOut[x] = (line[x] == 0 ? black : white);
    }

}
