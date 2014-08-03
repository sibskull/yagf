/*
    YAGF - cuneiform and tesseract OCR graphical front-ends
    Copyright (C) 2009-2014 Andrei Borovsky <anb@symmetrica.net>

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

#include "pdfthread.h"
#include "pdfextractor.h"
#include <QProcess>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

PDFThread::PDFThread(PDFExtractor *parent) :
    QThread()
{
    mparent = parent;
    done = false;
    processRunning= true;
}

PDFThread::~PDFThread()
{
    done = true;
    sleep(2);
}

void PDFThread::run()
{
    done = false;
    QProcess process;
    connect(mparent, SIGNAL(terminate()), this, SLOT(politeStop()));
    process.startDetached(command, arguments);

    while (!done) {
        sleep(1);
        processRunning = (process.state() != QProcess::NotRunning);
    }
    process.kill();
}

void PDFThread::setProcess(const QString &cmd, const QStringList &args)
{
    command = cmd;
    arguments.clear();
    arguments.append(args);
}

bool PDFThread::isProcessRunning()
{
    return processRunning;
}

void PDFThread::politeStop()
{
    done = true;
}
