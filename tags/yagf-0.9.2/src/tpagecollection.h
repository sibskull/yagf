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

#ifndef TPAGECOLLECTION_H
#define TPAGECOLLECTION_H

#include "tpage.h"
#include <QObject>
#include <QPixmap>
#include <QVector>

class QSnippet;

class PageCollection : public QObject
{
    Q_OBJECT
public:
    static PageCollection * instance();
    static void clearCollection();
    bool appendPage(const QString &fileName);
    int count();
    bool makePageCurrent(int index);
    bool makePageCurrentByID(int id);
    void setBeforeFirst(); // the page pointer is set before the first page
    bool makeNextPageCurrent();
    QSnippet * snippet();
    QPixmap pixmap();
    void savePageForRecognition(const QString &fileName);
    void saveRawBlockForRecognition(QRect r, const QString &fileName);
    void saveBlockForRecognition(QRect r, const QString &fileName, const QString &format = "BMP");
    void saveBlockForRecognition(int index, const QString &fileName);
    int blockCount();
    Block getBlock(const QRect &r);
    Block getBlock(int index);
    void selectBlock(const QRect &r);
    Block getSelectedBlock();
    bool pageValid();
    QString fileName();
    bool savePageAsImage(const QString &fileName, const QString &format);
    bool isDeskewed();
    bool isPreprocessed();
    qreal getRotation();
    void setRotation(const qreal value);
    void setDeskewed(const bool value);
    void setPreprocessed(const bool value);
    void reloadPage();
public slots:
    void makeLarger();
    void makeSmaller();
    void rotate90CW();
    void rotate90CCW();
    void rotate180();
    void deskew();
    void blockAllText();
    bool splitPage(bool preprocess);
    void addBlock(const QRect & rect);
    void deleteBlock(const QRect & rect);
    void clearBlocks();
    void clear();
signals:
    void loadPage(); // The page is already current
    void addSnippet(int index);
    void cleared();
private slots:
    void pageSelected(int id);
    void pageRemoved(int id);
private:
    PageCollection(QObject *parent = 0);
    PageCollection(const PageCollection &);
    ~PageCollection();
     Page * cp();
     int id2Index(int id);
private:
    QVector<Page *> pages;
    int index;
    int pid;
    static PageCollection * m_instance;
};

#endif // TPAGECOLLECTION_H
