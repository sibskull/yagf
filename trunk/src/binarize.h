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

#ifndef BINARIZE_H
#define BINARIZE_H

#include <QObject>
#include <QRgb>

const QRgb clBlack = qRgb(0,0,0);
const QRgb clWhite = qRgb(255,255,255);

class QImage;

class Binarize : public QObject
{
    Q_OBJECT
public:
    explicit Binarize(QObject *parent = 0);
    int otsu(const QImage &input, quint32 &background, int left = 0, int top = 0, int width = 0, int height = 0);
    int otsuMinimized(const QImage &input, quint32 median, quint32 upperbound, quint32 lowerBound, quint32 &background, int left = 0, int top = 0, int width = 0, int height = 0);
signals:
    
public slots:

private:
    quint32 m_maxSigma;
    uchar m_maxK;
    
};

#endif // BINARIZE_H
