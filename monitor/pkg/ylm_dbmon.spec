summary: YLEM Monitor for metadata database
name: ylm_dbmon
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
provides: ylm_dbmon
source0: ylm_dbmon
source1: ylm_dbmon-start
source2: ylm_dbmon-stop
requires: perl >= 5.8

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains the monitor program for YLEM metadata database(only
needed on the master side).

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}

cp %{_sourcedir}/ylm_dbmon %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_dbmon-start %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_dbmon-stop %{buildroot}%{_bindir}/

%post
%{_bindir}/ylm_dbmon-stop
%{_bindir}/ylm_dbmon-start

%preun
%{_bindir}/ylm_dbmon-stop

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/ylm_dbmon
%{_bindir}/ylm_dbmon-start
%{_bindir}/ylm_dbmon-stop

%changelog
* Wed Mar 03 2010 qingwu <qingwu@taobao.com>
+ ylm_dbmon-1.0.0-1
- initial version

