# enable debug features such as control environment variables
# WARNING! do not use for production builds as it will break security
%define debug_build 1

Name: tlm
Summary: Login manager for Tizen
Version: 0.0.6
Release: 0
Group: System/Service
License: LGPL-2.1+
Source: %{name}-%{version}.tar.gz
URL: https://github.com/01org/tlm
Source1001: %{name}.manifest
Requires(post): /sbin/ldconfig, systemd
Requires(postun): /sbin/ldconfig, systemd
Requires: gumd
Requires: libsystemd
BuildRequires: pkgconfig(glib-2.0) >= 2.30
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(gmodule-2.0)
BuildRequires: pkgconfig(libgum)
BuildRequires: pkgconfig(elementary)
BuildRequires: pam-devel
%if %{debug_build} == 1
BuildRequires: gtk-doc
%endif


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
cp %{SOURCE1001} .


%build
%if %{debug_build} == 1
./autogen.sh
%configure --enable-gum --enable-gtk-doc --enable-examples --enable-debug
%else
%configure --enable-gum --enable-examples
%endif
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%make_install
install -m 755 -d %{buildroot}%{_unitdir}
install -m 644 data/tlm.service %{buildroot}%{_unitdir}
install -m 755 -d %{buildroot}%{_sysconfdir}/pam.d
install -m 644 data/tlm-login %{buildroot}%{_sysconfdir}/pam.d/
install -m 644 data/tlm-default-login %{buildroot}%{_sysconfdir}/pam.d/


%post
/sbin/ldconfig
/usr/bin/systemctl enable tlm
/usr/bin/systemctl daemon-reload


%postun
/sbin/ldconfig
/usr/bin/systemctl disable tlm
/usr/bin/systemctl daemon-reload


%files
%defattr(-,root,root,-)
%manifest %{name}.manifest
%doc AUTHORS COPYING INSTALL NEWS README
%{_bindir}/%{name}
%{_bindir}/%{name}-sessiond
%{_bindir}/%{name}-client
%{_libdir}/lib%{name}*.so.*
%{_libdir}/%{name}/plugins/*.so*
%{_unitdir}/tlm.service
%config(noreplace) %{_sysconfdir}/tlm.conf
%config %{_sysconfdir}/pam.d/tlm-login
%config %{_sysconfdir}/pam.d/tlm-default-login


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/lib%{name}*.so
%{_libdir}/pkgconfig/%{name}.pc
%{_bindir}/tlm-ui


%files doc
%defattr(-,root,root,-)
%{_datadir}/gtk-doc/html/tlm/*
