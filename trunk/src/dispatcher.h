#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "tblock.h"
#include <QObject>
#include <QStringList>
#include <QRect>
#include <QPixmap>
#include <QMutex>

class PageCollection;
class MainForm;
class Dispatcher : public QObject
{
    Q_OBJECT
public:
    explicit Dispatcher(PageCollection * pages, MainForm * gui, QObject *parent = 0);
    void loadFiles(const QStringList &files);
    void deskew();
    void makeLarger();
    void makeSmaller();
    void rotate90CW();
    void rotate90CCW();
    void rotate180();
    void blockAllText();
    void splitPage(bool preprocess);
    void clearBlocks();    
    void enter();
    void leave();
    int refPageID();
    QPixmap refPixmap();
    QList<Block> refBlocks();
    QString refFileName();
signals:
    void _appendPages(const QStringList &files);
    void _deskew();
    void _makeLarger();
    void _makeSmaller();
    void _rotate90CW();
    void _rotate90CCW();
    void _rotate180();
    void _blockAllText();
    void _splitPage(bool preprocess);
    void _clearBlocks();
    void _addBlock(const QRect & rect);
    void _deleteBlock(const QRect & rect);

public slots:
    void addBlock(const QRect & rect);
    void deleteBlock(const QRect & rect);
private slots:
    void _refresh(int pid);
private:
    PageCollection * mPages;
    MainForm * mGui;
    QList<Block> blocks;
    int pageID;
    QPixmap pixmap;
    QString fileName;
    QMutex mutex;
};

#endif // DISPATCHER_H
