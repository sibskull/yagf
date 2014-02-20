#ifndef BINARIZE_H
#define BINARIZE_H

/*
    YAGF - cuneiform and tesseract OCR graphical front-ends
    Copyright (C) 2009-2014 Andrei Borovsky <anb@symmetrica.net>
    The original code is
	Copyright (C) 2008-2009  Timothy B. Terriberry (tterribe@xiph.org)
  
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

#if !defined(_qrcode_binarize_H)
# define _qrcode_binarize_H (1)

void qr_image_cross_masking_median_filter(unsigned char *_img,
 int _width,int _height);

void qr_wiener_filter(unsigned char *_img,int _width,int _height);

/*Binarizes a grayscale image.*/
void qr_binarize(unsigned char *_img,int _width,int _height);
unsigned char *qr_binarize1(const unsigned char *_img,int _width,int _height);
#endif

#endif // BINARIZE_H
