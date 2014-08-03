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

#ifndef PDFTHREAD_H
#define PDFTHREAD_H

#include <QThread>
#include <QStringList>

class PDFExtractor;
class PDFThread : public QThread
{
    Q_OBJECT
public:
    explicit PDFThread(PDFExtractor *parent = 0);
    ~PDFThread();
    void run();
    void setProcess(const QString &cmd, const QStringList &args);
    bool isProcessRunning();
public slots:
    void politeStop();
private:
    bool done;
    QString command;
    QStringList arguments;
    PDFExtractor * mparent;
    bool processRunning;
};

#endif // PDFTHREAD_H
