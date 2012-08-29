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

#include "tblock.h"

Block::Block(int x, int y, int width, int height) :
    QRect(x, y, width, height)
{
    number = -1;
    language = "default";
}

Block::Block(const QRect &r) : QRect(r.x(), r.y(), r.width(), r.height())
{
    number = -1;
}

int Block::blockNumber()
{
    return number;
}

void Block::setBlockNumber(const int value)
{
    number = value;
}

void Block::setLanguage(const QString &lang)
{
    language = lang;
}

QString Block::getLanguage()
{
    return language;
}

bool rectLessThan(const QRect &r1, const QRect &r2)
{
    if (r1.y() < r2.y())
        return true;
    if (r1.x() < r2.x())
        return true;
    return false;
}

void sortBlocks(TBlocks &blocks)
{
    qSort(blocks.begin(), blocks.end(), rectLessThan);
}
