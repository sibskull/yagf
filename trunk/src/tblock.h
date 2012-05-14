#ifndef TBLOCK_H
#define TBLOCK_H

#include <QRect>
#include <QList>

class TBlock : public QRect
{
public:
    explicit TBlock( int x, int y, int width, int height );
    TBlock(const QRect &r);
    int blockNumber();
    void setBlockNumber(const int value);
private:
    int number;
    
};

typedef QList<TBlock> TBlocks;

void sortBlocks(TBlocks &blocks);

#endif // TBLOCK_H
