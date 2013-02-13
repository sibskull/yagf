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

int Binarize::otsu(const QImage &input, quint32 &background, int left, int top, int width, int height)
{

    if (!width)
        width = input.width();
    if (!height)
        height = input.height();
    float hist[259]={0.0F};
    uint ht[259] = {0};
 //   output = input;
    for (int y = top; y < height; y++) {
       quint32 * lineIn = (quint32 *)input.scanLine(y);
       for(int x = left; x < width; x++) {
           ht[lineIn[x]]++;
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
           m_maxK = maxK;
           maxSigmaK = sigmaK;
           m_maxSigma = sigmaK;
       }
    }

    quint32 bgcount = 0;
    quint64 bgaccum = 0;

    for (int y = top; y < height; y++) {
        quint32 * lineIn = (quint32 *)input.scanLine(y);
        for (int x = left; x < width; x++) {
            if (lineIn[x] >= maxK) {
                bgaccum += lineIn[x];
                bgcount++;
            }
        }
    }

    if (bgcount)
        background = bgaccum/bgcount;
    else
        background = 0;
    return maxK;
}

int Binarize::otsuMinimized(const QImage &input, quint32 median, quint32 upperbound, quint32 lowerBound, quint32 &background, int left, int top, int width, int height)
{

    if (!width)
        width = input.width();
    if (!height)
        height = input.height();
    float hist[259]={0.0F};
    uint ht[259] = {0};
 //   output = input;
    for (int y = top; y < height; y++) {
       quint32 * lineIn = (quint32 *)input.scanLine(y);
       for(int x = left; x < width; x++) {
           int  jj = lineIn[x];
           ht[lineIn[x]]++;
      }
    }


    int size = width*height;


    for (int i = 0; i < 256; i++) {
        hist[i] = ht[i];
        hist[i]/=size;
    }


    float ut = 0;
    for (int i = 0; i < 256; i++)
       ut+= i*hist[i];

    int maxK= m_maxK;
    int maxSigmaK= m_maxSigma;

    uint mink = median - lowerBound;
    uint upto = median + upperbound;
    for (int k = mink; k < upto; ++k) {
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

    quint32 bgcount = 0;
    quint64 bgaccum = 0;

    for (int y = top; y < height; y++) {
        quint32 * lineIn = (quint32 *)input.scanLine(y);
        for (int x = left; x < width; x++) {
            if (lineIn[x] >= maxK) {
                bgaccum += lineIn[x];
                bgcount++;
            }
        }
    }

    if (bgcount)
        background = bgaccum/bgcount;
    else
        background = 0;
    return maxK;

}
