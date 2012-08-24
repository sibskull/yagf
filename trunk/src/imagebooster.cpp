#include "imagebooster.h"
#include <QColor>

ImageBooster::ImageBooster(QObject *parent) :
    QObject(parent)
{
    profile = new quint32[16];
    for (int i =0; i  < 16; i++)
        profile[i] = 0;
}

ImageBooster::~ImageBooster()
{
    delete[] profile;
}

void ImageBooster::boost(QImage *image)
{
    if (!image)
        return;
    int h = image->height();
    int w = image->width();
    if (h*w == 0) return;
    buildProfile(image);
    int imax = 0;
    quint32 pmax = 0;
    for (int i = 0; i < 16; i++) {
        if (profile[i] > pmax) {
            imax = i;
            pmax = profile[i];
        }
    }
   // quint32 * rl = new quint32[w];
  //  quint32 * gl = new quint32[w];
  //  quint32 * bl = new quint32[w];
  //  quint32 * brl = new quint32[w];
    if (imax > 10) {
        for(int i = 0; i < h; i++) {
            QRgb * line = (QRgb *) image->scanLine(i);
            for (int j = 0; j < w; j++) {
                int r = qRed(line[j]);
                int g = qGreen(line[j]);
                int b = qBlue(line[j]);
                r = r*8/10;
                g = g*8/10;
                b = b*8/10;
                r = r*r/160;
                g = g*g/160;
                b = b*b/160;
                //r = r*10/7;
                if (r > 255) r = 255;
                //g = g*10/7;
                if (g > 255) g = 255;
                //b = b*10/7;
                if (b > 255) b = 255;
               // rl[j] = r;
               // gl[j] = g;
              //  bl[j] = b;
               // brl[j] = r+ g+ b;
                line[j] = ((255 & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
            }
            //sharpenEdges(rl, gl, bl, brl, w);
        }
    } else {
        for(int i = 0; i < h; i++) {
            QRgb * line = (QRgb *) image->scanLine(i);
            for (int j = 0; j < w; j++) {
                int r = qRed(line[j]);
                int g = qGreen(line[j]);
                int b = qBlue(line[j]);
                r = r*r/160;
                g = g*g/160;
                b = b*b/160;
                r = r*10/9;
                if (r > 255) r = 255;
                g = g*10/9;
                if (g > 255) g = 255;
                b = b*10/9;
                if (b > 255) b = 255;
                //rl[j] = r;
                //gl[j] = g;
               // bl[j] = b;
               // brl[j] = r+ g+ b;
                line[j] = ((255 & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
            }
            //sharpenEdges(rl, gl, bl, brl, w);

        }
    }
    //delete [] gl;
    //delete[] rl;
    //delete[] bl;
    //delete[] brl;
}

void ImageBooster::brighten(QImage *image, int p, int q)
{
    if (!image)
        return;
    int h = image->height();
    int w = image->width();
    if (h*w == 0) return;
    for(int i = 0; i < h; i++) {
        QRgb * line = (QRgb *) image->scanLine(i);
        for (int j = 0; j < w; j++) {
            int r = qRed(line[j]);
            int g = qGreen(line[j]);
            int b = qBlue(line[j]);
            r = r*p/q;
            g = g*p/q;
            b = b*p/q;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            line[j] =  ((255 & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
        }
    }
}

void ImageBooster::flatten(QImage *image)
{
    if (!image)
        return;
    int h = image->height();
    int w = image->width();
    if (h*w == 0) return;
    for (int i = 0; i < h; i++) {
        QRgb * line = (QRgb *) image->scanLine(i);
        int start = 300;
        qreal med1 = 0;
        qreal med = 0;
        analyseStripe(line, i, w, med, med1, start);
        while (start < w) {
            for (int j = start; j < 300; j++) {
                int r = qRed(line[j]);
                int g = qGreen(line[j]);
                int b = qBlue(line[j]);
                r = r*10/5;
                g = g*10/5;
                b = b*10/5;
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                line[j] =  ((255 & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
            }
            med1 = med;
            start+=300;
            analyseStripe(line, i, w, med, med1, start);
        }
    }

}

QImage * ImageBooster::sharpen(QImage * origin){
    QImage * newImage = new QImage(* origin);

    int kernel [3][3]= {{0,-1,0},
                        {-1,5,-1},
                        {0,-1,0}};
    int kernelSize = 3;
    int sumKernel = 1;
    int r,g,b;
    QColor color;

    for(int x=kernelSize/2; x<newImage->width()-(kernelSize/2); x++){
        for(int y=kernelSize/2; y<newImage->height()-(kernelSize/2); y++){

            r = 0;
            g = 0;
            b = 0;

            for(int i = -kernelSize/2; i<= kernelSize/2; i++){
                for(int j = -kernelSize/2; j<= kernelSize/2; j++){
                    color = QColor(origin->pixel(x+i, y+j));
                    r += color.red()*kernel[kernelSize/2+i][kernelSize/2+j];
                    g += color.green()*kernel[kernelSize/2+i][kernelSize/2+j];
                    b += color.blue()*kernel[kernelSize/2+i][kernelSize/2+j];
                }
            }

            r = qBound(0, r/sumKernel, 255);
            g = qBound(0, g/sumKernel, 255);
            b = qBound(0, b/sumKernel, 255);

            newImage->setPixel(x,y, qRgb(r,g,b));

        }
    }
    return newImage;
}

void ImageBooster::buildProfile(QImage * image)
{
    int h = image->height();
    int w = image->width();
    if (h*w == 0) return;
    for(int i = 0; i < h; i++) {
        QRgb * line = (QRgb *) image->scanLine(i);
        for (int j = 0; j < w; j++) {
            int brightness = qRed(line[j])+qGreen(line[j])+qBlue(line[j]);
            brightness /= 48;
            profile[brightness]++;
        }
    }
}

void ImageBooster::sharpenEdges(quint32 *r, quint32 *g, quint32 *b, quint32 *br, int w)
{
    quint32 sp[4];
    quint32 mDelta = 0;
    quint32 counter = 0;
    quint32 * deltas = new quint32[w];
    for (int i = 3; i < w; i++) {
        sp[0] = br[i-3];
        sp[1] = br[i-2];
        sp[2] = br[i-1];
        sp[3] = br[i];
        if ( ( (sp[0]-sp[1])*(sp[1]-sp[2]) > 0)&&((sp[1]-sp[2])*(sp[2]-sp[3]) > 0)) {
            mDelta += abs(sp[0]-sp[3]);
            counter++;
            deltas[i] = mDelta;
        } else deltas[i] = 0;
    }
    if (counter)
        mDelta /= counter;
    else {
        delete[] deltas;
        return;
    }
    int i = 3;
    while (i < w) {
        if (deltas[i] > mDelta) {
            if (br[i-3] < br[i]) {
                r[i-2] = r[i-3];
                g[i-2] = g[i-3];
                b[i-2] = b[i-3];
            } else {
                r[i-1] = r[i];
                g[i-1] = g[i];
                b[i-1] = b[i];
            }
            i++;
        }
        i++;
    }
    delete[] deltas;
}

void ImageBooster::analyseStripe(QRgb * line, int i, int w, qreal &med, qreal &med1, int &start)
{
    if (start < 300) start = 300;
    for (int j = start; j < w - 600; j+=300) {
        int sum = 0;
        for (int k = j; k < j+300; k++) {
            sum += (qRed(line[k]) + qGreen(line[k]) + qBlue(line[k]));
        }
        med = sum/300;
        if ((sum - med1 < 100)&&(sum - med1 > 10)) {
            med = sum;
            start = j;
            return;
        }
        if ((sum - med1 > -100)&&(sum - med1 < -10)) {
            med = sum;
            start = j-300;
            return;
        }

        med1 = sum;


    }
    start = w;

}
