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

#include "tpagecollection.h"
#include "qsnippet.h"
#ifdef TIFF_IO
#include "qxttiffiohandler.h"
#endif
#include "settings.h"
#include <QApplication>
#include <QFile>
#include <QImage>

PageCollection * PageCollection::m_instance = NULL;

PageCollection::PageCollection(QObject *parent) :
    QObject(parent)
{
    index = -1;
    pid = 0;
}

PageCollection::PageCollection(const PageCollection &)
{
}

PageCollection::~PageCollection()
{
    clear();
}

bool PageCollection::appendPage(const QString &fileName)
{
    #ifdef TIFF_IO
    if (fileName.endsWith(".tif", Qt::CaseInsensitive) || fileName.endsWith(".tiff", Qt::CaseInsensitive)) {
        QXtTiffIOHandler * toh = new QXtTiffIOHandler();
        QFile f(fileName);
        f.open(QIODevice::ReadOnly);
        toh->setDevice(&f);
        if (toh->canRead()) {
            if (toh->imageCount() == 1) {
                delete toh;
                appendPage(fileName);
                return true;
            } else {
                for (int i = 0; i < toh->imageCount(); i++) {
                    QImage img;
                    toh->jumpToImage(i);
                    toh->read(&img);
                    QString nfn = Settings::instance()->workingDir() + QString("tiff-page-%1.jpg").arg(i+1);
                    img.save(nfn, "JPEG");
                    appendPage(nfn);
                    QApplication::processEvents();
                }
                delete toh;
                return true;
            }
        } else return false;
    } else 
    #endif // TIFF_IO
{
        Page * p = new Page(++pid);
        connect(p,SIGNAL(refreshView()), this, SIGNAL(loadPage()));
        if (p->loadFile(fileName)) {
            pages.append(p);
            index = pages.count() - 1;
            emit addSnippet(index);
            return true;
        } else return false;
    }
}

int PageCollection::count()
{
    return pages.count();
}

bool PageCollection::makePageCurrent(int index)
{
    this->index = index;
    return index < pages.count();
}

bool PageCollection::makePageCurrentByID(int id)
{
    return makePageCurrent(id2Index(id)) >= 0;
}

void PageCollection::setBeforeFirst()
{
    index = -1;
}

bool PageCollection::makeNextPageCurrent()
{
    index++;
    if (index < count())
        return true;
    index = -1;
    return false;
}

QSnippet *PageCollection::snippet()
{
    if (!cp()) return NULL;
    QSnippet * s = new QSnippet();
    s->setPage(cp()->pageID(), cp()->fileName(), cp()->thumbnail());
    return s;
}

QPixmap PageCollection::pixmap()
{
    if (!cp()) return QPixmap();
    return cp()->displayPixmap();
}

void PageCollection::savePageForRecognition(const QString &fileName)
{
    if (!cp()) return;
    cp()->savePageForRecognition(fileName);
}

void PageCollection::saveRawBlockForRecognition(QRect r, const QString &fileName)
{
    if (!cp()) return;
    cp()->saveRawBlockForRecognition(r, fileName);
}

void PageCollection::saveBlockForRecognition(QRect r, const QString &fileName, const QString &format)
{
    if (!cp()) return;
    cp()->saveBlockForRecognition(r, fileName, format);
}

void PageCollection::saveBlockForRecognition(int index, const QString &fileName)
{
    if (!cp()) return;
    if (index == 0)
        cp()->sortBlocksInternal();
    cp()->saveBlockForRecognition(index, fileName);
}

int PageCollection::blockCount()
{
    if (!cp()) return 0;
    return cp()->blockCount();
}

Block PageCollection::getBlock(const QRect &r)
{
    Block block(0,0,0,0);
    if (!cp()) return block;
    return cp()->getBlock(r);
}

Block PageCollection::getBlock(int index)
{
    Block block(0,0,0,0);
    if (!cp()) return block;
    return cp()->getBlock(index);
}

void PageCollection::selectBlock(const QRect &r)
{
    if (!cp()) return;
    cp()->selectBlock(r);
}

Block PageCollection::getSelectedBlock()
{
    Block block(0,0,0,0);
    if (!cp()) return block;
    return cp()->getSelectedBlock();
}

bool PageCollection::pageValid()
{
    return cp() != 0;
}

QString PageCollection::fileName()
{
    if (!cp())
        return "";
    return cp()->fileName();
}

bool PageCollection::savePageAsImage(const QString &fileName, const QString &format)
{
    if (!cp())
        return false;
    return cp()->savePageAsImage(fileName, format);
}

bool PageCollection::isDeskewed()
{
    if (cp())
        return cp()->isDeskewed();
    return false;
}

bool PageCollection::isPreprocessed()
{
    if (cp())
        return cp()->isPreprocessed();
    return false;
}

qreal PageCollection::getRotation()
{
    if (cp())
        return cp()->getRotation();
    return 0;
}

void PageCollection::setRotation(const qreal value)
{
    if (cp())
        cp()->rotate(value);
}

void PageCollection::setDeskewed(const bool value)
{
    if (cp())
        cp()->setDeskewed(value);
}

void PageCollection::setPreprocessed(const bool value)
{
    if (cp())
        cp()->setPreprocessed(value);
}

void PageCollection::reloadPage()
{
    emit loadPage();
}

void PageCollection::makeLarger()
{
    if (!cp()) return;
    cp()->makeLarger();
    emit loadPage();
}

void PageCollection::makeSmaller()
{
    if (!cp()) return;
    cp()->makeSmaller();
    emit loadPage();
}

void PageCollection::rotate90CW()
{
    if (!cp()) return;
    cp()->rotate90CW();
    emit loadPage();
}

void PageCollection::rotate90CCW()
{
    if (!cp()) return;
    cp()->rotate90CCW();
    emit loadPage();
}

void PageCollection::rotate180()
{
    if (!cp()) return;
    cp()->rotate180();
    emit loadPage();
}

void PageCollection::deskew()
{
    if (!cp()) return;
    cp()->deskew();
    emit loadPage();
}

void PageCollection::blockAllText()
{
    if (!cp()) return;
    cp()->blockAllText();
    emit loadPage();
}

bool PageCollection::splitPage(bool preprocess)
{
    if (!cp()) return false;
    bool res = cp()->splitPage(preprocess);
    emit loadPage();
    return res;
}

void PageCollection::addBlock(const QRect &rect)
{
    if (!cp()) return;
    Block block(rect.x(), rect.y(), rect.width(), rect.height());
    cp()->addBlock(block);
}

void PageCollection::deleteBlock(const QRect &rect)
{
    if (!cp()) return;
    cp()->deleteBlock(rect);
}

void PageCollection::clearBlocks()
{
    if (!cp()) return;
    cp()->clearBlocks();
}

void PageCollection::clear()
{
    foreach (Page * p, pages) {
        delete p;
    }
    pages.clear();
    emit cleared();
    index = -1;
    pid = 0;
}


Page *PageCollection::cp()
{
    if ((index < 0)|| (index >= count()))
        return (Page*) 0;
    return pages.at(index);
}

int PageCollection::id2Index(int id)
{
    foreach (Page *p, pages) {
        if (p->pageID() == id)
            return pages.indexOf(p);
    }
    return -1;
}

void PageCollection::pageSelected(int id)
{
    makePageCurrent(id2Index(id));
    emit loadPage();
}

void PageCollection::pageRemoved(int id)
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

PageCollection *PageCollection::instance()
{
    if (!m_instance)
        m_instance = new PageCollection();
    return m_instance;
}

void PageCollection::clearCollection()
{
    if (m_instance)
        m_instance->clear();
}
