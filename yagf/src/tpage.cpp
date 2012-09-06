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

#include "tpage.h"
#include "imagebooster.h"
#include "settings.h"
#include "ccbuilder.h"
#include "CCAnalysis.h"
#include "PageAnalysis.h"
#include "analysis.h"
#include <QImageReader>
#include <QImageWriter>
#include <QSize>
#include <QRect>
#include <QFile>
#include <QApplication>
#include <cmath>

Page::Page(const int pid, QObject *parent) :
    QObject(parent), selectedBlock(0,0,0,0)
{
    imageLoaded = false;
    loadedBefore = false;
    ccbuilder = NULL;
    rotation = 0;
    deskewed = false;
    preprocessed = false;
    mFileName.clear();
    this->pid = pid;
}

Page::~Page()
{
    delete ccbuilder;
}

bool Page::loadFile(QString fileName, bool loadIntoView)
{
    // TODO: перенести в TPages.
    /*if ((fileName != "") && (sideBar->getFileNames().contains(fileName))) {
        sideBar->select(fileName);
        sideBar->clearBlocks();
        for (int i = 0; i < graphicsInput->blocksCount(); i++)
            sideBar->addBlock(graphicsInput->getBlockRectByIndex(i).toRect());
    }*/
    if (fileName == "") {

        if(mFileName.isEmpty()) return false;
        fileName = mFileName;
    }

    rotation = 0;
    crop1.setX(0);
    crop1.setY(0);
    crop1.setWidth(0);
    crop1.setHeight(0);
    scale = 0.5;
    QImageReader ir(fileName);
    ir.setScaledSize(QSize(ir.size().width()/2, ir.size().height()/2));
    img2 = ir.read();
    imageLoaded = !img2.isNull();
    if (!imageLoaded)
        return false;
    if (ccbuilder){
        delete ccbuilder;
        ccbuilder = 0;
    }
    if (!loadedBefore) {
        settings = Settings::instance();
        if (settings->getCropLoaded()) {
            ccbuilder = new CCBuilder(img2);
            ccbuilder->setGeneralBrightness(360);
            ccbuilder->setMaximumColorComponent(100);
            QRect r = ccbuilder->crop();
            crop1 = r;
            img2 = img2.copy(crop1);
            delete ccbuilder;
            ccbuilder = 0;
          //  ccbuilder = new CCBuilder(img2);

        }
        //ccbuilder->labelCCs();
        //mGeneralBrightness = ccbuilder->getGB();
    } else {
        if (settings->getCropLoaded())
            img2 = img2.copy(crop1);
    }
    rotateImageInternal(img2, rotation);
    scaleImages();
    mFileName = fileName;
    loadedBefore = true;

    return true;
}

QPixmap Page::displayPixmap()
{
    return QPixmap::fromImage(currentImage());
}

QImage Page::thumbnail()
{
    return img16;
}

bool Page::makeLarger()
{
    if (scale >= 0.5) return false;
    if (scale < 0.2) {
        scale = 0.2;
        return true;
    }
    if (scale < 0.25) {
        scale = 0.25;
        return true;
    }
    if (scale < 0.5)
        scale = 0.5;
    return true;
}

bool Page::makeSmaller()
{
    if (scale <= 0.125) {
        return false;
    }
    if (scale > 0.25) {
        scale = 0.25;
        return true;
    }
    if (scale > 0.2) {
        scale = 0.2;
        return true;
    }
    if (scale > 0.125)
        scale = 0.125;
    return true;
}

void Page::rotate(qreal angle)
{
    rotateImageInternal(img2, angle);


    rotation += angle;
    scaleImages();
    clearBlocks();
}

void Page::unload()
{
    if (ccbuilder){
        delete ccbuilder;
        ccbuilder = 0;
    }
    img2 = QImage(0,0,QImage::Format_ARGB32);
    img4 = QImage(0,0,QImage::Format_ARGB32);
    img6 = QImage(0,0,QImage::Format_ARGB32);
    img8 = QImage(0,0,QImage::Format_ARGB32);
    img16 = QImage(0,0,QImage::Format_ARGB32);
    imageLoaded = false;
}

inline bool qrects_equal(QRect &r1, QRect &r2)
{
    if (abs(r1.x() - r2.x()) > 2)
        return false;
    if (abs(r1.y() - r2.y()) > 2)
        return false;
    if (abs(r1.width() - r2.width()) > 2)
        return false;
    if (abs(r1.height() - r2.height()) > 2)
        return false;
    return true;
}

void Page::addBlock(Block block)
{
    QRect r = block;
    normalizeRect(r);
    bool add = true;
    foreach (Block b, blocks) {
        QRect r1 = b;
        if (r == r1) {
            add = false;
            break;
        }
    }
    if (add) {
        block.setRect(r.x(), r.y(), r.width(), r.height());
        blocks.append(block);
    }
    sortBlocksInternal();
    renumberBlocks();
}

void Page::deleteBlock(const Block &b)
{
    blocks.removeOne(b);
    sortBlocksInternal();
    renumberBlocks();
}

void Page::deleteBlock(const QRect &r)
{
    QRect rx = r;
    normalizeRect(rx);
    foreach (Block b, blocks) {
        QRect r1 = b;
        if (qrects_equal(rx, r1)) {
            blocks.removeAll(b);
            break;
        }
    }
    sortBlocksInternal();
    renumberBlocks();
}

Block Page::getBlock(const QRect &r)
{
    QRect rn =r;
    normalizeRect(rn);
    foreach (Block b, blocks) {
        QRect r1 = b;
        if (qrects_equal(rn,r1)) {
            scaleRect(b);
            return b;
        }
    }
    return Block(0,0,0,0);
}

Block Page::getBlock(int index)
{
    Block b = blocks.at(index);
    scaleRect(b);
    return b;
}

int Page::blockCount()
{
    return blocks.count();
}

void Page::clearBlocks()
{
    blocks.clear();
}

void Page::savePageForRecognition(const QString &fileName)
{
    QImageReader ir(mFileName);
    QImage image = ir.read();
    applyTransforms(image, 1);
    image.save(fileName, "BMP");
}

bool Page::savePageAsImage(const QString &fileName, const QString &format)
{
    QImageReader ir(mFileName);
    QImage image = ir.read();
    //applyTransforms(image, 1);
    return image.save(fileName, format.toAscii().data());
}

void Page::saveRawBlockForRecognition(QRect r, const QString &fileName)
{
    normalizeRect(r);
    saveBlockForRecognition(r, fileName, "BMP");
}

void Page::saveBlockForRecognition(QRect r, const QString &fileName, const QString &format)
{
    int oldw = r.width();
    int oldh = r.height();
    r.setX(r.x()*2);
    r.setY(r.y()*2);
    r.setWidth(oldw*2);
    r.setHeight(oldh*2);
    QImageReader ir(mFileName);
    QImage image = ir.read();
    applyTransforms(image, 1);
    image.copy(r).save(fileName, format.toAscii().data());
}

void Page::saveBlockForRecognition(int index, const QString &fileName)
{
    saveBlockForRecognition(blocks.at(index), fileName, "BMP");
}

void Page::selectBlock(const QRect &r)
{
    QRect rn =r;
    normalizeRect(rn);
    foreach (Block b, blocks) {
        QRect r1 = b;
        if (rn == r1) {
            selectedBlock = b;
            break;
        }
    }
}

Block Page::getSelectedBlock()
{
    return selectedBlock;
}

void Page::deskew(bool recreateCB)
{
    if (deskewed) return;
    if (imageLoaded) {
        prepareCCBuilder();
        CCAnalysis * an = new CCAnalysis(ccbuilder);
        if (an->analize()) {
            QImage timg = tryRotate(img2, -atan(an->getK())*360/6.283);
            CCBuilder * cb2 = new CCBuilder(timg);
            cb2->setGeneralBrightness(360);
            cb2->setMaximumColorComponent(100);
            cb2->labelCCs();
            CCAnalysis * an2 = new CCAnalysis(cb2);
            an2->analize();
            qreal angle = -atan(an2->getK())*360/6.283;
            delete an2;
            delete cb2;
            if (abs(angle*10) >= abs(5))
                angle += (-atan(an->getK())*360/6.283);
            else
                angle = -atan(an->getK())*360/6.283;
            rotate(angle);
            deskewed = true;
            delete ccbuilder;
            ccbuilder = 0;
            if (recreateCB)
                prepareCCBuilder();
        }
        delete an;
    }
}

void Page::rotate90CW()
{
    rotate(90);
}

void Page::rotate90CCW()
{
    rotate(-90);
}

void Page::rotate180()
{
    rotate(180);
}

void Page::blockAllText()
{
    prepareCCBuilder();
    clearBlocks();
    BlockSplitter bs;
    bs.setImage(img2, rotation, scale);
    QRect r = bs.getRootBlock(currentImage());
    addBlock(r);
}

QList<Rect> Page::splitInternal() {
    clearBlocks();
    BlockSplitter bs;
    //rotation  = 0;
    bs.setImage(img2, 0, 0.5);// sideBar->getScale());
    bs.splitBlocks();
    return bs.getBlocks();
}

void Page::prepareCCBuilder()
{
    if (!ccbuilder) {
        ccbuilder = new CCBuilder(img2);
        ccbuilder->setGeneralBrightness(360);
        ccbuilder->setMaximumColorComponent(100);
        ccbuilder->labelCCs();
    }
}

bool Page::splitPage(bool preprocess)
{
    QList<Rect> blocks;
    prepareCCBuilder();
    if (preprocess) {
        QString fn =Settings::instance()->workingDir() + QString::fromUtf8("tmp-%1.bmp").arg((quint64)img2.data_ptr());
        saveTmpPage(fn, !preprocessed, !preprocessed, false);
        loadedBefore = false;
        loadFile(fn);
        blocks = splitInternal();
    /*if (blocks.count() == 0) {
        deskew();
        fn =Settings::instance()->workingDir() + QString::fromUtf8("tmp-%1.bmp").arg((quint64)img2.data_ptr());
        saveTmpPage(fn, true, false);
        loadedBefore = false;
        loadFile(fn);
        blocks = splitInternal();
    }*/
        if (!preprocessed) {
            fn =Settings::instance()->workingDir() + QString::fromUtf8("tmp-%1.bmp").arg((quint64)img2.data_ptr());
            if (blocks.count() == 0) {
                saveTmpPage(fn, false, false, true, 2);
            } else {
                saveTmpPage(fn, false, false, false, 7, 5);
            }
            loadedBefore = false;
            loadFile(fn);
            blocks = splitInternal();
        }
        preprocessed = true;
    } else {
        deskew();
        blocks = splitInternal();
    }
    qreal sf = 2.0*scale;
    foreach (Rect block, blocks) {
        QRect r;
        block.x1 *=sf;
        block.y1 *=sf;
        block.x2 *= sf;
        block.y2 *=sf;

        r.setX(block.x1);
        r.setY(block.y1);
        r.setWidth(block.x2 - block.x1);
        r.setHeight(block.y2 - block.y1);
        addBlock(r);
    }
    return blocks.count() != 0;
}

QString Page::fileName()
{
    return mFileName;
}

int Page::pageID()
{
    return pid;
}

void Page::sortBlocksInternal()
{
    sortBlocks(blocks);
}

bool Page::isDeskewed()
{
    return deskewed;
}

bool Page::isPreprocessed()
{
    return preprocessed;
}

qreal Page::getRotation()
{
    return rotation;
}

void Page::setDeskewed(bool value)
{
    deskewed = value;
}

void Page::setPreprocessed(bool value)
{
    preprocessed = value;
}


void Page::applyTransforms(QImage &image, qreal scale)
{
    scale = scale*2;
    QRect crop;
    crop.setX(crop1.x()*scale);
    crop.setY(crop1.y()*scale);
    crop.setWidth(crop1.width()*scale);
    crop.setHeight(crop1.height()*scale);
    image = image.copy(crop);
    rotateImageInternal(image, rotation);
}

void Page::rotateImageInternal(QImage &image, qreal angle)
{
    qreal x = image.width() / 2;
    qreal y = image.height() / 2;
    image = image.transformed(QTransform().translate(-x, -y).rotate(angle).translate(x, y), Qt::SmoothTransformation);
}

void Page::scaleImages()
{
    img4 = img2.scaledToWidth(img2.width() / 2);
    img6 = img2.scaledToWidth(img2.width() / 2.5);
    img8 = img4.scaledToWidth(img4.width() / 2);
    img16 = img8.scaledToWidth(img8.width() / 2);
}

void Page::normalizeRect(QRect &rect)
{
    qreal oldw = rect.width();
    qreal oldh = rect.height();
    rect.setX(rect.x()*0.5/scale);
    rect.setY(rect.y()*0.5/scale);
    rect.setWidth(oldw*0.5/scale);
    rect.setHeight(oldh*0.5/scale);
}

void Page::scaleRect(QRect &rect)
{
    qreal oldw = rect.width();
    qreal oldh = rect.height();
    rect.setX(rect.x()/0.5*scale);
    rect.setY(rect.y()/0.5*scale);
    rect.setWidth(oldw/0.5*scale);
    rect.setHeight(oldh/0.5*scale);
}

QImage Page::tryRotate(QImage image, qreal angle)
{
    qreal x = image.width() / 2;
    qreal y = image.height() / 2;
    return image.transformed(QTransform().translate(-x, -y).rotate(angle).translate(x, y), Qt::SmoothTransformation);
}

QImage Page::currentImage()
{
    if (scale <= 0.125)
        return img8;
    if (scale <= 0.2)
        return img6;
    if (scale <= 0.25)
        return img4;
    return img2;
}

void Page::saveTmpPage(const QString &fileName, bool cc, bool boost, bool brighten, int p, int q)
{
    QImageReader ir(mFileName);
    QImage image = ir.read().convertToFormat(QImage::Format_ARGB32);
    if (image.isNull())
        return;
    ImageBooster booster;
    if (boost)
        booster.boost(&image);
    if (brighten)
        booster.brighten(&image,p,q);
    if (cc) {
        deskewed = false;
        deskew(false);
    }
    //    booster.flatten(&image);
    applyTransforms(image, 1);
    image.save(fileName, "BMP");
}



void Page::renumberBlocks()
{
    for (int i = 0; i < blocks.count(); i++) {
        blocks[i].setBlockNumber(i+1);
    }
}
