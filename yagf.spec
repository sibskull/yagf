Summary: Yet Another Graphic Front-end for Cuneiform
Name: yagf
Version: 0.8.5
Release: alt1
License: GPL
Group: Graphics
URL: http://symmetrica.net/cuneiform-linux/yagf-ru.html

Source: http://symmetrica.net/cuneiform-linux/yagf-%{version}-Source.tar.gz

BuildRequires: gcc-c++ libqt4-devel
BuildRequires: cmake libaspell-devel
Requires: cuneiform libaspell

Packager: Andrey Cherepanov <cas@altlinux.org>

%description
YAGF is a graphical front-end for cuneiform OCR tool.
With YAGF you can open already scanned image files or obtain new images
via XSane (scanning results are automatically passed to YAGF).
Once you have a scanned image you can prepare it for recognition,
select particular image areas for recognition,
set the recognition language and so no.
Recognized text is displayed in a editor window where it can be corrected,
saved to disk or copied to clipboard.
YAGF also provides some facilities for a multi-page recognition
(see the online help for more details).
Authors:
--------
    Andrei Borovsky <anb@symmetrica.net>

%prep
%setup -q
subst "s,/usr/local,%buildroot/usr/,g" ./CMakeLists.txt

%build
cmake ./
%make

%install
make install DESTDIR=%buildroot

%files 
%doc README COPYING DESCRIPTION AUTHORS ChangeLog
%_bindir/%name
%_libdir/%name/libxspreload.so
%_datadir/%name/translations/*.qm
%_datadir/pixmaps/yagf.png
%_datadir/icons/hicolor/96x96/apps/yagf.png
%_datadir/applications/YAGF.desktop

%changelog
* Fri Jan 28 2011 Andrey Cherepanov <cas@altlinux.org> 0.8.5-alt1
- Version 0.8.5

* Mon Jan 17 2011 Andrey Cherepanov <cas@altlinux.org> 0.8.3-alt1
- Version 0.8.3

* Tue Aug 18 2009 Andrey Cherepanov <cas@altlinux.org> 0.8.1-alt1
- Version 0.8.1

* Mon Jul 06 2009 Andrey Cherepanov <cas@altlinux.org> 0.5.0-alt1
- First version for Sisyphus

