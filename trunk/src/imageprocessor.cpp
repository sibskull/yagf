/*
    YAGF - cuneiform OCR graphical front-end
    Copyright (C) 2009 Andrei Borovsky <anb@symmetrica.net>

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
#include "binarize.h"
#include <QImage>

ImageProcessor::ImageProcessor(QObject *parent) :
    QObject(parent)
{
}

void ImageProcessor::start(const QImage &input)
{
   img = input;
   toGrayScale(input);
   m_treshold = Binarize::otsu(gray, m_middleBG);
}

void ImageProcessor::binarize(const QImage &input)
{
    img = input;
    m_treshold = Binarize::otsu(gray, m_treshold);
}

QRect ImageProcessor::crop()
{
    int newtop = cropTop(img);
    int newbottom = cropBottom(img);
    int newleft = cropLeft(img);
    int newright = cropRight(img);
    //if ((newright < img.width()-2)&&(newbottom < img.height()-2)) {
        img = img.copy(newleft, newtop, newright-newleft, newbottom-newtop);
        gray = gray.copy(newleft, newtop, newright-newleft, newbottom-newtop);
    //}
    return QRect(newleft, newtop, img.width(), img.height());
}

QImage ImageProcessor::finalize()
{
    quint32 k[8] = {m_treshold};
    quint32 prevk = m_treshold;
    quint32 l = img.width()/8;
    for (int y = 0; y < img.height(); y++) {
        QRgb * lineGray = (QRgb *)gray.scanLine(y);
        QRgb * lineImg = (QRgb *)img.scanLine(y);
        /*for (int i =0; i <8; i++)
            for (int j = 0; j < l; j++) {
                quint32 ind = i*l + j;
                if (lineMask[ind] == clWhite) {
                    QRgb pix = lineImg[ind];
                    quint32 grl = (qRed(pix) + qGreen(pix) + qBlue(pix))/3;
                    acc += grl;
                    count++;
                }
                if (count < 64)
                    k[i] = prevk;
                else {
                    k[i] = acc/count;
                    prevk = k[i];
                }
            }*/
        for (int x = 0; x < img.width(); x++) {
           if (lineGray[x] < m_treshold)
                lineImg[x] = qRgb(qRed(lineImg[x])/2,qGreen(lineImg[x])/2,qBlue(lineImg[x])/2);
           else
               lineImg[x] = qRgb(255, 255, 255);
        }
    }
    return img;
}

void ImageProcessor::toGrayScale(const QImage &input)
{
    gray = QImage(input.width(), input.height(), QImage::Format_RGB32);

    uchar lut[256*3] = {0};

    int lutcount = 0;
    for (int i = 0; i < 256; i++) {
        for(int j = 0; j < 3; j++) {
            lut[lutcount] = i;
            lutcount++;
        }
    }

    int width = input.width();
    int  height = input.height();
    for (int y = 0; y < height; y++) {
        QRgb * lineIn = (QRgb *)input.scanLine(y);
        quint32 * lineOut = (quint32 *)gray.scanLine(y);
        for(int x = 0; x < width; x++) {
            QRgb cur = lineIn[x];
            uint grayLevel = (qRed(cur) + qGreen(cur) + qBlue(cur));
            lineOut[x] = lut[grayLevel];
       }
    }
}

int ImageProcessor::cropTop(const QImage &input)
{
    int newtop = 0;
    for (int y = 0; y < input.height(); y++) {
        quint32 * lineOut = (quint32 *)gray.scanLine(y);
        int count = 0;
        int strx = 0;
        for (int x = 0; x < input.width(); x++) {
            if (lineOut[x] >= m_treshold)
               count++;
            else {
                if (count)
                    strx += count/8;
                count = 0;
            }
        }

        //if (count/(input.width() + 1 - count) >= 15)
        if(strx < 8)
            newtop++;
        else break;
        strx = 0;
    }
    return newtop;
}

int ImageProcessor::cropBottom(const QImage &input)
{
    int newbottom = input.height()-1;
    for (int y = input.height()-1; y > 0; y--) {
        QRgb * lineOut = (QRgb *)gray.scanLine(y);
        int count = 0;
        int strx = 0;
        for (int x = 0; x < input.width(); x++) {
            if (lineOut[x] >= m_treshold)
               count++;
            else {
                if (count)
                    strx += count/8;
                count = 0;
            }
        }
        if(strx < 8)
            newbottom--;
        else break;
    }
    return newbottom;
}

int ImageProcessor::cropLeft(const QImage &input)
{
    int newleft = 0;
    for (int x = 0; x < input.width(); x++) {
        int count = 0;
        int strx = 0;
        for (int y = 0; y < input.height(); y++) {
           QRgb * lineOut = (QRgb *)gray.scanLine(y);
           if (lineOut[x] >= m_treshold)
               count++;
           else {
               if (count)
                   strx += count/8;
               count = 0;
           }
        }
        if (strx<8)
            newleft++;
        else break;
    }
    return newleft;
}

int ImageProcessor::cropRight(const QImage &input)
{
    int newright = input.width()-1;
    for (int x = input.width()-1; x > 0; x--) {
        int count = 0;
        int strx = 0;
        for (int y = 0; y < input.height(); y++) {
           QRgb * lineOut = (QRgb *)gray.scanLine(y);
           if (lineOut[x] >= m_treshold)
               count++;
           else {
               if (count)
                   strx += count/8;
               count = 0;
           }
        }
        if (strx<8)
            newright--;
        else break;
    }
    return newright;
}

uchar ImageProcessor::getMediumBG(QRgb *line, uint length, uchar prev)
{
    quint32 * il = (quint32 *) line;
    quint32 acc = 0;
    quint32 count = 0;
    for (int x = 0; x < length; x++) {
        if (il[x] >= m_treshold) {
            acc += il[x];
            count++;
        }
    }
    if (count < 64)
        return prev;
    else
        return acc/count;
}
