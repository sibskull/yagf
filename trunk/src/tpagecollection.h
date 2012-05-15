#ifndef TPAGECOLLECTION_H
#define TPAGECOLLECTION_H

#include "tpage.h"
#include <QObject>
#include <QPixmap>
#include <QVector>

class QSnippet;

class TPageCollection : public QObject
{
    Q_OBJECT
public:
    explicit TPageCollection(QObject *parent = 0);
    ~TPageCollection();
    void appendPage(const QString &fileName);
    int count();
    bool makePageCurrent(int index);
    bool makePageCurrentByID(int id);
    void setBeforeFirst(); // the page pointer is set before the first page
    bool makeNextPageCurrent();
    QSnippet * snippet();
    QPixmap pixmap();
    void savePageForRecognition(const QString &fileName);
    void saveBlockForRecognition(QRect r, const QString &fileName);
    void saveBlockForRecognition(int index, const QString &fileName);
    int blockCount();
    TBlock getBlock(const QRect &r);
    TBlock getBlock(int index);
    void selectBlock(const QRect &r);
    TBlock getSelectedBlock();
public slots:
    void makeLarger();
    void makeSmaller();
    void rotate90CW();
    void rotate90CCW();
    void rotate180();
    void deskew();
    void blockAllText();
    void splitPage();
    void addBlock(const QRect & rect);
    void deleteBlock(const QRect & rect);
    void clearBlocks();
    void clear();
signals:
    void loadPage(); // The page is already current
    void addSnippet();
    void cleared();
private slots:
    void pageSelected(int id);
    void pageRemoved(int id);
private:
     TPage * cp();
     int id2Index(int id);
private:
    QVector<TPage *> pages;
    int index;
    int pid;
};

#endif // TPAGECOLLECTION_H
