#include "projectsaver.h"
#include "tpagecollection.h"
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
    QString fileName = dir+"/yagf_project.xml";
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    stream = new QXmlStreamWriter(&f);
    stream->setAutoFormatting(true);
    stream->writeStartDocument();
    stream->writeStartElement(URI, "yagf");
    stream->writeAttribute(URI, "version", VERSION);
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

QString ProjectSaver::copyFile(const QString &source, const QString &destDir)
{
    QFileInfo fi(source);
    QString dir = fi.absolutePath();
    if (!dir.endsWith("/"))
        dir = dir + "/";
    if (dir == destDir)
        return source;
    QString base = fi.baseName();
    QString ext = fi.suffix();
    QString newName = destDir + base + ".png";
    if (ext.endsWith(".png", Qt::CaseInsensitive)) {
        if (QFile::copy(source, newName))
            return newName;
        else
            return "";
    } else {
        QImage image(source);
        if (image.save(newName))
            return newName;
        else
            return "";

    }
    return "";
}

