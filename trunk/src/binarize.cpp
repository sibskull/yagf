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

void Binarize::buildHist(const QImage &input, int left, int top, int width, int height)
{
    uint ht[259] = {0};
 //   output = input;
    for (int y = top; y < height; y++) {
       quint32 * lineIn = (quint32 *)input.scanLine(y);
       for(int x = left; x < width; x++) {
           ht[lineIn[x]]++;
      }
    }


    int size = (width - left)*(height-top);


    for (int i = 0; i < 256; i++) {
        hist[i] = ht[i];
        hist[i]/=size;
    }

}

int Binarize::otsu()
{

    float ihist[256];
    float ut = 0;
    for (int i = 0; i < 256; i++)
        ut+= (ihist[i] = i*hist[i]);

    int maxK=0;
    int maxSigmaK=0;

    float wk = 0;
    float uk = 0;
    for (int k = 0; k < 256; ++k) {
       //for (int i = 0; i <= k; ++i)
        if (hist[k] == 0)
            continue;
        wk += hist[k];

       //for (int i = 0; i <=k; ++i)
       uk += ihist[k];

       float sigmaK = 0;
       if ((wk !=0) && (wk!=1))
          sigmaK  = ((ut*wk - uk)*(ut*wk - uk))/(wk*(1-wk));
       if (sigmaK > maxSigmaK) {
           maxK = k;
           m_maxK = maxK;
           maxSigmaK = sigmaK;
       }
    }

    return maxK;
}

QImage Binarize::tiledOtsu(const QImage &input)
{
    QImage mask(input.size(), input.format());
    int xstep = input.width()/8;
    int ystep  = input.height()/8;
    if (xstep < 64) xstep = 64;
    if (ystep < 64) ystep = 64;
    int y = 0;
    int x = 0;
    int xfin, yfin;
    bool landscape = input.width() > input.height();
    if (landscape) {
        yfin = y + input.height();
        xfin = x + 0.5*input.width();
    } else {
        xfin = x + input.width();
        yfin = y + 0.5*input.height();
    }
    buildHist(input, x, y, xfin, yfin);
    int th = otsu();
    polishHist(th);
    fillMask(input, th, x, y, xfin, yfin, mask);
    /*if (landscape) {
        y = 0;
        x = xfin;
        xfin = x + 0.33*input.width();
    } else {
        x = 0;
        y = yfin;
        yfin = y + 0.33*input.height();
    }
    buildHist(input, x, y, xfin, yfin);
    th = otsu();
    polishHist(th);
    fillMask(input, th, x, y, xfin, yfin, mask);*/
    if (landscape) {
        y = 0;
        x = xfin;
        xfin = input.width();
    } else {
        x = 0;
        y = yfin;
        yfin = input.height();
    }
    buildHist(input, x, y, xfin, yfin);
    th = otsu();
    polishHist(th);
    fillMask(input, th, x, y, xfin, yfin, mask);

    return mask;
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

void Binarize::fillMask(const QImage &input, int threshold, int left, int top, int width, int height, QImage &mask)
{
    for (int y = top; y < height; y++) {
        quint32 * lineIn = (quint32 *)input.scanLine(y);
        QRgb * lineOut = (QRgb *)input.scanLine(y);
        for (int x = left; x < width; x++) {
            if (lineIn[x] > threshold)
                lineOut[x] = clWhite;
            else
                lineOut[x] = clBlack;
        }
    }
}

void Binarize::polishHist(int &th)
{
    uint ww = 4;
    for (int k = 0; k < 256 - ww; k++) {
        float sum = 0;
        for (int l = k; l < k + ww; l++)
            sum += hist[l];
        if (sum >= 0.25) {
            if (k > 200) {
                th = th - 32;
                break;
            }

        }
    }
    if (th > 220) th-=32;
}
