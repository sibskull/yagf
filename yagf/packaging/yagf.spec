Summary: Yet Another Graphic Front-end for Cuneiform
Name: yagf
Version: 0.8.1
Release: 0
License: GPL
Group: Productivity/Graphics/Other
Summary:	Graphical frontend for Cuneiform OCR tool
URL: http://symmetrica.net/cuneiform-linux/yagf-en.html

Source: http://symmetrica.net/cuneiform-linux/yagf-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: libqt4-devel aspell-devel cmake
Requires:       libqt4 > 4.2 aspell

%description
YAGF is a graphical front-end for cuneiform OCR tool.
With YAGF you can open already scanned image files or obtain new images via XSane (scanning results are automatically passed to YAGF).
Once you have a scanned image you can prepare it for recognition, select particular image areas for recognition, set the recognition language and so no.
Recognized text is displayed in a editor window where it can be corrected, saved to disk or copied to clipboard.  
YAGF also provides some facilities for a multi-page recognition (see the online help for more details).
Authors:
--------
    Andrei Borovsky <anb@symmetrica.net>

%prep
%setup -q


%build
cmake ./
%{__make} CPACK_PREFX=/usr/ 

%install

%{__rm} -rf %{buildroot}
mkdir %{buildroot}

%makeinstall
#%find_lang %{name}

strip %{buildroot}/usr/bin/yagf
strip %{buildroot}%{_libdir}/yagf/libxspreload.so

%clean
%{__rm} -rf %{buildroot}

%files 
%defattr (-,root,root)
    %{_bindir}/yagf
    %{_libdir}/yagf/libxspreload.so
    %{_datadir}/yagf/translations/*.qm
    %{_datadir}/pixmaps/yagf.png
    %{_datadir}/icons/hicolor/96x96/apps/yagf.png
    %{_datadir}/applications/YAGF.desktop
    
%defattr(-, root, root, 0755)
%doc README COPYING DESCRIPTION AUTHORS ChangeLog

%changelog

* Sun Aug 16 2009 Andrei Borovsky <anb@symmetrica.net> - 0.8.1
- batch recognition added
* Wed Aug 5 2009  Andrei Borovsky <anb@symmetrica.net> - 0.8.0
- text selection blocks are now resizable
- images management bar is added
* Sat Jul 25 2009 Andrei Borovsky <anb@symmetrica.net> - 0.7.1
- scaling and rotation is kept between images in the series
- images and text may be scaled by Ctrl + mouse wheel or by Ctrl + [+]/[-] keys.
* Sun Jul 19 2009 Andrei Borovsky <anb@symmetrica.net> - 0.7.0
- spell-checking is added
- saving to html with images is added
* Fri Jul 17 2009 Andrei Borovsky <anb@symmetrica.net> - 0.6.2
- merged the patches with the appropriate files
- removed unnessesary ldconfig call
* Wed Jul 15 2009 Kyrill Detinov <lazy.kent.suse@gmail.com> - 0.6.1
- update to 0.6.1
- fixed build in x86-64
- corrected build requires
* Sat Jun 20 2009 Kyrill Detinov <lazy.kent.suse@gmail.com> - 0.5.0
- change compiling outside of the source tree
* Mon Jun 15 2009 Kyrill Detinov <lazy.kent.suse@gmail.com> - 0.5.0
- fix requires Qt version
* Mon Jun 08 2009 Kyrill Detinov <lazy.kent.suse@gmail.com> - 0.5.0
- correct build requires:  libqt4-devel <= 4.4.3, cmake >= 2.6
* Fri Jun 05 2009 Kyrill Detinov <lazy.kent.suse@gmail.com> - 0.5.0
- initial package created
 

