/*
    YAGF - cuneiform and tesseract OCR graphical front-ends
    Copyright (C) 2009-2010 Andrei Borovsky <anb@symmetrica.net>

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
#include "settings.h"
#include "utils.h"
#include <QThread>
#include <QProcess>
#include  <QStringList>
#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QFileInfoList>



PDFExtractor::PDFExtractor(QObject *parent) :
    QObject(parent)
{
}

void PDFExtractor::setCommandStringPaged(const QString &cmdStr)
{
    commandStringPaged = cmdStr;
}

void PDFExtractor::setCommandStringEntire(const QString &cmdStr)
{
    commandStringEntire = cmdStr;
}

void PDFExtractor::setOutputDir()
{
    QString pdfout = Settings::instance()->workingDir()+ QString("pdfout/");
    outputDir = pdfout;
    QDir dir(pdfout);
    if (!dir.exists())
        dir.mkdir(pdfout);
    else {
        dir.setFilter(QDir::Files);
        QStringList sl = dir.entryList();
        foreach (QString s, sl)
        dir.remove(pdfout+s);
    }
}

QString PDFExtractor::getOutputDir()
{
    return outputDir;
}

void PDFExtractor::setOutputExtension(const QString &value)
{
    outputExtension = value;
}

QString PDFExtractor::getOutputExtension()
{
    return outputExtension;
}

void PDFExtractor::setOutputPrefix(const QString &value)
{
    outputPrefix = value;
}

QString PDFExtractor::getOutputPrefix()
{
    return outputPrefix;
}

void PDFExtractor::setResolution(const QString &value)
{
    resolution = value;
}

QString PDFExtractor::getResolution()
{
    return resolution;
}

void PDFExtractor::setSourcePDF(const QString &value)
{
    sourcePDF = value;
}

QString PDFExtractor::getSourcePDF()
{
    return sourcePDF;
}

void PDFExtractor::setStartPage(const QString &value)
{
    startPage = value.toInt();
}

int PDFExtractor::getStartPage()
{
    return startPage;
}

void PDFExtractor::setStopPage(const QString &value)
{
    stopPage = value.toInt();
}

int PDFExtractor::getStopPage()
{
    return stopPage;
}

void PDFExtractor::cancel()
{
    stopPage = 1;
    emit terminate();
}

void PDFExtractor::execInternal(const QString &command, const QStringList &arguments)
{
    filters.clear();
    filters << QString("page*.%1").arg(getOutputExtension());
    PDFThread thread(this);
    thread.setProcess(command, arguments);
    thread.start();
     qSleep(1000); // dirty quick fix
    QDir dir;
    prepareDir(dir);
    QFileInfoList prefil = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    QFileInfoList oldFil;
    bool cont = true;
    while (cont) {
        if (oldFil.count() == 0)
            qSleep(500);
        else qSleep(200);
        QDir dir;
        prepareDir(dir);
        QFileInfoList fil;
        QApplication::processEvents();
        fil = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        foreach (QFileInfo fi, fil) {
            if (!oldFil.contains(fi)) {
                oldFil.append(fi);
                emit addPage(fi.absoluteFilePath());
                QApplication::processEvents();
            }
        }
        if (!thread.isProcessRunning())
            cont = false;
    }
    {
        QDir dir;
        prepareDir(dir);
        QFileInfoList fil;
        if (stopPage < 0) {
            stopPage = pageCount();
            startPage = 1;
        }
        if (stopPage > 0) {
            while (oldFil.count() + fil.count() < stopPage - (startPage == 0 ? startPage - prefil.count() - 4 : startPage - prefil.count() -1)) {
                QApplication::processEvents();
                fil = dir.entryInfoList(filters, QDir::Files, QDir::Name);
                foreach (QFileInfo fi, fil) {
                    if (!oldFil.contains(fi)) {
                        oldFil.append(fi);
                        emit addPage(fi.absoluteFilePath());
                        QApplication::processEvents();
                    } else
                        QApplication::processEvents();
                }
            }
        } else {
            QApplication::processEvents();
            fil = dir.entryInfoList(filters, QDir::Files, QDir::Name);
            foreach (QFileInfo fi, fil) {
                if (!oldFil.contains(fi)) {
                    oldFil.append(fi);
                    emit addPage(fi.absoluteFilePath());
                    QApplication::processEvents();
                } else
                    QApplication::processEvents();
            }
        }

    }
    emit finished();
}

void PDFExtractor::prepareDir(QDir &dir)
{
    pageCount();
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);
    dir.setSorting(QDir::Name);
    dir.setPath(outputDir);
}

int PDFExtractor::filesRemaining(const QString &fileName)
{
    lastFile = fileName;
    QDir dir;
    prepareDir(dir);
    QStringList sl =  dir.entryList();
    sl.sort();
    for (int i = 0; i < sl.count(); i++) {
        if (fileName.endsWith(sl.at(i)))
            return sl.count() - i - 1;
    }
    return -1;
}

void PDFExtractor::removeRemaining()
{
    if (lastFile != "") {
        QDir dir;
        prepareDir(dir);
        QStringList sl =  dir.entryList();
        sl.sort();
        bool doDelete = false;
        for (int i = 0; i < sl.count(); i++) {
            if (doDelete) {
                QFile f(outputDir+sl.at(i));
                f.remove();
            }
            if (lastFile.endsWith(sl.at(i)))
                doDelete = true;

        }

    }
    lastFile = "";
}

int PDFExtractor::filesTotal()
{
    QDir dir;
    prepareDir(dir);
    QStringList sl =  dir.entryList();
    return sl.count();
}

#include "utils.h"

int PDFExtractor::pageCount()
{
    if (findProgram(QString("pdfinfo"))) {
        QProcess proc;
        proc.setEnvironment(QProcess::systemEnvironment());
        QString tex = "pdfinfo " + sourcePDF;
        proc.setReadChannel(QProcess::StandardOutput);
        proc.start(tex);
        proc.waitForFinished();
        //proc.execute(tex);
        proc.waitForReadyRead(500);
        QByteArray ba = proc.readAllStandardOutput();
        QString out = QString(ba);
        QStringList sl = out.split('\n');
        sl.count();
        foreach (QString s, sl) {
            if (s.startsWith("Pages:")) {
                QStringList sl2 = s.split(' ');
                QString num = sl2.last();
                return num.toInt();
            }
        }
    }
    return -1;
}
