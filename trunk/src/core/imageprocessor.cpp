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

#include "imageprocessor.h"
#include "qipgrayscaleimage.h"
#include "qipblackandwhiteimage.h"
#include <QImage>
#include <QMap>

const uint m_treshold = 0;

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent)

{
    img = NULL;
    bwimg = NULL;
}

ImageProcessor::~ImageProcessor()
{
    delete img;
    delete bwimg;
}

QRect ImageProcessor::crop()
{
    bwimg = new QIPBlackAndWhiteImage(img->binarize(QIPGrayscaleImage::OtsuBinarization));
    QPoint p = bwimg->cropGrayScaleImage(&img);
    delete bwimg;
    bwimg = NULL;
    return QRect(p.x(),p.y(), img->width(), img->height());
}

void ImageProcessor::loadImage(const QImage &image)
{
    img = new QIPGrayscaleImage(image,QIPGrayscaleImage::MinValue);
}

QImage ImageProcessor::gsImage() const
{
    return img->toImage();
}

void ImageProcessor::binarize()
{
    bwimg = new QIPBlackAndWhiteImage(img->binarize(QIPGrayscaleImage::GatosBinarization));
    img->blendImage(*bwimg);
    //img->darken(235);
}

void ImageProcessor::saveYGF(const QImage &image, const QString &fileName)
{
    //QIPGrayscaleImage gsi(image, QIPGrayscaleImage::MinValue);
    QIPGrayscaleImage::saveGrayscale(image, fileName);
}

QImage ImageProcessor::loadYGF(const QString &fileName)
{
    QIPGrayscaleImage gsi(fileName);
    return gsi.toImage();
}

void ImageProcessor::polishImage(QImage &image)
{
    for(int y =1; y < image.height()-1; y++) {
        QRgb * line = (QRgb *) image.scanLine(y);
        QRgb * linep = (QRgb *) image.scanLine(y-1);
        QRgb * linen = (QRgb *) image.scanLine(y+1);
        //if
        for (int x = 1; x < image.width()-1; x++) {
            if (line[x]%255 > 64) {
                if ((line[x-1]%255 < 64)&&(line[x+1]%255 < 64))
                    line[x] = 0xFF404040;
                else
                if ((linep[x]%255 < 64)&&(linen[x]%255 < 64))
                    line[x] = 0xFF404040;
                else
                    if ((linep[x-1]%255 < 64)&&(linen[x+1]%255 < 64))
                        line[x] = 0xFF404040;
                else
                        if ((linep[x+1]%255 < 64)&&(linen[x-1]%255 < 64))
                            line[x] = 0xFF404040;
            }
        }
    }

}

void ImageProcessor::polishImage2(QImage &image)
{

    /*for (int y = 0; y < image.height(); y++) {
        quint8 * line = image.scanLine(y);
        uint strokLen = 24;
        for (int x = 0; x < image.width()*4 - strokLen*4; x+=strokLen*2) {
            int strokes[8] = {0};
            int xvals[8] = {0};
            int curstrok = 0;
            for (int t = x+4; t < x+strokLen*4; t+=4) {
                if (abs(line[t]-line[t-4]) > strokes[curstrok]) {
                        strokes[curstrok] = line[t]-line[t-4];
                        xvals[curstrok] = t;
                }
                else
                    curstrok++;
                if (curstrok >= 8)
                    break;
            }
            if (curstrok < 4)
                break;
            int start = -1;
            int stop = -1;
            int k = 0;
            while (k < curstrok -1) {
                if (((strokes[k] < 0)&&(strokes[k+1] < 0))||((strokes[k] > 0)&&(strokes[k+1] > 0))) {
                    strokes[k] += strokes[k+1];
                    xvals[k] = xvals[k+1];
                    for (int l = k+1; l < curstrok-1; l++)
                        strokes[l] = strokes[l+1];
                    curstrok--;
                } else k++;
            }
            if (curstrok < 4)
                break;
            for (int m = 1; m < curstrok-1; m++) {
                if (start < 0) {
                    if (strokes[m-1] < -15)
                        if (strokes[m] > 7)
                            if (-strokes[m-1] > strokes[m])
                                start = xvals[m];
                } else {
                    if (strokes[m-1] < -7)
                        if (strokes[m] > 15)
                            if (strokes[m] > -strokes[m-1]) {
                                stop = xvals[m];
                                break;
                            }

                }
                if (stop > 0) {
                    for (int p = start; p < stop; p +=4)
                        line[p] = 60;
                }

            }
        }
    }*/
}

bool ImageProcessor::isTextHorizontal(QImage &image)
{
    return true;
    if (image.width() > image.height())
        return true;
    QMap<int, int> stripes;
    for (int y = 600; y < image.height() - 600; y++) {
        quint8 * line = image.scanLine(y);
        for (int x = 1200; x < image.width()*4 - 1200; x+=160) {
            if (stripes.contains(x)) {
                if (line[x] > 128)
                    stripes.insert(x, stripes.value(x)+1);
                else
                    if (stripes.value(x) < 600)
                        stripes.insert(x, 0);
            } else
                stripes.insert(x, 0);
        }
    }
    int longCount = 0;
    int shortCount = 0;
    foreach (int count, stripes.values()) {
        if (count >= 600)
            longCount++;
        else
            shortCount++;
    }
    if (longCount*2 > shortCount)
        return false;
    if (longCount*3 > shortCount)
        return true;
    return true;
}


