/*
    YAGF - cuneiform and tesseract OCR graphical front-ends
    Copyright (C) 2009-2012 Andrei Borovsky <anb@symmetrica.net>

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

#ifndef IMAGEBOOSTER_H
#define IMAGEBOOSTER_H

#include <QObject>
#include <QImage>

class ImageBooster : public QObject
{
    Q_OBJECT
public:
    explicit ImageBooster(QObject *parent = 0);
    ~ImageBooster();
    void boost(QImage * image);
    void brighten(QImage * image, int p, int q);
    void flatten(QImage * image);
    void sharpen(QImage * image);
signals:

public slots:
private:
    void buildProfile(QImage *image);
    void sharpenEdges(quint32 * r, quint32 * g, quint32 * b, quint32 * br, int w);
    void analyseStripe(QRgb *line, int i, int w, qreal &med, qreal &med1, int &start);
private:
    quint32 * profile;
};

#endif // IMAGEBOOSTER_H
