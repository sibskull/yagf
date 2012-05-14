#include "tblock.h"

TBlock::TBlock(int x, int y, int width, int height) :
    QRect(x, y, width, height)
{
    number = -1;
}

TBlock::TBlock(const QRect &r) : QRect(r.x(), r.y(), r.width(), r.height())
{
    number = -1;
}

int TBlock::blockNumber()
{
    return number;
}

void TBlock::setBlockNumber(const int value)
{
    number = value;
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
