#include "tpage.h"
#include "settings.h"
#include "ccbuilder.h"
#include "CCAnalysis.h"
#include "PageAnalysis.h"
#include "analysis.h"
#include <QImageReader>
#include <QImageWriter>
#include <QSize>
#include <QRect>
#include <cmath>

TPage::TPage(const int pid, QObject *parent) :
    QObject(parent), selectedBlock(0,0,0,0)
{
    imageLoaded = false;
    loadedBefore = false;
    ccbuilder = NULL;
    rotation = 0;
    deskewed = false;
    mFileName.clear();
    this->pid = pid;
}

TPage::~TPage()
{
    delete ccbuilder;
}

bool TPage::loadFile(QString fileName, bool loadIntoView)
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
    if (!loadedBefore) {
        ccbuilder = new CCBuilder(img2);
        settings = Settings::instance();
        if (settings->getCropLoaded()) {
            ccbuilder->setGeneralBrightness(360);
            ccbuilder->setMaximumColorComponent(100);
            QRect r = ccbuilder->crop();
            crop1 = r;
            img2 = img2.copy(crop1);
            delete ccbuilder;
            ccbuilder = new CCBuilder(img2);
        }
        ccbuilder->labelCCs();
        mGeneralBrightness = ccbuilder->getGB();
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

QPixmap TPage::displayPixmap()
{
    return QPixmap::fromImage(currentImage());
}

QImage TPage::thumbnail()
{
    return img16;
}

bool TPage::makeLarger()
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

bool TPage::makeSmaller()
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

void TPage::rotate(qreal angle)
{
    rotateImageInternal(img2, angle);
    rotation += angle;
    scaleImages();
    clearBlocks();
}

void TPage::unload()
{
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

void TPage::addBlock(TBlock block)
{
    QRect r = block;
    normalizeRect(r);
    bool add = true;
    foreach (TBlock b, blocks) {
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
    sortBlocks(blocks);
}

void TPage::deleteBlock(const TBlock &b)
{
    blocks.removeOne(b);
    sortBlocks(blocks);
}

void TPage::deleteBlock(const QRect &r)
{
    QRect rx = r;
    normalizeRect(rx);
    foreach (TBlock b, blocks) {
        QRect r1 = b;
        if (qrects_equal(rx, r1)) {
            blocks.removeAll(b);
            break;
        }
    }
    sortBlocks(blocks);
}

TBlock TPage::getBlock(const QRect &r)
{
    QRect rn =r;
    normalizeRect(rn);
    foreach (TBlock b, blocks) {
        QRect r1 = b;
        if (qrects_equal(rn,r1)) {
            scaleRect(b);
            return b;
        }
    }
    return TBlock(0,0,0,0);
}

TBlock TPage::getBlock(int index)
{
    TBlock b = blocks.at(index);
    scaleRect(b);
    return b;
}

int TPage::blockCount()
{
    return blocks.count();
}

void TPage::clearBlocks()
{
    blocks.clear();
}

void TPage::savePageForRecognition(const QString &fileName)
{
    QImageReader ir(mFileName);
    QImage image = ir.read();
    applyTransforms(image, 1);
    image.save(fileName, "BMP");
}

void TPage::saveBlockForRecognition(QRect r, const QString &fileName)
{
    r.setX(r.x()*2);
    r.setY(r.y()*2);
    r.setWidth(r.width()*2);
    r.setHeight(r.height()*2);
    QImageReader ir(mFileName);
    QImage image = ir.read();
    applyTransforms(image, 1);
    image.copy(r).save(fileName);
}

void TPage::saveBlockForRecognition(int index, const QString &fileName)
{
    saveBlockForRecognition(blocks.at(index), fileName);
}

void TPage::selectBlock(const QRect &r)
{
    QRect rn =r;
    normalizeRect(rn);
    foreach (TBlock b, blocks) {
        QRect r1 = b;
        if (rn == r1) {
            selectedBlock = b;
            break;
        }
    }
}

TBlock TPage::getSelectedBlock()
{
    return selectedBlock;
}

void TPage::deskew()
{
    if (deskewed) return;
    if (imageLoaded) {
        CCAnalysis * an = new CCAnalysis(ccbuilder);
        an->analize();
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
        delete an;
    }
}

void TPage::rotate90CW()
{
    rotate(90);
}

void TPage::rotate90CCW()
{
    rotate(-90);
}

void TPage::rotate180()
{
    rotate(180);
}

void TPage::blockAllText()
{
    clearBlocks();
    BlockSplitter bs;
    bs.setImage(img2, rotation, scale);
    QRect r = bs.getRootBlock(currentImage());
    addBlock(r);
}

void TPage::splitPage()
{
    clearBlocks();
    BlockSplitter bs;
    bs.setImage(img2, rotation, 0.5);// sideBar->getScale());
    bs.getBars();
    bs.splitBlocks();
    QList<Rect> blocks = bs.getBlocks();
    qreal sf = 2.0*scale;
    QRect cr = bs.getRotationCropRect(currentImage());
    foreach (Rect block, blocks) {
        QRect r;
        block.x1 *=sf;
        block.y1 *=sf;
        block.x2 *= sf;
        block.y2 *=sf;

        block.x1 += cr.x();
        block.y1 += cr.y();
        block.x2 += cr.x();
        block.y2 += cr.y();

        r.setX(block.x1);
        r.setY(block.y1);
        r.setWidth(block.x2 - block.x1);
        r.setHeight(block.y2 - block.y1);
        addBlock(r);
    }
}

QString TPage::fileName()
{
    return mFileName;
}

int TPage::pageID()
{
    return pid;
}


void TPage::applyTransforms(QImage &image, qreal scale)
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

void TPage::rotateImageInternal(QImage &image, qreal angle)
{
    qreal x = image.width() / 2;
    qreal y = image.height() / 2;
    image = image.transformed(QTransform().translate(-x, -y).rotate(angle).translate(x, y), Qt::SmoothTransformation);
}

void TPage::scaleImages()
{
    img4 = img2.scaledToWidth(img2.width() / 2);
    img6 = img2.scaledToWidth(img2.width() / 2.5);
    img8 = img4.scaledToWidth(img4.width() / 2);
    img16 = img8.scaledToWidth(img8.width() / 2);
}

void TPage::normalizeRect(QRect &rect)
{
    qreal oldw = rect.width();
    qreal oldh = rect.height();
    rect.setX(rect.x()*0.5/scale);
    rect.setY(rect.y()*0.5/scale);
    rect.setWidth(oldw*0.5/scale);
    rect.setHeight(oldh*0.5/scale);
}

void TPage::scaleRect(QRect &rect)
{
    qreal oldw = rect.width();
    qreal oldh = rect.height();
    rect.setX(rect.x()/0.5*scale);
    rect.setY(rect.y()/0.5*scale);
    rect.setWidth(oldw/0.5*scale);
    rect.setHeight(oldh/0.5*scale);
}

QImage TPage::tryRotate(QImage image, qreal angle)
{
    qreal x = image.width() / 2;
    qreal y = image.height() / 2;
    return image.transformed(QTransform().translate(-x, -y).rotate(angle).translate(x, y), Qt::SmoothTransformation);
}

QImage TPage::currentImage()
{
    if (scale <= 0.125)
        return img8;
    if (scale <= 0.2)
        return img6;
    if (scale <= 0.25)
        return img4;
    return img2;
}

