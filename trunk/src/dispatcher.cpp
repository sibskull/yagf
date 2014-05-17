#include "dispatcher.h"
#include "tpagecollection.h"
#include "mainform.h"
#include "qsnippet.h"
#include  <unistd.h>



Dispatcher::Dispatcher(PageCollection *pages, MainForm *gui, QObject *parent) :
    QObject(parent), mPages(pages), mGui(gui), mutex()
{
    gui->setDispatcher(this);
    connect(this, SIGNAL(_appendPages(QStringList)), mPages, SLOT(appendPages(QStringList)), Qt::QueuedConnection);
    connect(mPages, SIGNAL(loadPage(int)), gui, SLOT(loadPage(int)), Qt::QueuedConnection);
    connect(mPages, SIGNAL(loadPage(int)), this, SLOT(_refresh(int)), Qt::DirectConnection);
    connect (mPages, SIGNAL(addSnippet(QSnippet* )), gui, SLOT(addSnippet(QSnippet* )), Qt::QueuedConnection);
    connect(gui, SIGNAL(pageSelected(int)), pages, SLOT(pageSelected(int)), Qt::QueuedConnection);
    connect(gui, SIGNAL(pageRemoved(int)), pages, SLOT(pageRemoved(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(_deskew()), mPages, SLOT(deskew()),Qt::QueuedConnection);
    connect(this, SIGNAL(_makeLarger()), mPages, SLOT(makeLarger()), Qt::QueuedConnection);
    connect(this, SIGNAL(_makeSmaller()), mPages, SLOT(makeSmaller()), Qt::QueuedConnection);
    connect(this, SIGNAL(_rotate90CW()), mPages, SLOT(rotate90CW()), Qt::QueuedConnection);
    connect(this, SIGNAL(_rotate90CCW()), mPages, SLOT(rotate90CCW()), Qt::QueuedConnection);
    connect(this, SIGNAL(_rotate180()), mPages, SLOT(rotate180()), Qt::QueuedConnection);
    connect(this, SIGNAL(_blockAllText()), mPages, SLOT(blockAllText()), Qt::QueuedConnection);
    connect(this, SIGNAL(_splitPage(bool)), mPages, SLOT(splitPage(bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(_clearBlocks()), mPages, SLOT(clearBlocks()), Qt::QueuedConnection);
    connect(this, SIGNAL(_addBlock(QRect)), mPages, SLOT(addBlock(QRect)), Qt::QueuedConnection);
    connect(this, SIGNAL(_deleteBlock(QRect)), mPages, SLOT(deleteBlock(QRect)), Qt::QueuedConnection);
}

void Dispatcher::loadFiles(const QStringList &files)
{
    emit _appendPages(files);
}

void Dispatcher::deskew()
{
    emit _deskew();
}

void Dispatcher::makeLarger()
{
    emit _makeLarger();
}

void Dispatcher::makeSmaller()
{
    emit _makeSmaller();
}

void Dispatcher::rotate90CW()
{
    emit _rotate90CW();
}

void Dispatcher::rotate90CCW()
{
    emit _rotate90CCW();
}

void Dispatcher::rotate180()
{
    emit _rotate180();
}

void Dispatcher::blockAllText()
{
    emit _blockAllText();
}

void Dispatcher::splitPage(bool preprocess)
{
    emit _splitPage(preprocess);
}

void Dispatcher::clearBlocks()
{
    emit _clearBlocks();
}

void Dispatcher::enter()
{
    mutex.lock();
}

void Dispatcher::leave()
{
    mutex.unlock();
}

int Dispatcher::refPageID()
{
    return pageID;
}

QPixmap Dispatcher::refPixmap()
{
    QPixmap res = pixmap;
    pixmap.scaled(QSize(0,0));
    return res;
}

QList<Block> Dispatcher::refBlocks()
{
    return blocks;
}

QString Dispatcher::refFileName()
{
    return fileName;
}

void Dispatcher::addBlock(const QRect &rect)
{
    emit _addBlock(rect);
}

void Dispatcher::deleteBlock(const QRect &rect)
{
    emit _deleteBlock(rect);
}

void Dispatcher::_refresh(int pid)
{
    enter();
    mPages->makePageCurrentByID(pid);
    pageID = pid;
    pixmap = mPages->pixmap();
    blocks.clear();
    for (int i = 0; i < mPages->blockCount(); i++)
        blocks.append(mPages->getBlock(i));
    fileName = mPages->fileName();
    leave();
}
