#ifndef TPAGE_H
#define TPAGE_H

#include "tblock.h"
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
    bool loadFile(const QString fileName, bool loadIntoView = true);
    QPixmap displayPixmap();
    QImage thumbnail();
    bool makeLarger();
    bool makeSmaller();
    void rotate(qreal angle);
    void unload();
    void addBlock(const TBlock &block);
    void deleteBlock(const TBlock &b);
    void deleteBlock(const QRect &r);
    TBlock getBlock(const QRect &r);
    TBlock getBlock(int index);
    int blockCount();
    void clearBlocks();
    void savePageForRecognition(const QString &fileName);
    void saveBlockForRecognition(QRect r, const QString &fileName);
    void saveBlockForRecognition(int index, const QString &fileName);
    void selectBlock(const QRect &r);
    TBlock getSelectedBlock();
    void deskew();
    void rotate90CW();
    void rotate90CCW();
    void rotate180();
    void blockAllText();
    void splitPage();
    QString fileName();
    int pageID();
signals:
    
public slots:
private:
    void applyTransforms(QImage &image, qreal scale);
    void rotateImageInternal(QImage &image, qreal angle);
    void scaleImages();
    void normalizeRect(QRect &rect);
    void scaleRect(QRect &rect);
    QImage tryRotate(QImage image, qreal angle);
    QImage currentImage();
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
    QString mFileName;
    CCBuilder * ccbuilder;
    Settings * settings;
    int blockPointer;
    int pid;
    TBlock selectedBlock;
};

#endif // TPAGE_H
