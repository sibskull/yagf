#include "dispatcher.h"
#include "tpagecollection.h"
#include "mainform.h"
#include "qsnippet.h"

Dispatcher::Dispatcher(PageCollection *pages, MainForm *gui, QObject *parent) :
    QObject(parent), mPages(pages), mGui(gui)
{
    gui->setDispatcher(this);
    connect(this, SIGNAL(_appendPages(QStringList)), mPages, SLOT(appendPages(QStringList)), Qt::QueuedConnection);
    connect(mPages, SIGNAL(loadPage(int)), gui, SLOT(loadPage(int)), Qt::QueuedConnection);
    connect (mPages, SIGNAL(addSnippet(QSnippet* )), gui, SLOT(addSnippet(QSnippet* )), Qt::QueuedConnection);
    connect(gui, SIGNAL(pageSelected(int)), pages, SLOT(pageSelected(int)), Qt::QueuedConnection);
    connect(gui, SIGNAL(pageRemoved(int)), pages, SLOT(pageRemoved(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(_deskew()), mPages, SLOT(deskew()),Qt::QueuedConnection);
    connect(this, SIGNAL(_makeLarger()), mPages, SLOT(makeLarger()), Qt::QueuedConnection);
    connect(this, SIGNAL(_makeSmaller()), mPages, SLOT(makeSmaller()), Qt::QueuedConnection);
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
