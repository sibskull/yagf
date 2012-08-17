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
#include "qxttiffiohandler.h"
#include "settings.h"
#include <QApplication>
#include <QFile>
#include <QImage>


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

bool TPageCollection::appendPage(const QString &fileName)
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
#endif
    {
        TPage * p = new TPage(++pid);
        connect(p,SIGNAL(refreshView()), this, SIGNAL(loadPage()));
        if (p->loadFile(fileName)) {
            pages.append(p);
            index = pages.count() - 1;
            emit addSnippet(index);
            return true;
        } else return false;
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

void TPageCollection::saveRawBlockForRecognition(QRect r, const QString &fileName)
{
    if (!cp()) return;
    cp()->saveRawBlockForRecognition(r, fileName);
}

void TPageCollection::saveBlockForRecognition(QRect r, const QString &fileName)
{
    if (!cp()) return;
    cp()->saveBlockForRecognition(r, fileName);
}

void TPageCollection::saveBlockForRecognition(int index, const QString &fileName)
{
    if (!cp()) return;
    if (index == 0)
        cp()->sortBlocksInternal();
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

bool TPageCollection::pageValid()
{
    return cp() != 0;
}

QString TPageCollection::fileName()
{
    if (!cp())
        return "";
    return cp()->fileName();
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
