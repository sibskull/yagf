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

#include "binarize.h"
#include <QImage>

Binarize::Binarize(QObject *parent) :
    QObject(parent)
{
}

const QRgb clBlack = qRgb(0,0,0);
const QRgb clWhite = qRgb(255,255,255);


void Binarize::otsu(const QImage &input, QImage &output, int left, int top, int width, int height)
{
   uchar lut[256*3] = {0};
   int lutcount = 0;
   for (int i = 0; i < 256; i++) {
       for(int j = 0; j < 3; j++) {
           lut[lutcount] = i;
           lutcount++;
       }
   }

    if (!width)
        width = input.width();
    if (!height)
        height = input.height();
    float hist[259]={0.0F};
    uint ht[259] = {0};
    output = input;
    for (int y = top; y < height; y++) {
       QRgb * lineIn = (QRgb *)input.scanLine(y);
       quint32 * lineOut = (quint32 *)output.scanLine(y);
       for(int x = left; x < width; x++) {
           QRgb cur = lineIn[x];
           uint grayLevel = (qRed(cur) + qGreen(cur) + qBlue(cur));
           ht[lut[grayLevel]]++;
           lineOut[x] = lut[grayLevel];
      }
    }

/*    for (int y = top; y < height; y++) {
       QRgb * lineIn = (uchar *)input.scanLine(y);
       QRgb * lineOut = (QRgb *)output.scanLine(y);
       for(int x = left; x < width; x++) {
           QRgb cur = lineIn[x];
           //int r = /3;
           //int g = qGreen(cur)/3;
           //int b = qBlue(cur)/3;
           uint grayLevel = (qRed(cur) + qGreen(cur) + qBlue(cur))/3;
           lineOut[x] = 0xFF000000 +grayLevel;
           hist[grayLevel]+=1;
       }
    }*/

    int size = width*height;


    for (int i = 0; i < 256; i++) {
        hist[i] = ht[i];
        hist[i]/=size;
    }


    float ut = 0;
    for (int i = 0; i < 256; i++)
       ut+= i*hist[i];

    int maxK=0;
    int maxSigmaK=0;

    for (int k = 0; k < 256; ++k) {
       float wk = 0;
       for (int i = 0; i <= k; ++i)
           wk += hist[i];
       float uk = 0;
       for (int i = 0; i <=k; ++i)
       uk += i*hist[i];

       float sigmaK = 0;
       if ((wk !=0) && (wk!=1))
          sigmaK  = ((ut*wk - uk)*(ut*wk - uk))/(wk*(1-wk));
       if (sigmaK > maxSigmaK) {
           maxK = k;
           maxSigmaK = sigmaK;
       }
    }

    for (int y = top; y < height; y++) {
        quint32 * lineOut = (quint32 *)output.scanLine(y);
        for (int x = left; x < width; x++) {
           lineOut[x] = lineOut[x] < maxK ? clBlack : clWhite;
       }
    }
}
