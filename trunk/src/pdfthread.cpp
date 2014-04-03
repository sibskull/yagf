#include "pdfthread.h"
#include "pdfextractor.h"
#include <QProcess>

PDFThread::PDFThread(PDFExtractor *parent) :
    QThread()
{
    mparent = parent;
    done = false;
}

void PDFThread::run()
{
    done = false;
    QProcess process;
   // connect(this, SIGNAL(finished()), mparent, SIGNAL(finished()), Qt::QueuedConnection);
   // connect(this, SIGNAL(terminated()), mparent, SIGNAL(finished()), Qt::QueuedConnection);
    connect(mparent, SIGNAL(terminate()), this, SLOT(politeStop()));
    //connect (mparent, SIGNAL(killProcess()), &process, SLOT(kill()), Qt::QueuedConnection);
    //connect (mparent, SIGNAL(terminateProcess()), &process, SLOT(terminate()), Qt::QueuedConnection);
    process.start(command, arguments);
    while (!done)
        if (process.waitForFinished(100))
            break;
        process.kill();
}

void PDFThread::setProcess(const QString &cmd, const QStringList &args)
{
    command = cmd;
    arguments.clear();
    arguments.append(args);
}

void PDFThread::politeStop()
{
    done = true;
}
