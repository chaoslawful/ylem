summary: Volume list for cluster monitoring
name: ylm_clmon_conf
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
provides: ylm_clmon_conf
source: dbmon_boxes
#requires:

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains the volume list of entire YLEM cluster.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_sysconfdir}/ylm_clmon

cp %{_sourcedir}/dbmon_boxes %{buildroot}%{_sysconfdir}/ylm_clmon/

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%dir %{_sysconfdir}/ylm_clmon
%config(noreplace) %{_sysconfdir}/ylm_clmon/dbmon_boxes

%changelog
* Wed Mar 03 2010 qingwu <qingwu@taobao.com>
+ ylm_clmon_conf-1.0.0-1
- initial version

