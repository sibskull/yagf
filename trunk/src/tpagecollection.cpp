#include "tpagecollection.h"
#include "qsnippet.h"
//#include "settings.h"

TPageCollection::TPageCollection(QObject *parent) :
    QObject(parent)
{
    index = -1;
    pid = 0;
}

TPageCollection::~TPageCollection()
{
    clear();
}

void TPageCollection::appendPage(const QString &fileName)
{
    //Settings * settings = Settings::instance();
    TPage * p = new TPage(++pid);
    if (p->loadFile(fileName)) {
        pages.append(p);
        emit addSnippet();
        index++;
    }
}

int TPageCollection::count()
{
    return pages.count();
}

bool TPageCollection::makePageCurrent(int index)
{
    this->index = index;
    return index < pages.count();
}

bool TPageCollection::makePageCurrentByID(int id)
{
    return makePageCurrent(id2Index(id)) >= 0;
}

void TPageCollection::setBeforeFirst()
{
    index = -1;
}

bool TPageCollection::makeNextPageCurrent()
{
    index++;
    if (index < count())
        return true;
    index = -1;
    return false;
}

QSnippet *TPageCollection::snippet()
{
    if (!cp()) return NULL;
    QSnippet * s = new QSnippet();
    s->setPage(cp()->pageID(), cp()->fileName(), cp()->thumbnail());
    return s;
}

QPixmap TPageCollection::pixmap()
{
    if (!cp()) return QPixmap();
    return cp()->displayPixmap();
}

void TPageCollection::savePageForRecognition(const QString &fileName)
{
    if (!cp()) return;
    cp()->savePageForRecognition(fileName);
}

void TPageCollection::saveBlockForRecognition(QRect r, const QString &fileName)
{
    if (!cp()) return;
    cp()->saveBlockForRecognition(r, fileName);
}

void TPageCollection::saveBlockForRecognition(int index, const QString &fileName)
{
    if (!cp()) return;
    cp()->saveBlockForRecognition(index, fileName);
}

int TPageCollection::blockCount()
{
    if (!cp()) return 0;
    return cp()->blockCount();
}

TBlock TPageCollection::getBlock(const QRect &r)
{
    TBlock block(0,0,0,0);
    if (!cp()) return block;
    return cp()->getBlock(r);
}

TBlock TPageCollection::getBlock(int index)
{
    TBlock block(0,0,0,0);
    if (!cp()) return block;
    return cp()->getBlock(index);
}

void TPageCollection::selectBlock(const QRect &r)
{
    if (!cp()) return;
    cp()->selectBlock(r);
}

TBlock TPageCollection::getSelectedBlock()
{
    TBlock block(0,0,0,0);
    if (!cp()) return block;
    return cp()->getSelectedBlock();
}

void TPageCollection::makeLarger()
{
    if (!cp()) return;
    cp()->makeLarger();
    emit loadPage();
}

void TPageCollection::makeSmaller()
{
    if (!cp()) return;
    cp()->makeSmaller();
    emit loadPage();
}

void TPageCollection::rotate90CW()
{
    if (!cp()) return;
    cp()->rotate90CW();
    emit loadPage();
}

void TPageCollection::rotate90CCW()
{
    if (!cp()) return;
    cp()->rotate90CCW();
    emit loadPage();
}

void TPageCollection::rotate180()
{
    if (!cp()) return;
    cp()->rotate180();
    emit loadPage();
}

void TPageCollection::deskew()
{
    if (!cp()) return;
    cp()->deskew();
    emit loadPage();
}

void TPageCollection::blockAllText()
{
    if (!cp()) return;
    cp()->blockAllText();
    emit loadPage();
}

void TPageCollection::splitPage()
{
    if (!cp()) return;
    cp()->splitPage();
    emit loadPage();
}

void TPageCollection::addBlock(const QRect &rect)
{
    if (!cp()) return;
    TBlock block(rect.x(), rect.y(), rect.width(), rect.height());
    cp()->addBlock(block);
}

void TPageCollection::deleteBlock(const QRect &rect)
{
    if (!cp()) return;
    cp()->deleteBlock(rect);
}

void TPageCollection::clearBlocks()
{
    if (!cp()) return;
    cp()->clearBlocks();
}

void TPageCollection::clear()
{
    foreach (TPage * p, pages) {
        delete p;
    }
    pages.clear();
    emit cleared();
    index = -1;
}


TPage *TPageCollection::cp()
{
    if ((index < 0)|| (index >= count()))
        return (TPage*) 0;
    return pages.at(index);
}

int TPageCollection::id2Index(int id)
{
    foreach (TPage *p, pages) {
        if (p->pageID() == id)
            return pages.indexOf(p);
    }
    return -1;
}

void TPageCollection::pageSelected(int id)
{
    makePageCurrent(id2Index(id));
    emit loadPage();
}

void TPageCollection::pageRemoved(int id)
{
    int index = id2Index(id);
    if (index >= 0) {
        delete pages.at(index);
        pages.remove(index);
    }
    if (index >= pages.count())
        index = pages.count() - 1;
    makePageCurrent(index);
    emit loadPage();
}
