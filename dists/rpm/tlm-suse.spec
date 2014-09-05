# enable debug features such as control environment variables
# WARNING! do not use for production builds as it will break security
%define debug_build 0

Name: tlm
Summary: Login manager for Tizen
Version: 0.0.3
Release: 2
Group: System/Daemons
License: LGPL-2.1+
Source: %{name}-%{version}.tar.gz
URL: https://github.com/01org/tlm
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires: pkgconfig(gtk-doc)
BuildRequires: pkgconfig(glib-2.0) >= 2.30
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(gmodule-2.0)
BuildRequires: pkgconfig(libgum)
BuildRequires: pam-devel

%description
%{summary}.


%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}


%description devel
%{summary}.


%package doc
Summary:    Documentation files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}


%description doc
%{summary}.


%prep
%setup -q -n %{name}-%{version}


%build
%if %{debug_build} == 1
%configure --enable-gum --enable-gtk-doc --enable-debug
%else
%configure --enable-gum --enable-gtk-doc
%endif


make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%make_install


%post
/sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING INSTALL NEWS README
%{_bindir}/%{name}
%{_bindir}/%{name}-sessiond
%{_bindir}/%{name}-client
%{_libdir}/lib%{name}*.so.*
%{_libdir}/%{name}/plugins/*.so*
%exclude %{_libdir}/tlm/plugins/*.la
%config(noreplace) %{_sysconfdir}/tlm.conf


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/lib%{name}*.so
%{_libdir}/lib%{name}*.la
%{_libdir}/tlm/plugins/*.la
%{_libdir}/pkgconfig/%{name}.pc


%files doc
%defattr(-,root,root,-)
%{_datadir}/gtk-doc/html/tlm/*


%changelog
* Wed Sep 03 2014 Jussi Laako <jussi.laako@linux.intel.com>
- Added separate PAM configuration file for default user

* Tue Aug 26 2014 Imran Zaman <imran.zaman@intel.com>
- session: set umask when logging in
- session: with pause session, set environment before PAM open session
- updated gitignore file
- Fixed white spaces

* Mon Jul 21 2014 Imran Zaman <imran.zaman@intel.com>
- Update to 0.0.3; create a new process (tlm-sessiond) for each session

* Thu Mar 13 2014 Jussi Laako <jussi.laako@linux.intel.com>
- Update to 0.0.2

* Thu Feb 13 2014 Imran Zaman <imran.zaman@intel.com>
- Initial RPM packaging

