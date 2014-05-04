#include "dispatcher.h"
#include "tpagecollection.h"
#include "mainform.h"

Dispatcher::Dispatcher(PageCollection *pages, MainForm *gui, QObject *parent) :
    QObject(parent), mPages(pages), mGui(gui)
{
    gui->setDispatcher(this);
    connect(this, SIGNAL(_appendPages(QStringList)), mPages, SLOT(appendPages(QStringList)), Qt::QueuedConnection);
    connect(mPages, SIGNAL(loadPage(int)), gui, SLOT(loadPage(int)), Qt::QueuedConnection);
}

void Dispatcher::loadFiles(const QStringList &files)
{
    emit _appendPages(files);
}
