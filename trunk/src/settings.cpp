/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "settings.h"
#include "utils.h"
#include <QProcessEnvironment>
#include <QDir>
#include <QLocale>
#include <QVariant>

Settings * Settings::m_instance = NULL;

Settings::Settings()
{
    makeLanguageMaps();
}

Settings::Settings(const Settings &)
{
}

Settings::~Settings()
{
}

Settings *Settings::instance()
{
    if (!m_instance)
        m_instance = new Settings();
    return m_instance;
}

void Settings::readSettings(const QString &path)
{
    mPath = path;
    mPath = mPath.append("yagf.ini");
    settings = new QSettings(mPath, QSettings::IniFormat);
    lastDir = settings->value("mainwindow/lastDir").toString();
    lastOutputDir = settings->value("mainwindow/lastOutputDir").toString();
    QString defEngine;
    if (findProgram("tesseract")&&(!findProgram("cuneiform")))
        defEngine = "tesseract";
    else
        defEngine = "cuneiform";
    QString engine = settings->value("ocr/engine", QVariant(defEngine)).toString();
    if (engine == "cuneiform")
        selectedEngine = UseCuneiform;
    else
        selectedEngine = UseTesseract;
    language = settings->value("ocr/language",  selectDefaultLanguageName()).toString();
    //selectLangsBox->setCurrentIndex(selectLangsBox->findData(QVariant(language)));
    outputFormat = settings->value("ocr/outputFormat", QString("text")).toString();
    if (outputFormat == "") outputFormat = "text";
    checkSpelling = settings->value("mainWindow/checkSpelling", bool(true)).toBool();
    bool ok;
    fontSize = settings->value("mainWindow/fontSize", int(12)).toInt(&ok);
    findTessDataPath();
    tessdataPath = settings->value("ocr/tessData", QVariant(tessdataPath)).toString();
    if (tessdataPath.isEmpty())
        findTessDataPath();
    cropLoaded =  settings->value("processing/crop1", QVariant(true)).toBool();
    size = settings->value("mainwindow/size", QSize(800, 600)).toSize();
    iconSize = settings->value("mainwindow/iconSize", QSize(48, 48)).toSize();
    position = settings->value("mainwindow/pos", QPoint(0, 0)).toPoint();
    fullScreen = settings->value("mainwindow/fullScreen", QVariant(false)).toBool();

}

void Settings::writeSettings()
{
    settings->setValue("mainwindow/size", size);
    settings->setValue("mainwindow/iconSize", iconSize);
    settings->setValue("mainwindow/pos", position);
    settings->setValue("mainwindow/fullScreen", fullScreen);
    settings->setValue("mainwindow/lastDir", lastDir);
    settings->setValue("mainWindow/checkSpelling", checkSpelling);
    settings->setValue("mainwindow/lastOutputDir", lastOutputDir);
    settings->setValue("mainWindow/fontSize", fontSize);
    settings->setValue("ocr/language", language);
    //settings->setValue("ocr/singleColumn", singleColumn);
    settings->setValue("ocr/outputFormat", outputFormat);
    QString engine = selectedEngine == UseCuneiform ? QString("cuneiform") : QString("tesseract");
    settings->setValue("ocr/engine", engine);
    settings->setValue("ocr/tessData", tessdataPath);
    settings->setValue("processing/crop1", cropLoaded);
    settings->sync();
}

QString Settings::getLanguage()
{
    return language;
}

QString Settings::getOutputFormat()
{
    return outputFormat;
}

QString Settings::getLastDir()
{
    return lastDir;
}

QString Settings::getLastOutputDir()
{
    return lastOutputDir;
}

bool Settings::getCheckSpelling()
{
    return checkSpelling;
}

QString Settings::getTessdataPath()
{
    if (!tessdataPath.endsWith("/"))
        tessdataPath = tessdataPath.append("/");
    return tessdataPath;
}

SelectedEngine Settings::getSelectedEngine()
{
    return selectedEngine;
}

QSize Settings::getSize()
{
    return size;
}

QPoint Settings::getPosition()
{
    return position;
}

bool Settings::getFullScreen()
{
    return fullScreen;
}

int Settings::getFontSize()
{
    return fontSize;
}

bool Settings::getCropLoaded()
{
    return cropLoaded;
}

void Settings::setLanguage(const QString &value)
{
    language = value;
}

void Settings::setOutputFormat(const QString &value)
{
    outputFormat = value;
}

void Settings::setLastDir(const QString &value)
{
    lastDir = value;
}

void Settings::setLastOutputDir(const QString &value)
{
    lastOutputDir = value;
}

void Settings::setCheckSpelling(const bool value)
{
    checkSpelling = value;
}

void Settings::setTessdataPath(const QString &value)
{
    tessdataPath = value;
}

void Settings::setSelectedEngine(const SelectedEngine value)
{
    selectedEngine = value;
}

void Settings::setSize(const QSize &value)
{
    size = value;
}

void Settings::setPosition(const QPoint &value)
{
    position = value;
}

void Settings::setFullScreen(const bool value)
{
    fullScreen = value;
}

void Settings::setFontSize(const int &value)
{
    fontSize = value;
}

void Settings::setCropLoaded(const bool value)
{
    cropLoaded = value;
}

QString Settings::workingDir()
{
    QString wDir = QDir::homePath();
    if (!wDir.endsWith("/"))
        wDir += '/';
    QDir d(wDir + ".config");
    if (d.exists()) wDir += ".config/";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    wDir = env.value("XDG_CONFIG_HOME", wDir);
    if (!wDir.endsWith("/"))
        wDir += '/';
    wDir += "yagf/";
    QDir dir(wDir);
    if (!dir.exists())
        dir.mkdir(wDir);
    return wDir;
}

void Settings::startLangPair()
{
    lpi = 0;
}

bool Settings::getLangPair(QString &full, QString &abbr)
{
    QMap<QString, QString> * map;
    if (selectedEngine == UseCuneiform)
        map = &cuMap;
    if (selectedEngine == UseTesseract)
        map = &tesMap;
    full = map->keys().at(lpi);
    abbr = map->value(full);
    lpi++;
    if (lpi < map->count())
        return true;
    return false;
}

void Settings::setProjectDir(const QString &dir)
{
    projectDir = dir;
}

QString Settings::getProjectDir()
{
    return projectDir;
}

void Settings::makeLanguageMaps()
{
    cuMap.insert("Bulgarian", "bul");
    cuMap.insert("Czech", "cze");
    cuMap.insert("Danish", "dan");
    cuMap.insert("Dutch", "dut");
    cuMap.insert("English", "eng");
    cuMap.insert("French", "fra");
    cuMap.insert("German", "ger");
    cuMap.insert("Hungarian", "hun");
    cuMap.insert("Italian", "ita");
    cuMap.insert("Latvian", "lav");
    cuMap.insert("Lithuanian", "lit");
    cuMap.insert("Polish", "pol");
    cuMap.insert("Portuguese", "por");
    cuMap.insert("Romanian", "rum");
    cuMap.insert("Russian", "rus");
    cuMap.insert("Russian-English", "ruseng");
    cuMap.insert("Spanish", "spa");
    cuMap.insert("Serbian", "srp");
    cuMap.insert("Slovenian", "slo");
    cuMap.insert("Swedish", "swe");
    cuMap.insert("Ukrainian", "ukr");

    tesMap.insert("Bulgarian", "bul");
    tesMap.insert("Czech", "ces");
    tesMap.insert("Danish", "dan");
    tesMap.insert("Dutch", "nld");
    tesMap.insert("English", "eng");
    tesMap.insert("Finnish", "fin");
    tesMap.insert("French", "fra");
    tesMap.insert("German", "deu");
    tesMap.insert("German Gothic", "gerf");
    tesMap.insert("Greek", "ell");
    tesMap.insert("Hebrew", "heb");
    tesMap.insert("Hungarian", "hun");
    tesMap.insert("Italian", "ita");
    tesMap.insert("Latvian", "lav");
    tesMap.insert("Lithuanian", "lit");
    tesMap.insert("Norwegian", "nor");
    tesMap.insert("Polish", "pol");
    tesMap.insert("Portuguese", "por");
    tesMap.insert("Romanian", "ron");
    tesMap.insert("Russian", "rus");
    tesMap.insert("Serbian", "srp");
    tesMap.insert("Slovenian", "slv");
    tesMap.insert("Slovak", "slk");
    tesMap.insert("Spanish", "spa");
    tesMap.insert("Swedish", "swe");
    tesMap.insert("Swedish Gothic", "swef");
    tesMap.insert("Turkish", "tur");
    tesMap.insert("Ukrainian", "ukr");

}

void Settings::findTessDataPath()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains("TESSDATA_PREFIX")) {
        tessdataPath = env.value("TESSDATA_PREFIX");
        return;
    }
    QDir dir;
    dir.setPath("/usr/share/tessdata");
    if (dir.exists()) {
        tessdataPath = "/usr/share/";
        return;
    }
    dir.setPath("/usr/local/share/tessdata");
    if (dir.exists()) {
        tessdataPath = "/usr/local/share/";
        return;
    }
    dir.setPath("/usr/local/share/tesseract-ocr/tessdata");
    if (dir.exists()) {
        tessdataPath = "/usr/local/share/tesseract-ocr/";
        return;
    }
    dir.setPath("/usr/share/tesseract-ocr/tessdata");
    if (dir.exists()) {
        tessdataPath = "/usr/share/tesseract-ocr/";
        return;
    }
    tessdataPath.clear();
    return;
}

QString Settings::selectDefaultLanguageName()
{
    QLocale loc = QLocale::system();
    QString name = "";
    QMap<QString, QString> * map;
    if (selectedEngine == UseCuneiform)
        map = &cuMap;
    if (selectedEngine == UseTesseract)
        map = &tesMap;
    switch (loc.language()) {
            case QLocale::Bulgarian:
                name = map->value("Bulgarian");
                break;
            case QLocale::Czech:
                name = map->value("Czech");
                break;
            case QLocale::Danish:
                name = map->value("Danish");
                break;
            case QLocale::German:
                name = map->value("German");
                break;
            case QLocale::Dutch:
                name = map->value("Dutch");
                break;
            case QLocale::Russian:
                name = map->value("Russian");
                break;
            case QLocale::English:
                name = "eng";
                break;
            case QLocale::Spanish:
                name = map->value("Spanish");
                break;
            case QLocale::French:
                name = map->value("French");
                break;
            case QLocale::Hungarian:
                name = map->value("Hungarian");
                break;
            case QLocale::Italian:
                name = map->value("Italian");
                break;
            case QLocale::Latvian:
                name = map->value("Latvian");
                break;
            case QLocale::Lithuanian:
                name = map->value("Lithuanian");
                break;
            case QLocale::Polish:
                name = map->value("Polish");
                break;
            case QLocale::Portuguese:
                name = map->value("Portuguese");
                break;
            case QLocale::Romanian:
                name = map->value("Romanian");
                break;
            case QLocale::Swedish:
                name = map->value("Swedish");
                break;
            case QLocale::Serbian:
                name = map->value("Serbian");
                break;
            case QLocale::Slovenian:
                name = map->value("Slovenian");
                break;
            case QLocale::Slovak:
                name = map->value("Slovak", "eng");
                break;
            case QLocale::Ukrainian:
                name = map->value("Ukrainian");
                break;
            case QLocale::Finnish:
                name = map->value("Finnish", "eng");
                break;
            case QLocale::Greek:
                name = map->value("Greek", "eng");
                break;
            case QLocale::Hebrew:
                name = map->value("Hebrew", "eng");
                break;
            case QLocale::Norwegian:
                name = map->value("Norwegian", "eng");
                break;
            case QLocale::Turkish:
                name = map->value("Turkish", "eng");
                break;
            default:
                break;
        }
    if (name == "")
        name = "eng";
    return name;
}

QSize Settings::getIconSize()
{
    return iconSize;
}

void Settings::setIconSize(const QSize &value)
{
    iconSize = value;
}
