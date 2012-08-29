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

#include "projectsaver.h"
#include "tpagecollection.h"
#include "settings.h"
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QImage>

const QString URI = "symmetrica.net/yagf";
const QString VERSION = "0.9.2";

ProjectSaver::ProjectSaver(QObject *parent) :
    QObject(parent)
{
}

bool ProjectSaver::save(const QString &dir)
{
    directory = dir;
    if (!directory.endsWith("/")) directory = directory + "/";
    QString fileName = directory+"yagf_project.xml";
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    stream = new QXmlStreamWriter(&f);
    stream->setAutoFormatting(true);
    stream->writeStartDocument();
    stream->writeStartElement(URI, "yagf");
    stream->writeAttribute(URI, "version", VERSION);
    writeSettings();
    beginWritePage();
    writeBlock();
    stream->writeEndElement();
    stream->writeEndElement();
    stream->writeEndDocument();
    f.flush();
    delete stream;
    f.close();
    return true;
}

void ProjectSaver::beginWritePage()
{
    stream->writeStartElement(URI, "page");
    PageCollection * pc = PageCollection::instance();
    stream->writeAttribute(URI, "image", "file.png");
    stream->writeAttribute(URI, "deskewed", "true");
    stream->writeAttribute(URI, "preprocessed", "true");

}

void ProjectSaver::writeBlock()
{
    stream->writeStartElement(URI, "block");
    stream->writeAttribute(URI, "language", "eng");
    stream->writeEndElement();
}

void ProjectSaver::writeSettings()
{
    stream->writeStartElement(URI, "settins");
    Settings * settings = Settings::instance();
    QString engine;
    if (settings->getSelectedEngine() == UseCuneiform)
        engine = "cuneiform";
    if (settings->getSelectedEngine() == UseTesseract)
        engine = "tesseract";
    stream->writeAttribute(URI, "engine", engine);
    stream->writeAttribute(URI, "defaultlanguage", settings->getLanguage());
}

QString ProjectSaver::copyFile(const QString &source)
{
    QFileInfo fi(source);
    QString dir = fi.absolutePath();
    if (!dir.endsWith("/"))
        dir = dir + "/";
    if (dir == directory)
        return source;
    QString base = fi.baseName();
    QString fileName = base+".png";
    QString newName = directory + fileName;
    if (source.endsWith(".png", Qt::CaseInsensitive)) {
        if (QFile::copy(source, newName))
            return fileName;
        else
            return "";
    } else {
        QImage image(source);
        if (image.save(newName))
            return fileName;
        else
            return "";

    }
    return "";
}

