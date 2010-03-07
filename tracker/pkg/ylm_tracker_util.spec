summary: YLEM Tracker utilities
name: ylm_tracker_util
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
source0: mogdbsetup
source1: mogadm
source2: mogtool
source3: lib
provides: ylm_tracker_util
requires: ylm_tracker_util_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains several useful tools for YLEM system.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/ylm_tracker_util

cp %{_sourcedir}/mogdbsetup %{buildroot}%{_bindir}/
cp %{_sourcedir}/mogadm %{buildroot}%{_bindir}/
cp %{_sourcedir}/mogtool %{buildroot}%{_bindir}/
cp -r %{_sourcedir}/lib %{buildroot}%{_datadir}/ylm_tracker_util/

%post

%preun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/mogdbsetup
%{_bindir}/mogadm
%{_bindir}/mogtool
%{_datadir}/ylm_tracker_util

%changelog
* Sun Mar 07 2010 qingwu <qingwu@taobao.com>
+ ylm_tracker_util-1.0.0-1
- initial package

