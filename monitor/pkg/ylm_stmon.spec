summary: YLEM Monitor for storage nodes
name: ylm_stmon
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
provides: ylm_stmon
source0: ylm_stmon
source1: ylm_stmon-start
source2: ylm_stmon-stop
source3: lib 
requires: perl >= 5.8

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains the monitor program for YLEM storage server.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/ylm_stmon

cp %{_sourcedir}/ylm_stmon %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_stmon-start %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_stmon-stop %{buildroot}%{_bindir}/
cp -r %{_sourcedir}/lib %{buildroot}%{_datadir}/ylm_stmon/

%post
%{_bindir}/ylm_stmon-stop
sudo -u nobody %{_bindir}/ylm_stmon-start

%preun
%{_bindir}/ylm_stmon-stop

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%attr(0755,root,root) %{_bindir}/ylm_stmon
%attr(0755,root,root) %{_bindir}/ylm_stmon-start
%attr(0755,root,root) %{_bindir}/ylm_stmon-stop
%{_datadir}/ylm_stmon

%changelog
* Wed Mar 03 2010 qingwu <qingwu@taobao.com>
+ ylm_stmon-1.0.0-1
- initial version

