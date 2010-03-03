summary: YLEM Monitor for entire cluster
name: ylm_clmon
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
provides: ylm_clmon
source0: ylm_clmon
source1: ylm_clmon-start
source2: ylm_clmon-stop
requires: perl >= 5.8, ylm_clmon_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains the monitor program for entire YLEM cluster.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}

cp %{_sourcedir}/ylm_clmon %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_clmon-start %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_clmon-stop %{buildroot}%{_bindir}/

%post
%{_bindir}/ylm_clmon-stop
%{_bindir}/ylm_clmon-start

%preun
%{_bindir}/ylm_clmon-stop

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/ylm_clmon
%{_bindir}/ylm_clmon-start
%{_bindir}/ylm_clmon-stop

%changelog
* Wed Mar 03 2010 qingwu <qingwu@taobao.com>
+ ylm_clmon-1.0.0-1
- initial package

