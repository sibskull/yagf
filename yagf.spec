Summary: Yet Another Graphic Front-end for Cuneiform
Name: yagf
Version: 0.5.0
Release: 0
License: GPL
Group: Applications/Text
URL: http://symmetrica.net/cuneiform-linux/yagf-ru.html

Source: http://symmetrica.net/cuneiform-linux/yagf-%{version}-Source.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: qt4-x11-tools
BuildRequires: cmake

Packager: Andrei Borovsky

%description
YAGF is a graphical front-end for cuneiform OCR tool.
With YAGF you can open already scanned image files or obtain new images via XSane (scanning results are automatically passed to YAGF).
Once you have a scanned image you can prepare it for recognition, select particular image areas for recognition, set the recognition language and so no.
Recognized text is displayed in a editor window where it can be corrected, saved to disk or copied to clipboard.  
YAGF also provides some facilities for a multi-page recognition (see the online help for more details).

YAGF is free software released under the GNU General Public License (GPL).

%prep
%setup -q


%build
%{__make} CPACK_PREFX=/usr/

%install
%{__rm} -rf %{buildroot}
%makeinstall
#%find_lang %{name}

%clean
%{__rm} -rf %{buildroot}

%files 
    %{_prefix}/local/bin/yagf
    %{_prefix}/local/share/yagf/translations/*.qm
    %{_prefix}/local/share/yagf/COPYING
    %{_prefix}/local/share/yagf/DESCRIPTION
    %{_prefix}/local/share/yagf/README

%defattr(-, root, root, 0755)
%doc README COPYING

#%{_datadir}/icons/hicolor/*
#%{_datadir}/applications/subtitleeditor.desktop

%changelog
* Fri Aug 3 2007 - chantra AatT rpm-based DdOoTt org 0.20.alpha4-1.rb
- Initial release.
 
