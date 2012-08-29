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

#ifndef TBLOCK_H
#define TBLOCK_H

#include <QRect>
#include <QList>
#include <QString>

class Block : public QRect
{
public:
    explicit Block( int x, int y, int width, int height );
    Block(const QRect &r);
    int blockNumber();
    void setBlockNumber(const int value);
    void setLanguage(const QString &lang);
    QString getLanguage();
private:
    int number;
    QString language;
    
};

typedef QList<Block> TBlocks;

void sortBlocks(TBlocks &blocks);

#endif // TBLOCK_H
