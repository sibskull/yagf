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

#include "settings.h"
#include "qipgrayscaleimage.h"
#include "binarize.h"
#include <cmath>
#include "common.h"
#include <QRect>
#include <QFile>

const QString fheader = QString::fromUtf8("YGF1");

static const int kSharpen [3][3]= {{0,-1,0}, {-1,5,-1}, {0,-1,0}};
static const int wkSharpen = 1;
static const int kBlur [3][3]= {{1,1,1}, {1,1,1}, {1,1,1}};
static const int wkBlur = 5;
static const int kEdges [3][3]= {{-1,-1,-1}, {-1,8,-1}, {-1,-1,-1}};
static const int wkEdges = 1;

QIPGrayscaleImage::QIPGrayscaleImage(const QImage &image, GrayscaleConversion conversionMethod) : data(new quint8[image.width()*image.height()], deallocator<quint8>)
{
    h = image.height();
    w = image.width();
    //data = QSharedPointer<quint8>();
    switch (conversionMethod) {
    case RGBDevideByThree:
        toGrayScale(image);
        break;
    case MinMaxValue:
        toGrayscaleMinMax(image);
        break;
    case MinValue:
        toGrayscaleMinOrMax(image, true);
        break;
    case MaxValue:
        toGrayscaleMinOrMax(image, false);
        break;
    case MaxEntropyChannel:
        toGrayScaleByEntropyChannel(image, true);
        break;
    case MinEntropyChannel:
        toGrayScaleByEntropyChannel(image, false);
        break;
    default:
        toGrayScale(image);
        break;
    }
}

QIPGrayscaleImage QIPGrayscaleImage::fromImage(const QImage &image, QIPGrayscaleImage::GrayscaleConversion conversionMethod)
{
    return QIPGrayscaleImage(image, conversionMethod);
}

bool QIPGrayscaleImage::isNull() const
{
    return w*h == 0;
}

QIPGrayscaleImage::QIPGrayscaleImage(const QIPGrayscaleImage &I) : data(I.data.data(), deallocator<quint8>)
{
    w = I.w;
    h = I.h;
}

QIPGrayscaleImage::QIPGrayscaleImage(const QString &ygfFileName)
{
    w = 0;
    h = 0;
    QFile f(ygfFileName);
    if (!f.open(QIODevice::ReadOnly))
        return;
    QPoint p = loadHeader(&f);
    w = p.x();
    h = p.y();
    if (w*h == 0) {
        f.close();
        return;
    }
    quint8 * d = new quint8[w*h];
    data = QSharedPointer<quint8>(d, deallocator<quint8>);
    f.read((char*)data.data(), w*h);
    f.flush();
    f.close();
}

QIPGrayscaleImage::~QIPGrayscaleImage()
{
    //delete [] data;
}

QImage QIPGrayscaleImage::toImage() const
{
    QImage image(w, h, QImage::Format_ARGB32);
    #ifndef IPRIT_MULTITHREADING
    IntRect r = {0,0,image.width(),image.height()};
        toImageInternal(image.scanLine(0),r, image.width());
    #endif
#ifdef IPRIT_MULTITHREADING
    IntRect r1 = {0,0,image.width(),image.height()/2};
    IntRect r2 = {0,image.height()/2,image.width(),image.height()};
    QFuture<void> future1 = QtConcurrent::run(this, &QIPGrayscaleImage::toImageInternal,image.scanLine(0),r1, image.width());
    QFuture<void> future2 = QtConcurrent::run(this, &QIPGrayscaleImage::toImageInternal,image.scanLine(0),r2, image.width());
    future1.waitForFinished();
    future2.waitForFinished();
#endif

    return image;
}

void QIPGrayscaleImage::histogram(QIPHistogram &result, quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
if (x2 == 0) x2 = w;
if (y2 == 0) y2 = h;
#ifndef IPRIT_MULTITHREADING
    IntRect r = {x1,y1,x2,y2};
    histogramInternal(&result[0], r);
#endif
#ifdef IPRIT_MULTITHREADING
    IntRect r1 = {x1,y1,x2,(y1+y2)/2};
    IntRect r2 = {x1,(y1+y2)/2,x2,y2};
    qreal h1[256] = {0.0};
    qreal h2[256] = {0.0};
    QFuture<void> future1 = QtConcurrent::run(this, &QIPGrayscaleImage::histogramInternal,&h1[0],r1);
    QFuture<void> future2 = QtConcurrent::run(this, &QIPGrayscaleImage::histogramInternal,&h2[0],r2);
future1.waitForFinished();
future2.waitForFinished();
for (int i = 0; i < 256; i++)
    result[i] = (h1[i] + h2[i])/2.;
#endif

}

void QIPGrayscaleImage::histogramInternal(qreal * result, const IntRect &r) const
{
    uint ht[256] = {0};
    for (quint32 y = r.y1; y < r.y2; y++) {
       quint8 * lineIn = scanLine(y);
       for(quint32 x = r.x1; x < r.x2; x++) {
           ht[lineIn[x]]++;
      }
    }


    int size = (r.x2-r.x1)*(r.y2-r.y1);

    for (int i = 0; i < 256; i++) {
        result[i] = ht[i];
        result[i]/=size;
    }
}

void QIPGrayscaleImage::copyInternal(const IntRect &r, uint *image) const
{
    for (uint y = r.y1; y < r.y2; y++) {
        quint8 * line = scanLine(y);
        uint * lineout = &image[y*w];
        for (uint x = r.x1; x < r.x2; x++)
            lineout[x] = line[x];
    }

}

void QIPGrayscaleImage::darken(uint threshold)
{
    uint dataSize = w*h;
    quint8 * d = (quint8 *)data.data();
    uint ac = 0;
    for (uint i = 0; i < dataSize; i++) {
        ac = ac + d[i];
    }
    ac = ac/dataSize;
    if (ac > threshold)
        for (uint i = 0; i < dataSize; i++) {
            d[i] = qMax(0, d[i]*d[i]/255);
        }
}

bool QIPGrayscaleImage::save(const QString &fileName, bool overwrite)
{

    quint16 hx = h;
    quint16 wx = w;
    QFile f(fileName);
    if ((f.exists())&&(!overwrite))
        return false;
    bool res = f.open(QIODevice::WriteOnly);
    if (!res)
        return false;
    f.write(fheader.toAscii(), 4);
    f.write((char*) &hx, 2);
    f.write((char*) &wx, 2);
    f.write((char*)data.data(), wx*hx);
    f.flush();
    f.close();
    return true;
}


bool QIPGrayscaleImage::saveGrayscale(const QImage &image, const QString &fileName, bool overwrite)
{
    quint16 hx = image.height();
    quint16 wx = image.width();
    QFile f(fileName);
    if ((f.exists())&&(!overwrite))
        return false;
    if(!f.open(QIODevice::WriteOnly))
        return false;
    f.write(fheader.toAscii(), 4);
    f.write((char*) &hx, 2);
    f.write((char*) &wx, 2);
    quint8 * d = new quint8[wx];
    for (int  y = 0; y < hx; y++) {
        quint8 * line = (quint8 *) image.scanLine(y);
        for (int x = 0; x < wx; x++)
            d[x] = line[x*4];
        f.write((char*)d,wx);
    }
    delete[] d;
    f.flush();
    f.close();
    return true;
}

quint32 QIPGrayscaleImage::width() const
{
    return w;
}

quint32 QIPGrayscaleImage::height() const
{
    return h;
}

QIPGrayscaleImage QIPGrayscaleImage::copy(quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
    QIPGrayscaleImage result(x2 - x1, y2 - y1);
    for (uint y = y1; y < y2; y ++) {
        quint8 * src = &scanLine(y)[x1];
        quint8 * dst = result.scanLine(y-y1);
        memcpy(dst, src, result.w);
    }
    return result;
}

QIPBlackAndWhiteImage QIPGrayscaleImage::binarize(QIPGrayscaleImage::BinarizationMethod method) const
{
    switch (method) {
    case OtsuBinarization :
        return otsuBinarize();
        break;
    case NiblackBinarization :
        return niblackSauvolaBinarize(false);
        break;
    case SauvolaBinarization :
        return niblackSauvolaBinarize(true);
        break;
    case MaxEntropyBinarization :
        return maxEntropyBinarize();
        break;
    case BradleyBinarization :
        return bradleyBinarize();
        break;
    case IterativeBinarization:
        return iterativeBinarize();
        break;
    case BernsenBinarization:
        return bernsenBinarize();
        break;
    case GatosBinarization:
        return gatosBinarize();
        break;

    default:
        break;
    }
    return QIPBlackAndWhiteImage(0,0);
}

QIPGrayscaleImage QIPGrayscaleImage::sharpen() const
{
   return applyFilter(SharpenFilter);
}

QIPGrayscaleImage QIPGrayscaleImage::blur() const
{
    return applyFilter(BlurFilter);
}

void QIPGrayscaleImage::isolateEdges()
{
    applyFilter(EdgesFilter);
}

void QIPGrayscaleImage::invert()
{
    for (uint y = 0; y < h; y++) {
        quint8 * line = scanLine(y);
        for (uint x = 0; x < w; x++)
            line[x] = 255 - line[x];
    }
}

void QIPGrayscaleImage::wienerFilter()
{
    qr_wiener_filter(data.data(),w,h);
}

void QIPGrayscaleImage::blendImage(const QIPBlackAndWhiteImage &image)
{
    if (w*h < 32) return;
    if (h < 4) return;
    const quint8 * bw = image.data.data();
    quint8 * gs = data.data();
    for (int i = 0; i < w; i++)
        gs[i] = bw[i] == 1 ? qMin(gs[i] + 32, 255) : gs[i]*3/4;
    for (int i = 0; i < w*h; i+=w) {
        gs[i] = bw[i] == 1 ? qMin(gs[i] + 32, 255) : gs[i]*3/4;
        gs[i+w-1] = bw[i+w-1] == 1 ? qMin(gs[i+w-1] + 32, 255) : gs[i+w-1]*3/4;
    }
    for (int i = w*(h-1); i < w*h; i++)
        gs[i] = bw[i] == 1 ? qMin(gs[i] + 32, 255) : gs[i]*3/4;
    int up =Settings::instance()->getForegroundBrightenFactor();
    uint d1 = 3;
    uint d2 = 4;
    uint ra = 0;
    uint c = 0;

    for (int i = w+1; i < w*(h-1)-1; i++) {
        if (bw[i] == 1) {
            ra += gs[i];
            c++;
            if ((bw[i-1] != 0)||(bw[i+1] != 0))
                if ((bw[i-2] != 0)||(bw[i+1] != 0))
                if ((bw[i-w] != 0)||(bw[i+w] != 0))
                    if ((bw[i-w-1] != 0)||(bw[i+w+1] != 0))
                        if ((bw[i-w+1] != 0)||(bw[i+w-1] != 0))
                                gs[i] = qMin(gs[i] + up, 255);

        } else
            gs[i] = gs[i]*d1/d2;
    }
    up = Settings::instance()->getGlobalBrightenFactor();
    if (ra/c < Settings::instance()->getDarkBackgroundThreshold())
        for (int i = w+1; i < w*(h-1)-1; i++)
            gs[i] = qMin(gs[i]+up, 255);
    up = Settings::instance()->getGlobalDarkenFactor();
    int thr = Settings::instance()->getGlobalDarkenThreshold();
    for (int i = w+1; i < w*(h-1)-1; i++)
        if ((gs[i] < thr)&&(gs[i]>up)) gs[i]-=up;
}

QIPGrayscaleImage::QIPGrayscaleImage(quint32 width, quint32 height) : data(new quint8[width*height])
{
    w = width;
    h = height;
}

QPoint QIPGrayscaleImage::loadHeader(QFile *file)
{
    QPoint res(0,0);
    char header[5] = {0};
    file->read(header, 4);
    if (QString::fromAscii(header) != fheader)
        return res;
    quint16 wx, hx;
    file->read((char*) &wx, 2);
    file->read((char*) &hx, 2);
    res.setX(wx);
    res.setY(hx);
    return res;
}


void QIPGrayscaleImage::toGSRGDBBy3(const QImage &input, int top, int left, int bottom, int right)
{
    uchar lut[256*3] = {0};

    int lutcount = 0;
    for (int i = 0; i < 256; i++) {
        for(int j = 0; j < 3; j++) {
            lut[lutcount] = i;
            lutcount++;
        }
    }
    for (int y = top; y < bottom; y++) {
        QRgb * lineIn = (QRgb *)input.scanLine(y);
        quint8 * lineOut = scanLine(y);
        for(int x = left; x < right; x++) {
            QRgb cur = lineIn[x];
            uint grayLevel = (qRed(cur) + qGreen(cur) + qBlue(cur));
            lineOut[x] = lut[grayLevel];
       }
    }
}

void QIPGrayscaleImage::toImageInternal(uchar * image, const IntRect &rect, int imageWidth) const
{
    int im4 =4*imageWidth;
    for (quint32 y = rect.y1; y < rect.y2; y++) {

        quint8 * line = scanLine(y);
        QRgb * lineOut = (QRgb *)&(image[im4*y]);
        for (quint32 x = rect.x1; x < rect.x2; x++ ) {
             lineOut[x] = qRgb(line[x], line[x], line[x]);
        }
    }
}

void QIPGrayscaleImage::toGrayScale(const QImage &input)
{
#ifndef IPRIT_MULTITHREADING
    toGSRGDBBy3(input,0,0,input.height(),input.width());
#endif
#ifdef IPRIT_MULTITHREADING
    QFuture<void> future1 = QtConcurrent::run(this, &QIPGrayscaleImage::toGSRGDBBy3,input,0,0,input.height()/2,input.width());
    QFuture<void> future2 = QtConcurrent::run(this, &QIPGrayscaleImage::toGSRGDBBy3,input,input.height()/2,0,input.height(),input.width());
    future1.waitForFinished();
    future2.waitForFinished();
#endif

}

quint8 QIPGrayscaleImage::otsuThreshold(quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
    qreal hist[256];
    histogram(hist, x1, x2, y1, y2);
    qreal ihist[256];
    qreal ut = 0;
    for (int i = 0; i < 256; i++)
        ut+= (ihist[i] = i*hist[i]);

    int maxK=0;
    int maxSigmaK=0;

    qreal wk = 0;
    qreal uk = 0;
    for (int k = 0; k < 256; ++k) {
        if (hist[k] == 0)
            continue;
        wk += hist[k];
        uk += ihist[k];

        qreal sigmaK = 0;
        if ((wk !=0) && (wk!=1))
            sigmaK  = ((ut*wk - uk)*(ut*wk - uk))/(wk*(1-wk));
        if (sigmaK > maxSigmaK) {
            maxK = k;
            maxSigmaK = sigmaK;
        }
    }
    return maxK;
}

quint8 QIPGrayscaleImage::simpleThreshold(quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
    if (x2 == 0) x2 = w;
    if (y2 == 0) y2 = h;
    quint32 accum = 0;
    for (uint y = y1; y < y2; y ++) {
        quint8 * lineIn = scanLine(y);
        for(quint32 x = x1; x < x2; x++)
            accum += lineIn[x];
    }
    return accum/((x2-x1)*(y2-y1));
}

quint8 QIPGrayscaleImage::maxEntropyThreshold(quint32 x1, quint32 x2, quint32 y1, quint32 y2) const
{
    if (x2 == 0) x2 = w;
    if (y2 == 0) y2 = h;
    qreal hist[256];
    histogram(hist, x1, x2, y1, y2);
    qreal cdv[256];
    cdv[0] = hist[0];
    for (uint i = 1; i < 256; i++)
        cdv[i] = cdv[i - 1] + hist[i];
    qreal epsilon = .1e-256;
    qreal hBlack[256] = {0.0};
    qreal hWhite[256] = {0.0};
    for (uint t = 0; t < 256; t++) {
         if (cdv[t] > epsilon) {
           qreal hhB = 0;
           for (int i = 0; i <= t; i++) {
             if (hist[i] > epsilon)
               hhB -= hist[i] / cdv[t] * log(hist[i] / cdv[t]);
           }
           hBlack[t] = hhB;
         } else {
           hBlack[t] = 0;
         }
         double cdi = 1 - cdv[t];
         if (cdi > epsilon) {
           qreal hhW = 0;
           for (int i = t + 1; i < 256; ++i) {
             if (hist[i] > epsilon)
               hhW -= hist[i] / cdi * log(hist[i] / cdi);
           }
           hWhite[t] = hhW;
         } else {
           hWhite[t] = 0;
         }
       }

       qreal maxIndex = hBlack[0] + hWhite[0];
       quint8 maxT = 0;
       for (uint t = 1; t < 256; t++) {
         double j = hBlack[t] + hWhite[t];
         if (j > maxIndex) {
            maxIndex = j;
            maxT = t;
         }
       }
       return  maxT;
}

qreal QIPGrayscaleImage::meanOfDistribution(quint32 x1, quint32 x2, quint32 y1, quint32 y2)
{
    qreal hist[256];
    histogram(hist, x1, x2, y1, y2);
    qreal result = 0;
    for (uint i = 0; i < 256; i++)
        result += hist[i]*i;
    return result;
}

qreal QIPGrayscaleImage::entropy(quint32 x1, quint32 x2, quint32 y1, quint32 y2)
{
    qreal hist[256];
    histogram(hist, x1, x2, y1, y2);
    qreal result = 0;
    for (uint i = 0; i < 256; i++)
        result += hist[i]*log(hist[i]);
    return -result;
}

void QIPGrayscaleImage::equalize()
{
    qreal hist[256];
    histogram(hist);
    quint8 tt[256];
    //tt[0] = 0;
    for (uint i = 0; i < 256; i++) {
        tt[i] = cdf(hist, i)*255;
    }

    for (int y = 0; y < h; y++) {
        quint8 * lineOut = scanLine(y);
        for(int x = 0; x < w; x++)
            lineOut[x] = tt[lineOut[x]];
    }
}

qreal QIPGrayscaleImage::lpcEntropy(quint32 x1, quint32 x2, quint32 y1, quint32 y2)
{
    uint ihist[256] = {0};
    if (x2 == 0) x2 = w;
    if (y2 == 0) y2 = h;
    for (int y = y1; y < y2; y++) {
        quint8 * lineOut = scanLine(y);
        ihist[lineOut[x1]]++;
        ihist[lineOut[x1+1]]++;
        ihist[lineOut[x1+2]]++;
        quint8 cl = lineOut[x1+2];
        for(uint x = x1 + 3; x < x2; x++) {
            quint8 cp = predictor(&lineOut[x]);
            if ((cp-lineOut[x])*(cp-lineOut[x]) < 2) {
                ihist[cl]++;
            } else {
                cl = lineOut[x];
                ihist[cl]++;
            }
        }
    }
    qreal hist[256] = {0.0};
    qreal k = 1/((x2-x1)*(y2-y1));
    qreal sum = 0;
    for (uint i =0; i < 256; i++) {
        hist[i] = ihist[i]*k*log(ihist[i]*k);
        sum += hist[i];
    }
    return -sum;
}

qreal QIPGrayscaleImage::variance(quint32 x1, quint32 x2, quint32 y1, quint32 y2)
{
    qreal hist[256];
    histogram(hist, x1, x2, y1, y2);
    qreal max = 0;
    quint8 imax;
    for (int i = 0; i < 256; i++)
        if (hist[i] > max) {
            max = hist[i];
            imax = i;
        }
    qreal sum = 0;
    for (int i = 0; i < 256; i++)
        sum += abs(i-imax)*hist[i];
    return sum;
}

quint8 *QIPGrayscaleImage::scanLine(quint32 y) const
{
    return &(data.data()[y*w]);
}

quint8 QIPGrayscaleImage::pixel(quint32 x, quint32 y) const
{
    return data.data()[x + y*w];
}

void QIPGrayscaleImage::setPixel(quint32 x, quint32 y, quint8 value)
{
    data.data()[x+y*w] = value;
}

quint8 QIPGrayscaleImage::nextInColumn(quint32 x, quint32 &y)
{
    y++;
    return data.data()[x + y*w];
}

quint8 QIPGrayscaleImage::prevInColumn(quint32 x, quint32 &y)
{
    y--;
    return data.data()[x + y*w];
}

qreal QIPGrayscaleImage::cdf(QIPHistogram hist, quint8 x)
{
    qreal result = 0;
    for (uint i = 0; i < x+1; i++)
        result +=hist[i];
    return result;
}

quint8 QIPGrayscaleImage::predictor(quint8 * x)
{
    quint8 xp[3] = {*(x-3), *(x-2), *(x-1) };
    int d1 = xp[1] - xp[0];
    int d2 = xp[2] - xp[1];
    int dp = 2*d2 - d1;
    int xd = *x + dp;
    xd = qBound(0, xd, 255);
    return (quint8) xd;
}

QIPGrayscaleImage QIPGrayscaleImage::applyFilter(int type) const
{
    QIPGrayscaleImage result(w, h);
    int kernel [3][3];
    int wk;
    int a;
    if (type == SharpenFilter) {
        memcpy(kernel, kSharpen, 9*sizeof(int));
        wk = wkSharpen;
        a = 0;
    }
    if (type == BlurFilter) {
        memcpy(kernel, kBlur, 9*sizeof(int));
        wk = wkBlur;
        a = 0;
    }
    if (type == EdgesFilter) {
        memcpy(kernel, kEdges, 9*sizeof(int));
        wk = wkEdges;
        a = 128;
    }
    for(int x= 1; x < w - 1; x++) {
        for(int y= 1; y < h - 1; y++) {
            int c = 0;
            for(int i = -1; i <= 1; i++) {
                for(int j = -1; j <= 1; j++)
                    c += pixel(x+i, y+j)*kernel[i+1][j+1];
                c = qBound(0, c/wk + a, 255);
                result.setPixel(x, y, c);
            }
        }
    }
    if (type==SharpenFilter)
        result.invert();
    return result;
}


QIPBlackAndWhiteImage QIPGrayscaleImage::niblackSauvolaBinarize(bool sauvola) const
{
    QIPBlackAndWhiteImage result(w, h);
    const uint WindowSize  = 15;
    const uint halfWindowSize = WindowSize/2;
    const qreal weight = 0.2;
    uint xMin = halfWindowSize;
    uint yMin = halfWindowSize;
    int xMax = w - 1 - halfWindowSize;
    int yMax = h - 1 - halfWindowSize;
    quint8 * output = result.data.data();
    qreal sumPixelsWindow = 0;
    qreal sum2PixelsWindow = 0;
    qreal localMean = 0;
    qreal localVar = 0;
    qreal localStd = 0;
    qreal localValue = 0;
    qreal mainSumPixelWindow = 0;
    qreal mainSum2PixelWindow = 0;
    qreal *sumCols = new qreal[w]; //= {0.0};
    qreal *sumSqCols = new qreal[w];// = {0.0};
    memset(sumCols, 0, w*sizeof(qreal));
    memset(sumSqCols, 0, w*sizeof(qreal));
    for (uint i = 0; i < WindowSize; i++){
        for (uint x = 0; x < w; x++){
            sumCols[x]  += pixel(x, i);
            sumSqCols[x] += pixel(x, i)*pixel(x, i);
        }
    }
    for(uint j = 0; j < WindowSize; j++){
                mainSumPixelWindow  += sumCols[j];
                mainSum2PixelWindow += sumSqCols[j];
    }
    for(int y = yMin; y <= yMax; y++) {
        for(int j = xMin; j <= xMax; j++) {
         if(y == xMin){
                    if(j == yMin) {
                        sumPixelsWindow =  mainSumPixelWindow;
                        sum2PixelsWindow    =  mainSum2PixelWindow;
                    }else{
                        sumPixelsWindow += (sumCols[j+halfWindowSize]   - sumCols[j-halfWindowSize-1]);
                        sum2PixelsWindow    += (sumSqCols[j+halfWindowSize]  - sumSqCols[j-halfWindowSize-1]);
                    }
                }else{
                    if(j == yMin) {
                        for (uint x = 0; x < w; x++) {
                           sumCols[x]  += (pixel(x, y+halfWindowSize) - pixel(x, y-halfWindowSize-1));
                           sumSqCols[x] += (pixel (x, y+halfWindowSize)*pixel (x, y+halfWindowSize) - pixel(x, y-halfWindowSize-1)*pixel(x, y-halfWindowSize-1));
                        }
                        mainSumPixelWindow  = 0;
                        mainSum2PixelWindow = 0;
                        for(uint k = 0; k < WindowSize; k++) {
                                mainSumPixelWindow  += sumCols[k];
                                mainSum2PixelWindow += sumSqCols[k];
                        }

                        sumPixelsWindow = mainSumPixelWindow;
                        sum2PixelsWindow = mainSum2PixelWindow;
                    }else{
                        sumPixelsWindow  += (sumCols[j+halfWindowSize] - sumCols[j-halfWindowSize-1]);
                        sum2PixelsWindow += (sumSqCols[j+halfWindowSize]  - sumSqCols[j-halfWindowSize-1]);
                    }
                }

                localMean   = sumPixelsWindow/(WindowSize*WindowSize);
                localVar    = sum2PixelsWindow/(WindowSize*WindowSize) - localMean*localMean;
                localStd    = sqrt(fabs(localVar));

                if (!sauvola)
                    localValue = localMean+weight*localStd;
                else
                    localValue = localMean*(1 + (localStd/128-1)/6);

                if(pixel(j, y) < localValue)
                    output[y*w + j] = 0;
                else
                    output[y*w + j] = 1;

            }
        }

        for(uint y = 0; y < h; y++){
            for(uint x = 0; x < w; x++){
                if(y < yMin) output[y*w + x] = 1;
                if(y > yMax) output[y*w + x] = 1;
                if(x < xMin) output[y*w + x] = 1;
                if(x > xMax) output[y*w + x] = 1;
            }
        }
        delete [] sumCols;
        delete [] sumSqCols;
        return result;
}

QIPBlackAndWhiteImage QIPGrayscaleImage::otsuBinarize() const
{
    QIPBlackAndWhiteImage result(w, h);
    quint8 threshold = otsuThreshold();
    for (uint y = 0; y < h; y ++) {
        quint8 * line = scanLine(y);
        quint8 * lineOut = result.scanLine(y);
        for (uint x = 0; x < w; x++) {
            if (line[x] > threshold) lineOut[x] = 1;
            else lineOut[x] = 0;
        }
    }
    return result;
}

QIPBlackAndWhiteImage QIPGrayscaleImage::gatosBinarize() const
{
    QIPBlackAndWhiteImage result(w, h);
    memcpy(result.data.data(), data.data(), w*h);
    quint8 * d  = result.data.data();
    qr_binarize(d, w, h);
    for (int y = 0; y < h; y++) {
        quint8 * line = result.scanLine(y);
        for (int x = 0; x < w; x++)
            if (line[x] == 0) line[x] = 1;
            else line[x] = 0;

    }

    return result;
}

QIPBlackAndWhiteImage QIPGrayscaleImage::maxEntropyBinarize() const
{
    QIPBlackAndWhiteImage result(w, h);
    quint8 threshold = maxEntropyThreshold();
    for (uint y = 0; y < h; y ++) {
        quint8 * line = scanLine(y);
        quint8 * lineOut = result.scanLine(y);
        for (uint x = 0; x < w; x++) {
            if (line[x] >= threshold) lineOut[x] = 1;
            else lineOut[x] = 0;
        }
    }
    return result;
}


QIPBlackAndWhiteImage QIPGrayscaleImage::bradleyBinarize() const
{
    const  uint windowSize = 41;
    const qreal pixelBrightnessDifferenceLimit = 0.15;
    QIPBlackAndWhiteImage result(w, h);
    quint8 * resultData = result.data.data();
    memcpy(resultData, data.data(), w*h);

    uint * intImage = new uint[w*h];
    integralImage(w, h, intImage);
    int halfWindowSize = windowSize / 2;
    qreal avgBrightnessPart = 1.0f - pixelBrightnessDifferenceLimit;
    for (int y = 0; y < h; y++) {
        int y1 = y - halfWindowSize > 0 ? y - halfWindowSize : 0;
        int y2 = y + halfWindowSize > h-1 ? h-1 : y + halfWindowSize;
        for (int x = 0; x < w; x++) {
            int x1 = x - halfWindowSize < 0 ? 0 : x - halfWindowSize;
            int x2 = x + halfWindowSize < w - 1 ? x + halfWindowSize : w - 1;
            resultData[x + y*w] = resultData[x + y*w] < ( intImage[y2*w + x2] + intImage[y1*w + x1] - intImage[y1*w + x2] - intImage[y2*w + x1])/((x2-x1)*(y2-y1))*avgBrightnessPart ? 0 : 1;
        }
    }
    delete [] intImage;
    return result;
}

QIPBlackAndWhiteImage QIPGrayscaleImage::iterativeBinarize() const
{
    QIPBlackAndWhiteImage result(w, h);
    quint8 threshold = CalculateIterativeThreshold();
    for (uint y = 0; y < h; y++) {
        quint8 * line = scanLine(y);
        quint8 * lineOut = result.scanLine(y);
        for (uint x = 0; x < w; x++)
            lineOut[x] = line[x] < threshold ? 0 : 1;
    }
    return result;
}

QIPBlackAndWhiteImage QIPGrayscaleImage::bernsenBinarize() const
{
    const uint regSize = 7;
    const quint8 contrastLimit = 48;
    const quint8 confused = 1;
    QIPBlackAndWhiteImage result(w, h);
    for (uint y = 0; y < h; y++) {
        const quint8 * line = scanLine(y);
        quint8 * lineOut = result.scanLine(y);
        int istart = y == 0 ? 0 : y - 1;
        int istop = y + regSize >= h ? h : y + regSize - 1;
        quint8 minimum = 255;
        quint8 maximum = 0;
        int maxX = -1;
        int minX = -1;
        for (uint x = 0; x < w; x++) {
            int jstart = x == 0 ? 0 : x-1;
            int jstop = x+regSize >= w ? w : x + regSize - 1;
            if ((maxX < jstart)||(minX < jstart)) {
                minimum = 255;
                maximum = 0;
                 for (uint i = istart; i < istop; i++) {
                    const quint8 *  lineK = scanLine(i);
                    for (uint j = jstart; j < jstop; j++) {
                        if (minimum > lineK[j]) {
                            minimum = lineK[j];
                            minX = j;
                        }
                        if (maximum < lineK[j]) {
                            maximum = lineK[j];
                            maxX = j;
                        }

                    }
                }
            } else {
                for (int i = istart; i < istop; i++) {
                    if (minimum > scanLine(i)[jstop-1]) {
                        minimum = scanLine(i)[jstop-1];
                        minX = jstop-1;
                    }
                    if (maximum < scanLine(i)[jstop-1]) {
                        maximum = scanLine(i)[jstop-1];
                        maxX = jstop-1;
                    }
                }
            }
            quint8 c = maximum - minimum;
            quint8 value;
            if (c < contrastLimit)
                value = confused;
            else {
                uint p = (maximum + minimum) / 2;
                value = line[x] >= p ? 1 : 0;
            }
            lineOut[x] = value;
        }
    }
    return result;
}



quint8 QIPGrayscaleImage::CalculateIterativeThreshold() const
{
    quint32 distribution[256] = {0};
    for (uint y = 0; y < h; y++) {
        quint8 * line = scanLine(y);
        for (uint x = 0; x < w; x++)
            distribution[line[x]]++;
    }
    quint32 integralHist[256];
    quint32 integralDistribution[256];
    integralHist[0] = 0;
    integralDistribution[0] = distribution[0];
    for (int i = 1; i < 256; i++) {
        integralHist[i] = integralHist[i-1] + distribution[i]*i;
        integralDistribution[i] = integralDistribution[i-1] + distribution[i];
    }

    quint8 oldThreshold = 0;
    quint8 newThreshold = 128;
    while (abs(newThreshold - oldThreshold) > 0) {
        uint meanWhite;
        uint meanBlack;
        uint sumBlack = integralHist[newThreshold];
        uint numBlack = integralDistribution[newThreshold];
        uint sumWhite = integralHist[255] - integralHist[newThreshold];
        uint numWhite = integralDistribution[255] - integralDistribution[newThreshold];

        /*for (int i = 0; i < 256; i++)
            if (i < newThreshold) {
                sumBlack += valueHist[i];
                numBlack += distribution[i];
            } else {
                sumWhite += valueHist[i];
                numWhite += distribution[i];
            }*/
        meanBlack = sumBlack != 0 ? sumBlack/numBlack : 0;
        meanWhite = sumWhite != 0 ? sumWhite/numWhite : 0;
        oldThreshold = newThreshold;
        newThreshold = (meanBlack + meanWhite)/2;
    }
    return newThreshold;
}

void QIPGrayscaleImage::integralImage(uint w, uint h, uint *image) const
{
#ifndef IPRIT_MULTITHREADING
    IntRect r1 = {0,0,w,h};
    copyInternal(r1, image);
#endif
#ifdef IPRIT_MULTITHREADING
    IntRect r1 = {0,0,w,h/2};
    IntRect r2 = {0,h-h/2,w,h};
    QFuture<void> future1 = QtConcurrent::run(this, &QIPGrayscaleImage::copyInternal,r1,image);
    QFuture<void> future2 = QtConcurrent::run(this, &QIPGrayscaleImage::copyInternal,r2, image);
    future1.waitForFinished();
    future2.waitForFinished();
#endif

    for (uint x = 1; x < w; x++)
        image[x] += image[x-1];
    for (uint y = 1; y < h; y++)
        image[y*w] += image[(y-1)*w];
    for (uint y = 1; y < h; y++) {
        for (uint x = 1; x < w; x++)
            image[y*w+x] = image[y*w+x] + image[y*w+x-1] + image[(y-1)*w+x] - image[(y-1)*w+x-1];
    }
}

inline quint8 maxOfThree(quint8 v1, quint8 v2, quint8 v3)
{
    quint8 max = v1;
    if (v2 > max) max = v2;
    if (v3 > max) max = v3;
    return max;
}

inline quint8 minOfThree(quint8 v1, quint8 v2, quint8 v3)
{
    quint8 min = v1;
    if (v2 < min) min = v2;
    if (v3 < min) min = v3;
    return min;
}


void QIPGrayscaleImage::toGrayscaleMinMax(const QImage &input)
{
#ifndef IPRIT_MULTITHREADING
    IntRect r = {0,0,input.width(),input.height()};
    toGrayscaleMinMaxInternal(input,r);
#endif
#ifdef IPRIT_MULTITHREADING
    IntRect r1 = {0,0,input.width(),input.height()/2};
    IntRect r2 = {0,input.height()/2,input.width(),input.height()};
    QFuture<void> future1 = QtConcurrent::run(this, &QIPGrayscaleImage::toGrayscaleMinMaxInternal,input,r1);
    QFuture<void> future2 = QtConcurrent::run(this, &QIPGrayscaleImage::toGrayscaleMinMaxInternal,input,r2);
    future1.waitForFinished();
    future2.waitForFinished();
#endif

}

void QIPGrayscaleImage::toGrayscaleMinOrMax(const QImage &input, bool min)
{
#ifndef IPRIT_MULTITHREADING
    IntRect r = {0,0,input.width(),input.height()};
    toGrayscaleMinOrMaxInternal(input,r, min);
#endif
#ifdef IPRIT_MULTITHREADING
    IntRect r1 = {0,0,input.width(),input.height()/2};
    IntRect r2 = {0,input.height()/2,input.width(),input.height()};
    QFuture<void> future1 = QtConcurrent::run(this, &QIPGrayscaleImage::toGrayscaleMinOrMaxInternal,input,r1, min);
    QFuture<void> future2 = QtConcurrent::run(this, &QIPGrayscaleImage::toGrayscaleMinOrMaxInternal,input,r2, min);
    future1.waitForFinished();
    future2.waitForFinished();
#endif

}

void QIPGrayscaleImage::toGrayscaleMinOrMaxInternal(const QImage &input, const IntRect &rect, bool min)
{
    for (int y = rect.y1; y < rect.y2; y++) {
        QRgb * lineIn = (QRgb *)input.scanLine(y);
        quint8 * lineOut = scanLine(y);
        for(int x = rect.x1; x < rect.x2; x++) {
            QRgb cur = lineIn[x];
            lineOut[x] = min ? minOfThree(qRed(cur), qGreen(cur), qBlue(cur)) : maxOfThree(qRed(cur), qGreen(cur), qBlue(cur));
       }
    }
}

void QIPGrayscaleImage::toGrayscaleMinMaxInternal(const QImage &input, const IntRect &rect)
{
    for (int y = rect.y1; y < rect.y2; y++) {
        QRgb * lineIn = (QRgb *)input.scanLine(y);
        quint8 * lineOut = scanLine(y);
        for(int x = rect.x1; x < rect.x2; x++) {
            QRgb cur = lineIn[x];
            lineOut[x] = (minOfThree(qRed(cur), qGreen(cur), qBlue(cur)) + maxOfThree(qRed(cur), qGreen(cur), qBlue(cur))) >> 1;
       }
    }

}

double safeLog(double x)
{
    if (x > 0)
        return log(x);
    return 0;
}

void QIPGrayscaleImage::toGrayScaleByEntropyChannel(const QImage &input, bool maxEntropy)
{
    qreal rHist[256];
    qreal gHist[256];
    qreal bHist[256];
    quint32 rChannel[256] = {0};
    quint32 gChannel[256] = {0};
    quint32 bChannel[256] = {0};
    int width = input.width();
    int  height = input.height();
    if (width*height == 0) return;
    for (int y = 0; y < height; y++) {
        QRgb * lineIn = (QRgb *)input.scanLine(y);
        for(int x = 0; x < width; x++) {
            rChannel[qRed(lineIn[x])]++;
            gChannel[qGreen(lineIn[x])]++;
            bChannel[qBlue(lineIn[x])]++;
        }
    }
    qreal sizeI = 1.0/(width*height);
    for (int i = 0; i < 256; i++) {
        rHist[i] = rChannel[i]*sizeI;
        rHist[i] *= (-safeLog(rHist[i]));
        gHist[i] = gChannel[i]*sizeI;
        gHist[i] *= (-safeLog(gHist[i]));
        bHist[i] = bChannel[i]*sizeI;
        bHist[i] *= (-safeLog(bHist[i]));
    }
    qreal rEntropy = 0.0;
    qreal gEntropy = 0.0;
    qreal bEntropy = 0.0;
    for (int i = 0; i < 256; i++) {
        rEntropy += rHist[i];
        gEntropy += gHist[i];
        bEntropy += bHist[i];
    }
    int (*chan)(QRgb);
    if (maxEntropy) {
        qreal max = rEntropy;
        chan = qRed;
        if (gEntropy > max) {
            max = gEntropy;
            chan = qGreen;
        }
        if (bEntropy > max) {
            chan = qBlue;
        }

    } else {
        qreal min = gEntropy;
        chan = qGreen;
        if (rEntropy < min) {
            min = rEntropy;
            chan = qRed;
        }
        if (bEntropy < min) {
            chan = qBlue;
        }
    }
    for (int y = 0; y < height; y++) {
        QRgb * lineIn = (QRgb *)input.scanLine(y);
        quint8 * lineOut = scanLine(y);
        for(int x = 0; x < width; x++)
            lineOut[x] = chan(lineIn[x]);

    }

}

quint8 maxTh(qreal * hist, qreal * ihist, int start, int stop)
{
    qreal ut = 0;
    for (int i = start; i < stop; i++)
        ut+= ihist[i];
    int maxK=0;
    int maxSigmaK=0;

    qreal wk = 0;
    qreal uk = 0;
    for (int k = start; k < stop; ++k) {
        if (hist[k] == 0)
            continue;
        wk += hist[k];
        uk += ihist[k];

        qreal sigmaK = 0;
        if ((wk !=0) && (wk!=1))
            sigmaK  = ((ut*wk - uk)*(ut*wk - uk))/(wk*(1-wk));
        if (sigmaK > maxSigmaK) {
            maxK = k;
            maxSigmaK = sigmaK;
        }
    }
    return maxK;
}

QIPGrayscaleImage::QIPGrayscaleImage() : data(0)
{
    w = 0;
    h = 0;
}
