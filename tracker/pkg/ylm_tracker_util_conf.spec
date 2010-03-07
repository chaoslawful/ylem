summary: YLEM Tracker Utility Config Files
name: ylm_tracker_util_conf
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
source0: admin.conf
source0: tools.conf
provides: ylm_tracker_util_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains config files for YLEM tracker utilities.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_sysconfdir}/ylm_tracker_util

cp %{_sourcedir}/admin.conf %{buildroot}%{_sysconfdir}/ylm_tracker_util/
cp %{_sourcedir}/tools.conf %{buildroot}%{_sysconfdir}/ylm_tracker_util/

%post

%preun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_sysconfdir}/ylm_tracker_util/

%changelog
* Sun Mar 07 2010 qingwu <qingwu@taobao.com>
+ ylm_tracker_util_conf-1.0.0-1
- initial package


