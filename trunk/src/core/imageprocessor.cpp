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
    img = new QIPGrayscaleImage(image);
}

QImage ImageProcessor::gsImage() const
{
    return img->toImage();
}

void ImageProcessor::binarize()
{
    bwimg = new QIPBlackAndWhiteImage(img->binarize(QIPGrayscaleImage::GatosBinarization));
    img->blendImage(*bwimg);
}


