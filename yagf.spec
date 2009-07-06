Summary: Yet Another Graphic Front-end for Cuneiform
Name: yagf
Version: 0.5.0
Release: alt1
License: GPL
Group: Graphics
URL: http://symmetrica.net/cuneiform-linux/yagf-ru.html

Source: http://symmetrica.net/cuneiform-linux/yagf-%{version}-Source.tar.gz

BuildRequires: gcc-c++ libqt4-devel
BuildRequires: cmake
Requires: cuneiform

Packager: Andrey Cherepanov <cas@altlinux.org>

%description
YAGF is a graphical front-end for cuneiform OCR tool.
With YAGF you can open already scanned image files or obtain new images via XSane (scanning results are automatically passed to YAGF).
Once you have a scanned image you can prepare it for recognition, select particular image areas for recognition, set the recognition language and so no.
Recognized text is displayed in a editor window where it can be corrected, saved to disk or copied to clipboard.  
YAGF also provides some facilities for a multi-page recognition (see the online help for more details).

YAGF is free software released under the GNU General Public License (GPL).

%prep
%setup -q
rm -f src/moc_mainform.cxx
subst "s,/usr/local,%buildroot/usr/,g" ./CMakeLists.txt

%build
CPACK_PREFX=%buildroot/usr/ make

%install
make install INSTALL_ROOT=%buildroot
%__install -pD -m755 %name %buildroot%_bindir/%name

%files 
%doc README COPYING DESCRIPTION
%_bindir/%name
%_datadir/%name/translations/*.qm

%changelog
* Mon Jul 06 2009 Andrey Cherepanov <cas@altlinux.org> 0.5.0-alt1
- First version for Sisyphus 
 
