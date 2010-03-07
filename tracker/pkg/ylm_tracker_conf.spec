summary: YLEM Tracker Config Files
name: ylm_tracker_conf
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
source0: tracker.conf
provides: ylm_tracker_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains the necessary config files for YLEM tracker.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_sysconfdir}/ylm_tracker

cp %{_sourcedir}/tracker.conf %{buildroot}%{_sysconfdir}/ylm_tracker/

%post

%preun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_sysconfdir}/ylm_tracker/
%config %{_sysconfdir}/ylm_tracker/tracker.conf

%changelog
* Sun Mar 07 2010 qingwu <qingwu@taobao.com>
+ ylm_tracker_conf-1.0.0-1
- initial package


