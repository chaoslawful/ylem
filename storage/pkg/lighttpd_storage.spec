summary: YLEM storage backend (lighttpd)
name: lighttpd_storage
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
source0: spawn-fcgi
source1: lighttpd
source2: lighttpd-angel
source3: rc.lighttpd.redhat
source4: libs
provides: lighttpd_storage
requires: lighttpd_storage_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains YLEM storage backend - lighttpd.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_sbindir}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_var}/log/ylm_storage
mkdir -p %{buildroot}%{_var}/lib/ylem

cp %{_sourcedir}/libs/*.so %{buildroot}%{_libdir}/
cp %{_sourcedir}/spawn-fcgi %{buildroot}%{_bindir}/
cp %{_sourcedir}/lighttpd %{buildroot}%{_sbindir}/
cp %{_sourcedir}/lighttpd-angel %{buildroot}%{_sbindir}/
cp %{_sourcedir}/rc.lighttpd.redhat %{buildroot}%{_bindir}/lighttpd

%post

%preun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_libdir}/
%{_bindir}/spawn-fcgi
%{_bindir}/lighttpd
%{_sbindir}/lighttpd
%{_sbindir}/lighttpd-angel
%dir %attr(-,nobody,nobody) %{_var}/lib/ylem
%dir %attr(-,nobody,nobody) %{_var}/log/ylm_storage

%changelog
* Sat Mar 06 2010 qingwu <qingwu@taobao.com>
+ lighttpd_storage-1.0.0-1
- initial package

