summary: YLEM Tracker Server
name: ylm_tracker
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
source0: ylm_tracker
source1: ylm_tracker-start
source2: ylm_tracker-stop
source3: lib 
provides: ylm_tracker
requires: ylm_tracker_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This is the main package of YLEM Tracker server. It contains the main
program and depends on all necessary components.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/ylm_tracker

cp %{_sourcedir}/ylm_tracker %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_tracker-start %{buildroot}%{_bindir}/
cp %{_sourcedir}/ylm_tracker-stop %{buildroot}%{_bindir}/
cp -r %{_sourcedir}/lib %{buildroot}%{_datadir}/ylm_tracker/

%post
%{_bindir}/ylm_tracker-stop
%{_bindir}/ylm_tracker-start

%preun
%{_bindir}/ylm_tracker-stop

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/ylm_tracker
%{_bindir}/ylm_tracker-start
%{_bindir}/ylm_tracker-stop
%{_datadir}/ylm_tracker

%changelog
* Sun Mar 07 2010 qingwu <qingwu@taobao.com>
+ ylm_tracker-1.0.0-1
- initial package


