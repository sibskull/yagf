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

#include "sidebar.h"
#include "droplabel.h"
#include "popplerdialog.h"
#include "pdfextractor.h"
#include "pdf2ppt.h"
#include "ghostscr.h"
#include "configdialog.h"
#include "advancedconfigdialog.h"
#include "mainform.h"
#include "tpagecollection.h"
#include "scanner.h"
#include "projectmanager.h"
#include <signal.h>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QCloseEvent>
#include <QPainter>
#include <QSize>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QProcess>
#include <QFile>
#include <QByteArray>
#include <QRect>
#include <QRectF>
#include <QStatusBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QImage>
#include <QDesktopServices>
#include <QUrl>
#include <QRegExp>
#include <QClipboard>
#include <QMap>
#include <QWidgetAction>
#include "qgraphicsinput.h"
#include "utils.h"
#include "qxtunixsignalcatcher.h"
#include "PageAnalysis.h"
#include <QTextCodec>
#include <QCheckBox>
#include <QEvent>
#include <QCursor>
#include <QLineEdit>

const QString outputBase = "output";
const QString outputExt = ".txt";
const QString inputFile = "input.bmp";
const QString outputFile = "output.txt";
const QString scanOutputFile = "input.png";


MainForm::MainForm(QWidget *parent): QMainWindow(parent)
{
    setupUi(this);

    pages = PageCollection::instance();

    setWindowTitle("YAGF");
    //spellChecker = new SpellChecker(textEdit);
    textEdit->enumerateDicts();

    frame->show();
    //toolBar->addWidget(label1);
    //toolBar->addWidget(selectLangsBox);
    graphicsInput = new QGraphicsInput(QRectF(0, 0, 2000, 2000), graphicsView) ;
    graphicsInput->addToolBarAction(actionHideShowTolbar);
    graphicsInput->addToolBarAction(this->actionTBLV);
    graphicsInput->addToolBarAction(this->actionSmaller_view);
    graphicsInput->addToolBarSeparator();
    graphicsInput->addToolBarAction(actionRotate_90_CCW);
    graphicsInput->addToolBarAction(actionRotate_180);
    graphicsInput->addToolBarAction(actionRotate_90_CW);
    graphicsInput->addToolBarAction(actionPrepare_Page);
    graphicsInput->addToolBarAction(actionDeskew);
    graphicsInput->addToolBarSeparator();
    graphicsInput->addToolBarAction(actionSelect_Text_Area);
    graphicsInput->addToolBarAction(actionSelect_multiple_blocks);
    graphicsInput->addToolBarAction(ActionClearAllBlocks);

    label->setListWidget(sideBar);

    connect(sideBar, SIGNAL(pageSelected(int)), pages, SLOT(pageSelected(int)));
    connect(label, SIGNAL(pageRemoved(int)), pages, SLOT(pageRemoved(int)));

    statusBar()->show();
    useXSane = TRUE;
    scanner = NULL;
    //rotation = 0;
    m_menu = new QMenu(graphicsView);
    ifCounter = 0;

    connect(actionOpen, SIGNAL(triggered()), this, SLOT(loadImage()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this, SIGNAL(windowShown()), this, SLOT(onShowWindow()), Qt::QueuedConnection);
    connect(actionScan, SIGNAL(triggered()), this, SLOT(scanImage()));
    connect(actionPreviousPage, SIGNAL(triggered()), this, SLOT(loadPreviousPage()));
    connect(actionNextPage, SIGNAL(triggered()), this, SLOT(loadNextPage()));
    connect(actionRecognize, SIGNAL(triggered()), this, SLOT(recognize()));
    connect(action_Save, SIGNAL(triggered()), textEdit, SLOT(saveText()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDlg()));
    connect(actionOnlineHelp, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(actionCopyToClipboard, SIGNAL(triggered()),textEdit, SLOT(copyClipboard()));
    connect(graphicsInput, SIGNAL(rightMouseClicked(int, int, bool)), this, SLOT(rightMouseClicked(int, int, bool)));
    connect(actionSelect_HTML_format, SIGNAL(triggered()), this, SLOT(selectHTMLformat()));

    connect(graphicsInput, SIGNAL(increaseMe()), this, SLOT(enlargeButtonClicked()));
    connect(graphicsInput, SIGNAL(decreaseMe()), this, SLOT(decreaseButtonClicked()));
    connect(sideBar, SIGNAL(filesDropped(QStringList)), SLOT(loadFiles(QStringList)));
    connect(pages, SIGNAL(loadPage()), this, SLOT(loadPage()));
    connect(graphicsInput, SIGNAL(blockCreated(QRect)), pages, SLOT(addBlock(QRect)));
    connect(graphicsInput, SIGNAL(deleteBlock(QRect)), pages, SLOT(deleteBlock(QRect)));
    connect(sideBar, SIGNAL(fileRemoved(int)), pages, SLOT(pageRemoved(int)));
    connect (pages, SIGNAL(addSnippet(int)), this, SLOT(addSnippet(int)));

    selectLangsBox = new QComboBox();
    selectLangsBox->setToolTip(trUtf8("Recognition language"));
    toolBar->insertWidget(actionRecognize, selectLangsBox);
   // connect(selectLangsBox->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(LangTextChanged(QString)));

    initSettings();
    engineLabel = new QLabel();
    statusBar()->addPermanentWidget(engineLabel, 0);
    if (settings->getSelectedEngine() == UseCuneiform) {
        //fillLanguagesBoxCuneiform();
        engineLabel->setText(trUtf8("Using Cuneiform"));

    }
    if (settings->getSelectedEngine() == UseTesseract) {
        //fillLanguagesBoxTesseract();
        engineLabel->setText(trUtf8("Using Tesseract"));
    }
    fillLangBox();
    delTmpFiles();
    QXtUnixSignalCatcher::connectUnixSignal(SIGUSR2);
    ba = new QByteArray();
    connect(QXtUnixSignalCatcher::catcher(), SIGNAL(unixSignal(int)), this, SLOT(readyRead(int)));

    QPixmap l_cursor;
    l_cursor.load(":/resize.png");
    resizeCursor = new QCursor(l_cursor);
    graphicsInput->setMagnifierCursor(resizeCursor);
    l_cursor.load(":/resize_block.png");
    resizeBlockCursor = new QCursor(l_cursor);
   // textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);

    this->sideBar->show();
    //connect(sideBar, SIGNAL(fileSelected(const QString &)), this, SLOT(fileSelected(const QString &)));

    connect(actionRecognize_All_Pages, SIGNAL(triggered()), this, SLOT(recognizeAll()));

    QPixmap pm;
    pm.load(":/align.png");
    //alignButton->setIcon(pm);
    pm.load(":/undo.png");
    //unalignButton->setIcon(pm);
    //connect(unalignButton, SIGNAL(clicked()), this, SLOT(unalignButtonClicked()));

    //clearBlocksButton->setDefaultAction(ActionClearAllBlocks);
    loadFromCommandLine();
    emit windowShown();

    pdfx = NULL;
    if (findProgram("pdftoppm")) {
        pdfx = new PDF2PPT();
    } else
    if (findProgram("gs")) {
         pdfx = new GhostScr();
    }

    if (pdfx) {
        connect(pdfx, SIGNAL(addPage(QString)), this, SLOT(addPDFPage(QString)), Qt::QueuedConnection);
        connect (pdfx, SIGNAL(finished()), this, SLOT(finishedPDF()));
    }

    pdfPD.setWindowTitle("YAGF");
    pdfPD.setLabelText(trUtf8("Importing pages from the PDF document..."));
    pdfPD.setCancelButtonText(trUtf8("Cancel"));
    pdfPD.setMinimum(-1);
    pdfPD.setMaximum(-1);
    pdfPD.setWindowIcon(QIcon(":/yagf.png"));
    if (pdfx)
        connect(&pdfPD, SIGNAL(canceled()), pdfx, SLOT(cancel()));
}

void MainForm::onShowWindow()
{
    // actionCheck_spelling->setCheckable(true);
    connect(selectLangsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(newLanguageSelected(int)));
    selectLangsBox->setCurrentIndex(selectLangsBox->findData(QVariant(settings->getLanguage())));
    //spellChecker->setLanguage(language);
    //actionCheck_spelling->setEnabled(spellChecker->spellCheck());
}

void MainForm::loadFromCommandLine()
{
    QStringList sl = QApplication::arguments();
    if (sl.count() > 1) {
        if (QFile::exists(sl.at(1)))
            loadFile(sl.at(1));
        for (int i = 2; i < sl.count(); i++) {
            QApplication::processEvents();
            if (QFile::exists(sl.at(i)))
                loadFile(sl.at(i));
        }
        sideBar->select(sl.at(1));
    }
}

void MainForm::loadFiles(QStringList files)
{
    for (int i = 0; i < files.count(); i++)
        if (QFile::exists(files.at(i)))
            loadFile(files.at(i));
}

void MainForm::LangTextChanged(const QString &text)
{
    if (selectLangsBox->findText(text, Qt::MatchStartsWith) < 0)
        selectLangsBox->lineEdit()->setText("");

}

void MainForm::showConfigDlg()
{
    ConfigDialog dialog(this);
    SelectedEngine ose = settings->getSelectedEngine();
    if (settings->getSelectedEngine() == UseCuneiform)
        dialog.setSelectedEngine(0);
    else
        dialog.setSelectedEngine(1);
    dialog.setTessDataPath(settings->getTessdataPath());
    if (dialog.exec()) {
        settings->setSelectedEngine(dialog.selectedEngine() == 0 ? UseCuneiform : UseTesseract);
        settings->setTessdataPath(dialog.tessdataPath());
        if (settings->getSelectedEngine() != ose) {
            QString oldLang = selectLangsBox->currentText();
            selectLangsBox->clear();
            if (settings->getSelectedEngine() == UseCuneiform) {
                engineLabel->setText(trUtf8("Using Cuneiform"));
            }
            if (settings->getSelectedEngine() == UseTesseract) {
                engineLabel->setText(trUtf8("Using Tesseract"));
            }
            fillLangBox();
            int newIndex = selectLangsBox->findText(oldLang);
            if (newIndex >= 0) {
                selectLangsBox->setCurrentIndex(newIndex);
                settings->setLanguage(selectLangsBox->itemData(newIndex).toString());
            } else {
                settings->setLanguage("eng");
                for (int i = 0; i < selectLangsBox->count(); i++) {
                    QString s = selectLangsBox->itemData(i).toString();
                    if (s == "eng") {
                        newLanguageSelected(i);
                        selectLangsBox->setCurrentIndex(i);
                        break;
                    }
                }
            }

        }
    }
}

void MainForm::importPDF()
{
    if (!pdfx) {
        QMessageBox::critical(this, trUtf8("No PDF converter installed"), trUtf8("No compatible PDF converter software could be found. Please install either the pdftoppm utility or the GhostScript package (from this the gs command will be required)."));
        return;
    }
    PopplerDialog dialog(this);
    if (dialog.exec()) {
        pdfx->setSourcePDF(dialog.getPDFFile());
        if (pdfx->getSourcePDF().isEmpty()) {
            QMessageBox::information(this, trUtf8("Error"), trUtf8("PDF file name may not be empty"));
            return;
        }
        pdfx->setStartPage(dialog.getStartPage());
        pdfx->setStopPage(dialog.getStopPage());
        bool doit = true;
        QString outputDir;
        while (doit) {
            outputDir = QFileDialog::getExistingDirectory(this, trUtf8("Select an existing directory for output or create some new one")); //, QString(""), QString(), (QString*) NULL, QFileDialog::ShowDirsOnly);
            if (outputDir.isEmpty())
                return;
            QDir dir(outputDir);
            if (dir.count() > 2)
                QMessageBox::warning(this, trUtf8("Selecting Directory"), trUtf8("The selected directory is not empty"));
            else doit = false;
        }
        pdfx->setOutputDir(outputDir);
        QApplication::processEvents();
        pdfPD.setWindowFlags(Qt::Dialog|Qt::WindowStaysOnTopHint);
        pdfPD.show();
        pdfPD.setMinimum(0);
        pdfPD.setMaximum(100);
        QApplication::processEvents();
        pdfx->exec();
    }
}

void MainForm::addPDFPage(QString pageName)
{
    pages->appendPage(pageName);
    pdfPD.setValue(pdfPD.value()+1);
}

void MainForm::finishedPDF()
{
    pdfPD.hide();
}

void MainForm::loadImage()
{
    QFileDialog dialog(this,
                       trUtf8("Open Image"), settings->getLastDir(), trUtf8("Image Files (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.gif *.pnm *.pgm *.pbm *.ppm)"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (dialog.exec()) {
        QStringList fileNames;
        fileNames = dialog.selectedFiles();
        settings->setLastDir(dialog.directory().path());
        if (fileNames.count() > 0)
         loadFile(fileNames.at(0), true);
        if (!pages->pageValid())
            return;
        for (int i = 1; i < fileNames.count(); i++) {
            loadFile(fileNames.at(i), false);
        }
//        if (fileNames.count() > 0)
//            pages->makePageCurrent(0);
    }
}

void MainForm::singleColumnButtonClicked()
{
    //singleColumn = singleColumnButton->isChecked();
}

void MainForm::closeEvent(QCloseEvent *event)
{
    if (!textEdit->textSaved()) {
        QPixmap icon;
        icon.load(":/info.png");

        QMessageBox messageBox(QMessageBox::NoIcon, "YAGF", trUtf8("There is an unsaved text in the editor window. Do you want to save it?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
        messageBox.setIconPixmap(icon);
        int result = messageBox.exec();
        if (result == QMessageBox::Save)
            textEdit->saveText();
        else if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }

    }
    if (scanner) {
        delete scanner;
        scanner = NULL;
    }
    settings->setSize(size());
    settings->setPosition(pos());
    settings->setFullScreen(isFullScreen());
    settings->writeSettings();
    delTmpFiles();
    event->accept();
    QXtUnixSignalCatcher::catcher()->disconnectUnixSugnals();
    pages->clear();
}

void MainForm::rotateCWButtonClicked()
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    pages->rotate90CW();
    setCursor(oldCursor);
}

void MainForm::rotateCCWButtonClicked()
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    pages->rotate90CCW();
    setCursor(oldCursor);
}
void MainForm::rotate180ButtonClicked()
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    pages->rotate180();
    setCursor(oldCursor);
}

void MainForm::enlargeButtonClicked()
{
    pages->makeLarger();
}

void MainForm::decreaseButtonClicked()
{
    pages->makeSmaller();
}

void MainForm::initSettings()
{
    settings = Settings::instance();
    settings->readSettings(settings->workingDir());
    settings->writeSettings();
    if (settings->getFullScreen())
        showFullScreen();
    else {
        move(settings->getPosition());
        resize(settings->getSize());
    }
    actionCheck_spelling->setChecked(settings->getCheckSpelling());
    actionSelect_HTML_format->setChecked(settings->getOutputFormat() != "text");

    QList<int> li;
    li.append(1);
    li.append(1);
    splitter->setSizes(li);
    toolBar->setIconSize(settings->getIconSize());
}

void MainForm::newLanguageSelected(int index)
{
    settings->setLanguage(selectLangsBox->itemData(index).toString());
    actionCheck_spelling->setEnabled(textEdit->hasDict(settings->getLanguage()));
    if (settings->getCheckSpelling()) {
        settings->setCheckSpelling(textEdit->spellCheck(settings->getLanguage()));
        //actionCheck_spelling->setEnabled(checkSpelling);
        actionCheck_spelling->setChecked(settings->getCheckSpelling());
    }

}

void MainForm::scanImage()
{

    if (useXSane) {
        if (scanner) {
            delete scanner;
        }
        ScannerFactory * sf = new ScannerFactory();
        scanner = sf->createScannerFE("xsane");
        if (scanner == NULL) {
            QMessageBox::warning(this, trUtf8("Scanning is impossible"), trUtf8("No scanning front-end is found. Please install XSane in order to perform scanning."));
            return;
        }
        scanner->setOutputFile(settings->workingDir() + scanOutputFile);
        delete sf;
        scanner->exec();

    }
}

void MainForm::loadFile(const QString &fn, bool loadIntoView)
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);

    if (pages->appendPage(fn)) {
        if (loadIntoView) {
            pages->makePageCurrent(pages->count()-1);
            loadPage();
            sideBar->item(sideBar->count()-1)->setSelected(true);
        }
    } else {
        QMessageBox::warning(this, trUtf8("Failed to Load Image"), fn);
    }
    setCursor(oldCursor);
}

void MainForm::delTmpFiles()
{

    QDir dir(settings->workingDir());
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    for (uint i = 0; i < dir.count(); i++) {
        if (dir[i].endsWith("jpg") || dir[i].endsWith("bmp") || dir[i].endsWith("png") || dir[i].endsWith("txt"))
            dir.remove(dir[i]);
    }
    delTmpDir();
}

void MainForm::loadNextPage()
{
}

void MainForm::loadPreviousPage()
{
}

// TODO: think on blocks/page recognition

bool MainForm::useTesseract(const QString &inputFile)
{
    QProcess proc;
    proc.setWorkingDirectory(settings->workingDir());
    QStringList sl;
    sl.append(inputFile);
    sl.append(outputBase);
    sl.append("-l");
    sl.append(settings->getLanguage());
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("TESSDATA_PREFIX", settings->getTessdataPath());
    proc.setProcessEnvironment(env);
    proc.start("tesseract", sl);
    proc.waitForFinished(-1);
    if (proc.exitCode()) {
        QByteArray stdout = proc.readAllStandardOutput();
        QByteArray stderr = proc.readAllStandardError();
        QString output = QString(stdout) + QString(stderr);
        QMessageBox::critical(this, trUtf8("Starting tesseract failed"), trUtf8("The system said: ") + (output != "" ? output : trUtf8("program not found")));
        return false;
    }
    return true;
}

bool MainForm::useCuneiform(const QString &inputFile, const QString &outputFile)
{
    QProcess proc;
    proc.setWorkingDirectory(settings->workingDir());
    QStringList sl;
    sl.append("-l");
    sl.append(settings->getLanguage());
    sl.append("-f");
    if (settings->getOutputFormat() == "text")
        sl.append("text");
    else
        sl.append("html");
    sl.append("-o");
    sl.append(settings->workingDir() + outputFile);
    sl.append(settings->workingDir() + inputFile);
    proc.start("cuneiform", sl);
    proc.waitForFinished(-1);
    if (proc.exitCode()) {
        QByteArray stdout = proc.readAllStandardOutput();
        QByteArray stderr = proc.readAllStandardError();
        QString output = QString(stdout) + QString(stderr);
        QMessageBox::critical(this, trUtf8("Starting cuneiform failed"), trUtf8("The system said: ") + (output != "" ? output : trUtf8("program not found")));
        return false;
    }
    return true;
}

void MainForm::recognizeInternal()
{
    if (settings->getSelectedEngine() == UseCuneiform) {
        if (!useCuneiform(inputFile, outputFile))
            return;
    }
    if (settings->getSelectedEngine() == UseTesseract) {
        if (!useTesseract(inputFile))
           return;
    }
    QFile textFile(settings->workingDir() + outputFile);
    textFile.open(QIODevice::ReadOnly);
    QByteArray text = textFile.readAll();
    textFile.close();
    QString textData;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    textData = codec->toUnicode(text); //QString::fromUtf8(text.data());
    if (settings->getOutputFormat() == "text")
        textData.prepend("<meta content=\"text/html; charset=utf-8\" http-equiv=\"content-type\" />");
    textData.replace("<img src=output_files", "");
    textData.replace(".bmp\">", "\"--");
    textData.replace(".bmp>", "");
//       textData.replace("-</p><p>", "");
//        textData.replace("-<br>", "");
    textEdit->append(textData);
    textEdit->append(QString(" "));
    if (settings->getCheckSpelling()) {
        actionCheck_spelling->setChecked(textEdit->spellCheck(settings->getLanguage()));
    }

}

void MainForm::recognize()
{
    QFile::remove(settings->workingDir() + "input*.bmp");
    if (!pages->pageValid()) {
        QMessageBox::critical(this, trUtf8("Error"), trUtf8("No image loaded"));
        return;
    }
    if (!findEngine()) return;
    if (pages->blockCount() > 0) {
        for (int i = 0; i < pages->blockCount(); i++) {
                prepareBlockForRecognition(i);
                recognizeInternal();
        }
    } else {
        preparePageForRecognition();
        recognizeInternal();
    }
}

void MainForm::showAboutDlg()
{
    QPixmap icon;
    icon.load(":/yagf.png");
    QMessageBox aboutBox(QMessageBox::NoIcon, trUtf8("About YAGF"), trUtf8("<p align=\"center\"><b>YAGF - Yet Another Graphical Front-end for cuneiform and tesseract OCR engines</b></p><p align=\"center\">Version %1</p> <p align=\"center\">Ⓒ 2009-2012 Andrei Borovsky</p> This is a free software distributed under GPL v3. Visit <a href=\"http://symmetrica.net/cuneiform-linux/yagf-en.html\">http://symmetrica.net/cuneiform-linux/yagf-en.html</a> for more details.").arg(version), QMessageBox::Ok);
    aboutBox.setIconPixmap(icon);
    QList<QLabel *> labels = aboutBox.findChildren<QLabel *>();
    for (int i = 0; i < labels.count(); i++) {
        QLabel *lab = labels.at(i);
        lab->setTextInteractionFlags(Qt::TextBrowserInteraction);
    }
    aboutBox.setTextFormat(Qt::RichText);
    aboutBox.exec();
}

void MainForm::showHelp()
{
    QDesktopServices::openUrl(QUrl(trUtf8("http://symmetrica.net/cuneiform-linux/yagf-en.html")));
}

void MainForm::readyRead(int sig)
{
    QFile f(settings->workingDir() + scanOutputFile);
    QString newName = QString(settings->workingDir() + "scan-input-%1.png").arg(ifCounter);
    ifCounter++;
    QFileInfo fi(newName);
    if (fi.exists()) {
        QFile f2(newName);
        f2.remove();
    }
    f.rename(newName);
    loadFile(newName);
}


void MainForm::delTmpDir()
{
    QDir dir;
    dir.setPath(settings->workingDir() + "output_files");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    for (uint i = 0; i < dir.count(); i++) {
        if (dir[i].endsWith("jpg") || dir[i].endsWith("bmp"))
            dir.remove(dir[i]);
    }
    dir.rmdir(settings->workingDir() + "output_files");

}


void MainForm::clearTmpFiles()
{
    QFile::remove(settings->workingDir() + "tmp*.bmp");
    QFile f(settings->workingDir()+inputFile);
    f.remove();
    f.setFileName(settings->workingDir()+outputFile);
    f.remove();
}

void MainForm::fillLangBox()
{
    settings->startLangPair();
    QString full;
    QString abbr;
    selectLangsBox->clear();
    while(settings->getLangPair(full, abbr))
        selectLangsBox->addItem(full, QVariant(abbr));

}

void MainForm::preparePageForRecognition()
{
    clearTmpFiles();
    pages->savePageForRecognition(settings->workingDir() + inputFile);
}

void MainForm::prepareBlockForRecognition(const QRect &r)
{
    clearTmpFiles();
    pages->saveBlockForRecognition(r, settings->workingDir() + inputFile);
}

void MainForm::prepareBlockForRecognition(int index)
{
    clearTmpFiles();
    pages->saveBlockForRecognition(index, settings->workingDir() + inputFile);
}

void MainForm::setResizingCusor()
{
    //scrollArea->widget()->setCursor(*resizeBlockCursor);
}

void MainForm::setUnresizingCusor()
{
    //scrollArea->widget()->setCursor(QCursor(Qt::ArrowCursor));
}

void MainForm::loadPage()
{
    //graphicsInput->clearBlocks();
    graphicsInput->loadImage(pages->pixmap());
    QApplication::processEvents();
    for (int i = 0; i < pages->blockCount(); i++)
    graphicsInput->addBlockColliding(pages->getBlock(i));
    QFileInfo fi(pages->fileName());
    setWindowTitle(QString("YAGF - %1").arg(fi.fileName()) );
}

void MainForm::recognizeAll()
{
    if (pages->count() == 0)
        return;
    QProgressDialog progress(trUtf8("Recognizing pages..."), trUtf8("Abort"), 0, pages->count(), this);
    progress.setWindowTitle("YAGF");
    progress.show();
    progress.setValue(0);
    pages->makePageCurrent(-1);
    for (int i = 0; i < pages->count(); i++) {
        progress.setValue(i);
        if (progress.wasCanceled())
            break;
        pages->makeNextPageCurrent();
        recognize();
    }
}

void MainForm::unalignButtonClicked()
{
    /*if (((QSelectionLabel *) scrollArea->widget())->pixmap()->isNull())
        return;
    int rot = ((FileToolBar *) m_toolBar)->getRotation();
    int rrot = ((rot + 45)/90);
    rrot *=90;
    rotateImage(rrot - rot);
    rotation = rrot;*/
}

void MainForm::hideToolBar()
{
    graphicsInput->setToolBarVisible();
}

void MainForm::on_ActionClearAllBlocks_activated()
{
    pages->clearBlocks();
    loadPage();
}

void MainForm::rightMouseClicked(int x, int y, bool inTheBlock)
{
    m_menu->clear();
    m_menu->addAction(ActionClearAllBlocks);
    if (inTheBlock) {
        m_menu->addAction(ActionDeleteBlock);
        m_menu->addAction(actionRecognize_block);
        m_menu->addAction(actionSave_block);
        m_menu->addAction(actionDeskew_by_Block);
    } else {
        m_menu->addAction(actionSelect_Text_Area);
        m_menu->addAction(actionSelect_multiple_blocks);
    }
    QPoint p = graphicsView->mapToGlobal(QPoint(x, y));
    m_menu->move(p);
    m_menu->show();
}

void MainForm::on_ActionDeleteBlock_activated()
{
    QRect r = graphicsInput->getCurrentBlock();
    graphicsInput->deleteCurrentBlock();
    pages->deleteBlock(r);
}

bool MainForm::findEngine() {
	if (settings->getSelectedEngine() == UseCuneiform) {
        	if (!findProgram("cuneiform")) {
        	    if (findProgram("tesseract")) {
        	        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("cuneiform not found, switching to tesseract"));
        	        settings->setSelectedEngine(UseTesseract);
        	    } else {
        	        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("No recognition engine found.\nPlease install either cuneiform or tesseract"));
        	        return false;
        	    }
       		}
     	}
    	if (settings->getSelectedEngine() == UseTesseract) {
        	if (!findProgram("tesseract")) {
        	    if (findProgram("cuneiform")) {
        	        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("tesseract not found, switching to cuneiform"));
        	        settings->setSelectedEngine(UseCuneiform);
       	     } else {
        	        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("No recognition engine found.\nPlease install either cuneiform or tesseract"));
        	        return false;
       	    }
        }
     }
	return true;
}

void MainForm::on_actionRecognize_block_activated()
{
     if (!findEngine()) return;
     if (graphicsInput->getCurrentBlock().isNull())
        return;
    clearTmpFiles();
    pages->saveRawBlockForRecognition(graphicsInput->getCurrentBlock(), settings->workingDir() + inputFile);
    recognizeInternal();
}

/*void MainForm::on_actionRecognize_activated()
{

}*/


void MainForm::on_actionCheck_spelling_triggered()
{
    settings->setCheckSpelling(actionCheck_spelling->isChecked());
}

void MainForm::on_actionSave_current_image_activated()
{
    QCursor oc = cursor();
    setCursor(Qt::WaitCursor);
    QString format;
    QString fn = getFileNameToSaveImage(format);
    if (!fn.isEmpty()) {
        if (!(pages->savePageAsImage(fn, format)))
            QMessageBox::warning(this, QObject::trUtf8("Warning"), QObject::trUtf8("Failed to save the image"));
    }
    setCursor(oc);
}

QString MainForm::getFileNameToSaveImage(QString &format)
{
    QString jpegFilter = QObject::trUtf8("JPEG Files (*.jpg)");
    QString pngFilter = QObject::trUtf8("PNG Files (*.png)");
    QStringList filters;
    format = "JPEG";
    filters << jpegFilter << pngFilter;
    QFileDialog dialog(this,
                       trUtf8("Save Image"), settings->getLastOutputDir());
    dialog.setFilters(filters);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("jpg");
    if (dialog.exec()) {
        if (dialog.selectedNameFilter() == jpegFilter) {
            format = "JPEG";
            dialog.setDefaultSuffix("jpg");
        } else if (dialog.selectedNameFilter() == pngFilter) {
            format = "PNG";
            dialog.setDefaultSuffix("png");
        }
        QStringList fileNames;
        fileNames = dialog.selectedFiles();
        settings->setLastOutputDir(dialog.directory().path());
        return fileNames.at(0);
    }
    return "";
}


MainForm::~MainForm()
{
    delete resizeBlockCursor;
    delete resizeCursor;
    //delete fileChannel;
    delete graphicsInput;
    delete ba;
    delete pdfx;
}

void MainForm::on_actionSave_block_activated()
{
    QString format;
    QString fn = getFileNameToSaveImage(format);
    if (!fn.isEmpty())
        pages->saveBlockForRecognition(graphicsInput->getCurrentBlock(), fn, format);
}

void MainForm::on_actionCheck_spelling_activated()
{
    settings->setCheckSpelling(actionCheck_spelling->isChecked());
    if (settings->getCheckSpelling()) {
        actionCheck_spelling->setChecked(textEdit->spellCheck(settings->getLanguage()));
    } else
        textEdit->unSpellCheck();
}

/*void MainForm::on_alignButton_clicked()
{
    this->AnalizePage();
}*/

void MainForm::on_actionDeskew_activated()
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    pages->deskew();
    setCursor(oldCursor);
}

void MainForm::on_actionSelect_HTML_format_activated()
{
        if (actionSelect_HTML_format->isChecked())
            settings->setOutputFormat("html");
        else
            settings->setOutputFormat("text");
}

void MainForm::pasteimage()
{
    QClipboard *clipboard = QApplication::clipboard();
    QPixmap pm = clipboard->pixmap();
    if (pm.isNull()) return;
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    QString tmpFile = "input-01.png";
    QFileInfo fi(settings->workingDir() + tmpFile);
    while (fi.exists()) {
        QString digits = extractDigits(tmpFile);
        bool result;
        int d = digits.toInt(&result);
        if (!result) return;
        d++;
        if (d < 0) d = 0;
        QString newDigits = QString::number(d);
        while (newDigits.size() < digits.size())
            newDigits = '0' + newDigits;
        tmpFile = tmpFile.replace(digits, newDigits);
        fi.setFile(settings->workingDir(), tmpFile);
    }
    pm.save(fi.absoluteFilePath(), "PNG");
    loadFile(fi.absoluteFilePath());
    setCursor(oldCursor);
}

void MainForm::deskewByBlock()
{
    /*QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    graphicsInput->update();
    QApplication::processEvents();
    if (!graphicsInput->getCurrentBlock().isNull()) {
        QImage img = graphicsInput->getCurrentBlock();*/
        pages->deskew();
    //}
    ///setCursor(oldCursor);
}

void MainForm::selectTextArea()
{
    pages->blockAllText();
}

void MainForm::showAdvancedSettings()
{
    AdvancedConfigDialog dlg;
    dlg.setCrop1(settings->getCropLoaded());
    if (dlg.exec()) {
        settings->setCropLoaded(dlg.doCrop1());
    }
}

void MainForm::addSnippet(int index)
{
    sideBar->addItem((QListWidgetItem *) pages->snippet());
}

void MainForm::preprocessPage()
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    if (!pages->splitPage(true))
        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("Failed to detect text areas on this page.\nThe page possibly lacks contrast. Try to select blocks manually."));
    setCursor(oldCursor);
}

void MainForm::saveProject()
{
    if (settings->getProjectDir().isEmpty()) {
        QString dir = QFileDialog::getExistingDirectory(this, QObject::trUtf8("Select Project Directory"), "");
        if (dir.isEmpty())
            return;
        QCursor oldCursor = cursor();
        QDir dinfo(dir);
        if (dinfo.entryList().count() > 2) {
            QMessageBox::warning(this, trUtf8("Warning"), trUtf8("The selected directoy is not empty. Please select or create another one."));
        } else {
            ProjectSaver ps;
            if (!ps.save(dir))
                QMessageBox::warning(this, trUtf8("Warning"), trUtf8("Failed to save the project."));
            else
                settings->setProjectDir(dir);
        }
        setCursor(oldCursor);
    } else {
        QCursor oldCursor = cursor();
        ProjectSaver ps;
        if (!ps.save(settings->getProjectDir()))
            QMessageBox::warning(this, trUtf8("Warning"), trUtf8("Failed to save the project."));
        setCursor(oldCursor);
    }

 }

void MainForm::loadProject()
{
    pages->clear();
    QString dir = QFileDialog::getExistingDirectory(this, QObject::trUtf8("Select Project Directory"), "");
    if (dir.isEmpty())
        return;
    QCursor oldCursor = cursor();
    ProjectLoader pl;
    if (!pl.load(dir))
        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("Failed to load project."));
    else
        settings->setProjectDir(dir);
    setCursor(oldCursor);

}

void MainForm::selectBlocks()
{
    QCursor oldCursor = cursor();
    setCursor(Qt::WaitCursor);
    if (!pages->splitPage(false))
        QMessageBox::warning(this, trUtf8("Warning"), trUtf8("Failed to detect text areas on this page.\nThe page possibly lacks contrast. Try to select blocks manually."));
    setCursor(oldCursor);
}

void MainForm::setSmallIcons()
{
    QSize s = toolBar->iconSize();
    if (s.height() > 32) {
         s.setHeight(32);
         s.setWidth(32);
    }
    else {
        s.setHeight(48);
        s.setWidth(48);
    }
    toolBar->setIconSize(s);
    settings->setIconSize(s);
}

void MainForm::selectHTMLformat()
{
    if (actionSelect_HTML_format->isChecked())
    settings->setOutputFormat("html");
    else
    settings->setOutputFormat("text");

}
