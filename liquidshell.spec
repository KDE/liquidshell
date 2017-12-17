# spec file for package liquidshell
#
# Copyright (c) 2017 Martin Koller
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bug fixes or comments via mailto:kollix@aon.at
 
# norootforbuild
 
Summary: A replacement for plasmashell
Name:    liquidshell
Version: 1.0
Release: 1
License: GPL-3.0
#Group:   Productivity/Graphics/Viewers
Source:  %{name}-%{version}.tar.bz2
URL: https://cgit.kde.org/liquidshell.git
BuildRequires: kconfig-devel kio-devel kwidgetsaddons-devel kdoctools-devel	
BuildRequires: ki18n-devel libQt5Widgets-devel update-desktop-files
BuildRoot: %{_tmppath}/%{name}-%{version}-build
 
%description
Alternative desktop replacement for Plasma, using QtWidgets
instead of QtQuick to ensure hardware acceleration is not required

Authors:
--------
    Martin Koller <kollix@aon.at>

#================= PREP ==================
%prep
%setup -q
 
#================= BUILD ==================
%build
%if 0%{?suse_version}
%cmake
%make_jobs
%else
%cmake
make
%endif
 
#================= INSTALL ==================
%install
%if 0%{?fedora}
%else
pushd build
%endif
make DESTDIR=$RPM_BUILD_ROOT install
%if 0%{?fedora}
%else
popd
%endif
 
%if 0%{?suse_version}
%suse_update_desktop_file %name
%else
%endif
 
 
#================= POST / POSTUN Mandriva ==================
%if 0%{?mandriva_version}
%post
%update_menus
 
%postun
%clean_menus
%endif
 
#================= CLEAN ==================
%clean
rm -rf $RPM_BUILD_ROOT
 
#================= FILES ==================
%files
%defattr(-,root,root)
%doc README
%{_bindir}/%{name}
%{_datadir}/applications/org.kde.liquidshell.desktop
 
#================= CHANGELOG ==================
%changelog
* Fri Nov 3 2017 Martin Koller <kollix@aon.at> - 1.0
- First release
