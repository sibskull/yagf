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
#include "settings.h"
#include "ccbuilder.h"
#include "CCAnalysis.h"
#include "PageAnalysis.h"
#include "analysis.h"
#include "core/imageprocessor.h"
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

bool Page::loadFile(QString fileName, int tiled, bool loadIntoView)
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
    if ((ir.size().width() > 7500)||(ir.size().height() > 7500)) {
        ir.setScaledSize(QSize(ir.size().width()/4, ir.size().height()/4));
    } else {
        if ((ir.size().width() > 3800)||(ir.size().height() > 3800))
            ir.setScaledSize(QSize(ir.size().width()/2, ir.size().height()/2));
    }

    img = ir.read();
    imageLoaded = !img.isNull();
    if (!imageLoaded)
        return false;
    if (img.format() != QImage::Format_ARGB32)
        img = img.convertToFormat(QImage::Format_ARGB32);

    if (ccbuilder){
        delete ccbuilder;
        ccbuilder = 0;
    }
    ImageProcessor ip;
    ip.loadImage(img);
    settings = Settings::instance();
    if (settings->getCropLoaded())
        ip.crop();
    img = ip.gsImage();
    //ip.start(img2);
    //ip.tiledBinarize();
    rotateImageInternal(img, rotation);
    ip.loadImage(img);
    ip.binarize();
    img = ip.gsImage();
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
    return img.scaled(img.width()*0.125, img.height()*0.125);
}

bool Page::makeLarger()
{
    if (scale >= 1.0) return false;
    if (scale < 0.2) {
        scale = 0.2;
        return true;
    }
    if (scale < 0.25) {
        scale = 0.25;
        return true;
    }
    if (scale < 0.3) {
        scale = 0.3;
        return true;
    }
    if (scale < 0.5) {
        scale = 0.5;
        return true;
    }
    if (scale < 0.75) {
        scale = 0.75;
        return true;
    }
    if (scale < 1.0)
        scale = 1.0;
    return true;
}

bool Page::makeSmaller()
{
    if (scale <= 0.125) {
        return false;
    }
    if (scale > 0.75) {
        scale = 0.75;
        return true;
    }
    if (scale > 0.5) {
        scale = 0.5;
        return true;
    }
    if (scale > 0.3) {
        scale = 0.3;
        return true;
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
    rotateImageInternal(img, angle);
    rotation += angle;
    clearBlocks();
}

void Page::unload()
{
    if (ccbuilder){
        delete ccbuilder;
        ccbuilder = 0;
    }
    img = QImage(0,0,QImage::Format_ARGB32);
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
    //normalizeRect(r);
    scaleRect(r);
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
    scaleRect(rx);
    //normalizeRect(rx);
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
    scaleRectToScale(rn);
    //normalizeRect(rn);
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
    scaleRectToScale(b);
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

    saveBlockForRecognition(r, fileName, "BMP");
}

void Page::saveBlockForRecognition(QRect r, const QString &fileName, const QString &format)
{
    //QRect rs = scaleRect(r);
    QImage image = img.copy(r);
    //applyTransforms(image, 1);
    image.save(fileName, format.toAscii().data());
}

void Page::saveBlockForRecognition(int index, const QString &fileName)
{
    saveBlockForRecognition(blocks.at(index), fileName, "BMP");
}

void Page::selectBlock(const QRect &r)
{
    QRect rn =r;
    //normalizeRect(rn);
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

bool Page::deskew(bool recreateCB)
{
    if (deskewed) return false;
    if (imageLoaded) {
        prepareCCBuilder();
        CCAnalysis * an = new CCAnalysis(ccbuilder);
        if (an->analize()) {
            QImage timg;
            if ((img.height() > 3800)||(img.width() > 3800))
                return false;
            timg = tryRotate(img, -atan(an->getK())*360/6.283);
            CCBuilder * cb2 = new CCBuilder(timg);
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
            if (abs(angle) < 0.001) {
                deskewed = true;
                return false;
            }
            rotate(angle);
            //ImageProcessor::polishImage(img);
            ImageProcessor::polishImage2(img);
            QString fn = saveTmpPage(false, false);
            loadFile(fn, 1);
            deskewed = true;
            delete ccbuilder;
            ccbuilder = 0;
            if (recreateCB)
                prepareCCBuilder();
        }
        delete an;
    }
    return true;
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
    bs.setImage(img, rotation, scale);
    QRect r = bs.getRootBlock(currentImage());
    addBlock(r);
}

QList<Rect> Page::splitInternal() {
    clearBlocks();
    BlockSplitter bs;
    //rotation  = 0;
    bs.setImage(img, 0, 1.0);// sideBar->getScale());
    bs.splitBlocks();
    return bs.getBlocks();
}

void Page::prepareCCBuilder()
{
    if (!ccbuilder) {
        ccbuilder = new CCBuilder(img);
        ccbuilder->labelCCs();
    }
}

bool Page::splitPage(bool preprocess)
{
    QList<Rect> blocks;
    prepareCCBuilder();
    if (preprocess) {
        QString fn = saveTmpPage(!preprocessed, !preprocessed);
        loadedBefore = false;
        loadFile(fn, 1);
        blocks = splitInternal();
    /*if (blocks.count() == 0) {
        deskew();
        fn =Settings::instance()->workingDir() + QString::fromUtf8("tmp-%1.bmp").arg((quint64)img2.data_ptr());
        saveTmpPage(fn, true, false);
        loadedBefore = false;
        loadFile(fn);
        blocks = splitInternal();
    }*/
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

bool Page::textHorizontal()
{
    return ImageProcessor::isTextHorizontal(img);
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

QRect Page::scaleRect(QRect &rect)
{
    qreal iscale = 1./scale;
    qreal oldw = rect.width();
    qreal oldh = rect.height();
    rect.setX(rect.x()*iscale);
    rect.setY(rect.y()*iscale);
    rect.setWidth(oldw*iscale);
    rect.setHeight(oldh*iscale);
    return rect;
}

QRect Page::scaleRectToScale(QRect &rect)
{
    qreal oldw = rect.width();
    qreal oldh = rect.height();
    rect.setX(rect.x()*scale);
    rect.setY(rect.y()*scale);
    rect.setWidth(oldw*scale);
    rect.setHeight(oldh*scale);
    return rect;
}

QImage Page::tryRotate(QImage image, qreal angle)
{
    qreal x = image.width() / 2;
    qreal y = image.height() / 2;
    return image.transformed(QTransform().translate(-x, -y).rotate(angle).translate(x, y), Qt::SmoothTransformation);
}

QImage Page::currentImage()
{
    return img.scaled(img.width()*scale, img.height()*scale);
}

QString Page::saveTmpPage(bool cc, bool binarize)
{
    QString fileName =Settings::instance()->workingDir() + QString::fromUtf8("tmp-%1.bmp").arg((quint64)img.data_ptr() - (((quint64)&cc)>>1));
    QImageReader ir(mFileName);
    QImage image = ir.read().convertToFormat(QImage::Format_ARGB32);
    if (image.isNull())
        return fileName;
    if (binarize) {
        //ImageProcessor ip;
        //ip.start(image);
        //ip.nomalizeBackgroud();
        //ip.rebinarize(16, 16);
        //image = ip.finalize();
    }

    if (cc) {
        deskewed = false;
        deskew(false);
    }
    //    booster.flatten(&image);
    applyTransforms(image, 1);
    image.save(fileName, "BMP");
    return fileName;
}



void Page::renumberBlocks()
{
    for (int i = 0; i < blocks.count(); i++) {
        blocks[i].setBlockNumber(i+1);
    }
}

