#include "binarize.h"
#include <QImage>

Binarize::Binarize(QObject *parent) :
    QObject(parent)
{
}

void Binarize::otsu(const QImage &input, QImage &output, int left, int top, int width, int height)
{
    if (!width)
        width = input.width();
    if (!height)
        height = input.height();
    float hist[256]={0.0F};
    output = input;
    for (int y = top; y < height; y++) {
       QRgb * lineIn = (QRgb *)input.scanLine(y);
       QRgb * lineOut = (QRgb *)output.scanLine(y);
       for(int x = left; x < width; x++) {
           QRgb cur = lineIn[x];
           int r = qRed(cur)/3;
           int g = qGreen(cur)/3;
           int b = qBlue(cur)/3;
           int grayLevel = r + g + b;
           lineOut[x] = qRgb(r, g, b);
           hist[grayLevel]+=1;
       }
    }

    int size = width*height;


    for (int i = 0; i < 256; i++)
       hist[i]/=size;


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
        QRgb * lineOut = (QRgb *)output.scanLine(y);
        for (int x = left; x < width; x++) {
           QRgb cur = lineOut[x];
           int graylevel = qRed(cur)+qGreen(cur)+qBlue(cur);
           lineOut[x] = graylevel < maxK ? 0xFF000000 : 0xFFFFFFFF;
       }
    }
}
