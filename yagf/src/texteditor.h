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

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QFile>
#include <QTextEdit>
#include "spellchecker.h"


class TextEditor : public QTextEdit
{
    Q_OBJECT
    
public:
    explicit TextEditor(QWidget *parent = 0);
    ~TextEditor();
    bool textSaved();
    bool spellCheck(const QString &lang);
    void unSpellCheck();
    void enumerateDicts();
    bool hasDict(const QString &shname);
public slots:
    void saveText();
protected:
    void keyPressEvent ( QKeyEvent * e );
    void wheelEvent ( QWheelEvent * e );
private slots:
    void contextMenuRequested(const QPoint& point);
    void enlargeFont();
    void decreaseFont();
    void updateSP();
    void replaceWord();
    void copyAvailable(bool yes);
    void textChanged();
    void copyClipboard();
private:
    void saveHtml(QFile *file);
private:
    SpellChecker spellChecker;
    bool hasCopy;
    bool mTextSaved;
};

#endif // TEXTEDITOR_H
