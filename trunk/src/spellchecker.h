/*
    YAGF - cuneiform OCR graphical front-end 
    Copyright (C) 2009 Andrei Borovsky <anb@symmetrica.net>

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

#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QString>
#include <QMap>
#include <aspell.h>

typedef QMap<QString, QString> StringMap;

class QTextEdit;
class QRegExp;
class QTextCursor;

class SpellChecker
{
    public:
    SpellChecker(QTextEdit * textEdit);
    void setLanguage(const QString & lang);
    void spellCheck();
    void checkWord();
    private:
    void _checkWord(QTextCursor * cursor);
    QTextEdit * m_textEdit;
    QRegExp * m_regExp;
    QString m_lang1;
    QString m_lang2;
    StringMap * m_map;
    AspellConfig * spell_config1;
    AspellConfig * spell_config2;
    AspellSpeller * spell_checker1;
    AspellSpeller * spell_checker2;

};

#endif // SPELLCHECKER_H