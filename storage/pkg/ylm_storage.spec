summary: YLEM storage server
name: ylm_storage
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
provides: ylm_storage
requires: lighttpd_storage, lighttpd_storage_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains the storage server for YLEM storage nodes.

%prep

%build

%install
rm -rf %{buildroot}

%post
%{_bindir}/lighttpd stop
%{_bindir}/lighttpd start

%preun
%{_bindir}/lighttpd stop

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)

%changelog
* Fri Mar 05 2010 qingwu <qingwu@taobao.com>
+ ylm_storage-1.0.0-1
- initial package

