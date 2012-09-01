/*
    YAGF - cuneiform and tesseract OCR graphical front-end
    Copyright (C) 2011 Andrei Borovsky <anb@symmetrica.net>

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

#include "projectmanager.h"
#include "tpagecollection.h"
#include "settings.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QXmlStreamAttributes>

const QString URI = "symmetrica.net/yagf";
const QString VERSION = "0.9.2";

inline QString boolToString(bool value)
{
    return value ? "true" : "false";
}

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
    if (!f.open(QIODevice::WriteOnly|QIODevice::Truncate))
        return false;
    stream = new QXmlStreamWriter(&f);
    stream->setAutoFormatting(true);
    stream->writeStartDocument();
    stream->writeStartElement(URI, "yagf");
    stream->writeAttribute(URI, "version", VERSION);
    writeSettings();
    writePages();
    stream->writeEndDocument();
    f.flush();
    delete stream;
    f.close();
    return true;
}

void ProjectSaver::writePages()
{
    PageCollection * pc = PageCollection::instance();
    for (int i =0; i < pc->count(); i++) {
        stream->writeStartElement(URI, "page");
        pc->makePageCurrent(i);
        stream->writeAttribute(URI, "image", copyFile(pc->fileName()));
        stream->writeAttribute(URI, "deskewed", boolToString(pc->isDeskewed()));
        stream->writeAttribute(URI, "rotation", QString::number(pc->getRotation()));
        stream->writeAttribute(URI, "preprocessed", boolToString(pc->isPreprocessed()));
        writeBlocks();
        stream->writeEndElement();
    }

}

void ProjectSaver::writeBlocks()
{
    PageCollection * pc = PageCollection::instance();
    for (int i = 0; i < pc->blockCount(); i++) {
        stream->writeStartElement(URI, "block");
        Block b =pc->getBlock(i);
        stream->writeAttribute(URI, "left", QString::number(b.left()));
        stream->writeAttribute(URI, "top", QString::number(b.top()));
        stream->writeAttribute(URI, "width", QString::number(b.width()));
        stream->writeAttribute(URI, "height", QString::number(b.height()));
        // stream->writeAttribute(URI, "language", "eng");
        stream->writeEndElement();
    }
}

void ProjectSaver::writeSettings()
{
    stream->writeStartElement(URI, "settings");
    Settings * settings = Settings::instance();
    QString engine;
    if (settings->getSelectedEngine() == UseCuneiform)
        engine = "cuneiform";
    if (settings->getSelectedEngine() == UseTesseract)
        engine = "tesseract";
    stream->writeAttribute(URI, "engine", engine);
    stream->writeAttribute(URI, "defaultlanguage", settings->getLanguage());
    stream->writeEndElement();
}

QString ProjectSaver::copyFile(const QString &source)
{
    QFileInfo fi(source);
    QString dir = fi.absolutePath();
    if (!dir.endsWith("/"))
        dir = dir + "/";
    QString base = fi.baseName();
    QString fileName = base+".png";
    if (dir == directory)
        return fileName;
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

ProjectLoader::ProjectLoader(QObject *parent):   QObject(parent)
{
}

bool ProjectLoader::load(const QString &dir)
{
    directory = dir;
    if (!directory.endsWith("/")) directory = directory + "/";
    QString fileName = directory+"yagf_project.xml";
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    stream = new QXmlStreamReader(&f);
    stream->setNamespaceProcessing(true);
    if (!readSettings())
        return false;
    if (!readPages())
        return false;
    f.close();
    return true;
}

bool ProjectLoader::readSettings()
{
    Settings * settings = Settings::instance();
    if (!readNextElement())
        return false;
    QStringRef n;

    while ((n = stream->name()) != "settings")
         if (!readNextElement())
            return false;
    QStringRef engine = stream->attributes().value(URI, "engine");
    if (engine == "tesseract")
        settings->setSelectedEngine(UseTesseract);
    if (engine == "cuneiform")
        settings->setSelectedEngine(UseCuneiform);
    emit engineChanged();
    QString language = stream->attributes().value(URI, "defaultlanguage").toString();
    if (!language.isEmpty())
        settings->setLanguage(language);
    emit languageChanged();
    return true;
}

void ProjectLoader::loadPage()
{
    QString image = stream->attributes().value(URI, "image").toString();
    QString fn = directory + image;
    bool oldcl = Settings::instance()->getCropLoaded();
    Settings::instance()->setCropLoaded(false);
    PageCollection * pc = PageCollection::instance();
    Settings::instance()->setCropLoaded(oldcl);
    pc->appendPage(fn);
    QString value = stream->attributes().value(URI, "rotation").toString();
    if (!value.isEmpty()) {
        pc->setRotation(value.toDouble());
    }
    value = stream->attributes().value(URI, "deskewed").toString();
    if (!value.isEmpty()) {
        pc->setDeskewed(value.endsWith("true", Qt::CaseInsensitive) ? true : false);
    }
    value = stream->attributes().value(URI, "preprocessed").toString();
    if (!value.isEmpty()) {
        pc->setPreprocessed(value.endsWith("true", Qt::CaseInsensitive) ? true : false);
    }

}

bool ProjectLoader::readPages()
{
    if (!readNextElement())
       return false;
    QString name;
    if ((name = stream->name().toString()) != "page")
        return false;
    while(stream->name() == "page") {
        loadPage();

        if (!readBlocks())
           break;
     }
    PageCollection::instance()->reloadPage();
    return true;

}

bool ProjectLoader::readBlocks()
{
    if (!readNextElement())
        return false;
    while (stream->name() == "block") {
        int top = stream->attributes().value(URI, "top").toString().toInt();
        int left = stream->attributes().value(URI, "left").toString().toInt();
        int width = stream->attributes().value(URI, "width").toString().toInt();
        int height = stream->attributes().value(URI, "height").toString().toInt();
        PageCollection::instance()->addBlock(QRect(left, top, width, height));
        if (!readNextElement())
            return false;
    }
    return true;
}

bool ProjectLoader::readNextElement()
{
    while (!stream->readNextStartElement())
        if (stream->atEnd())
            return false;
    return true;
}
