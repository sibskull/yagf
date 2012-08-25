/*
    YAGF - cuneiform and tesseract OCR graphical front-end
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

#ifndef TPAGE_H
#define TPAGE_H

#include "tblock.h"
#include "analysis.h"
#include <QObject>
#include <QRect>
#include <QImage>
#include <QPixmap>

class CCBuilder;
class Settings;

class TPage : public QObject
{
    Q_OBJECT
public:
    explicit TPage(const int pid, QObject *parent = 0);
    ~TPage();
    bool loadFile(QString fileName, bool loadIntoView = true);
    QPixmap displayPixmap();
    QImage thumbnail();
    bool makeLarger();
    bool makeSmaller();
    void rotate(qreal angle);
    void unload();
    void addBlock(TBlock block);
    void deleteBlock(const TBlock &b);
    void deleteBlock(const QRect &r);
    TBlock getBlock(const QRect &r);
    TBlock getBlock(int index);
    int blockCount();
    void clearBlocks();
    void savePageForRecognition(const QString &fileName);
    bool savePageAsImage(const QString &fileName, const QString &format);
    void saveRawBlockForRecognition(QRect r, const QString &fileName);
    void saveBlockForRecognition(QRect r, const QString &fileName, const QString &format);
    void saveBlockForRecognition(int index, const QString &fileName);
    void selectBlock(const QRect &r);
    TBlock getSelectedBlock();
    void deskew();
    void rotate90CW();
    void rotate90CCW();
    void rotate180();
    void blockAllText();
    void splitPage(bool preprocess);
    QString fileName();
    int pageID();
    void sortBlocksInternal();
signals:
    void refreshView();
public slots:
private:
    void renumberBlocks();
    void applyTransforms(QImage &image, qreal scale);
    void rotateImageInternal(QImage &image, qreal angle);
    void scaleImages();
    void normalizeRect(QRect &rect);
    void scaleRect(QRect &rect);
    QImage tryRotate(QImage image, qreal angle);
    QImage currentImage();
    void saveTmpPage(const QString &fileName, bool cc,bool boost, bool brighten, int p = 1, int q =1);
    QList<Rect> splitInternal();
    void prepareCCBuilder();
private:
    qreal scale;
    qreal rotation;
    int mGeneralBrightness;
    QRect crop1;
    QRect crop2;
    bool deskewed;
    QImage img;
    QImage img2;
    QImage img4;
    QImage img6;
    QImage img8;
    QImage img16;
    TBlocks blocks;
    bool imageLoaded;
    bool loadedBefore;
    bool preprocessed;
    QString mFileName;
    CCBuilder * ccbuilder;
    Settings * settings;
    int blockPointer;
    int pid;
    TBlock selectedBlock;
};

#endif // TPAGE_H
