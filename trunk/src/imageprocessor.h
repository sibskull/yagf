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

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <binarize.h>
#include <QObject>
#include <QImage>


const  int  IPINITIAL = 0;
const  int  IPBINARIZED = 1;
const int IPCROPPED = 2;
const int IPREBINARIZED = 4;

class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor(QObject *parent = 0);
    void start(const QImage &input);
    void binarize();
    QRect crop();
    void rebinarize(quint32 upper, quint32 lower);
    void nomalizeBackgroud();
    QImage finalize();
signals:
    
public slots:
private:
    int state;
    Binarize bin;
    QImage output;
    QImage img;
    QImage mask;
    QImage gray;
    quint32 m_middleBG;
    quint32 m_treshold;
private:
    void toGrayScale(const QImage &input);
    int cropTop(const QImage &input);
    int cropBottom(const QImage &input);
    int cropLeft(const QImage &input);
    int cropRight(const QImage &input);

    uchar getMediumBG(QRgb * line, uint length, uchar prev);

    
};

#endif // IMAGEPROCESSOR_H
