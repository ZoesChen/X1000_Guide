Name:           libjpeg-turbo
License:        BSD3c(or similar)
Group:          Productivity/Graphics/Convertors
AutoReqProv:    on
Version: 	1.2.0
Release:        11
Summary:        A MMX/SSE2 accelerated library for manipulating JPEG image files
Url:            http://sourceforge.net/projects/libjpeg-turbo
Source0:        %{name}-%{version}.tar.gz

%description
The libjpeg-turbo package contains a library of functions for manipulating
JPEG images.

%package devel

License:        BSD3c(or similar)
Summary:        Developement files for libjpeg-turbo contains a wrapper library (TurboJPEG/OSS) that emulates the TurboJPEG API using libjpeg-turbo
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}-%{release}
Provides: 	libjpeg-devel
%ifarch i586
BuildRequires:  nasm
%endif
%description devel
The libjpeg-devel package includes the header files and documentation
necessary for developing programs which will manipulate JPEG files using
the libjpeg library.

If you are going to develop programs which will manipulate JPEG images,
you should install libjpeg-devel.  You'll also need to have the libjpeg
package installed.

%prep
%setup -q 

%build
autoreconf -fiv
%configure --enable-shared --disable-static --with-jpeg8
make %{?_smp_mflags}

#%check
#make test libdir=%{_libdir}

%install
%makeinstall
mkdir -p %{buildroot}/usr/share/license
cp COPYING %{buildroot}/usr/share/license/%{name}
# Fix perms
chmod -x README-turbo.txt release/copyright

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun  -p /sbin/ldconfig

%files
/usr/share/license/%{name}
%manifest libjpeg-turbo.manifest
%defattr(-,root,root)
%{_libdir}/libturbojpeg.so
%{_libdir}/libjpeg.so.*
%{_bindir}/cjpeg
%{_bindir}/djpeg
%exclude %{_datadir}/man/man1/*
%exclude %{_datadir}/doc/
#%exclude %{_bindir}/cjpeg
#%exclude %{_bindir}/djpeg
%exclude %{_bindir}/jpegtran
%exclude %{_bindir}/rdjpgcom
%exclude %{_bindir}/tjbench
%exclude %{_bindir}/wrjpgcom


%files devel
%defattr(-,root,root)
%{_libdir}/libjpeg.so
%{_includedir}/*.h
%{_libdir}/pkgconfig/turbojpeg.pc
%exclude %{_libdir}/libjpeg.la
%exclude %{_libdir}/libturbojpeg.la
%exclude %{_datadir}/doc/README
%exclude %{_datadir}/doc/README-turbo.txt
%exclude %{_datadir}/doc/example.c
%exclude %{_datadir}/doc/libjpeg.txt
%exclude %{_datadir}/doc/structure.txt
%exclude %{_datadir}/doc/usage.txt
%exclude %{_datadir}/doc/wizard.txt
