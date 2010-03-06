summary: YLEM storage backend config file (lighttpd)
name: lighttpd_storage_conf
version: 1.0.0
release: 1
url: http://github.com/chaoslawful/ylem
vendor: chaoslawful <http://github.com/chaoslawful>
license: BSD
group: Application/System
provides: lighttpd_storage_conf

#buildrequires:
buildroot: %{_tmppath}/%{name}-%{version}-%(%{__id} -u -n)

%description
This package contains config files for YLEM storage backend - lighttpd.

%prep

%build

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_sysconfdir}/cron.d
mkdir -p %{buildroot}%{_sysconfdir}/ylm_storage
mkdir -p %{buildroot}%{_var}/lib/ylem

# storage configuration file
cp %{_sourcedir}/lighttpd.conf %{buildroot}%{_sysconfdir}/ylm_storage/

# default html file
cp %{_sourcedir}/index.html %{buildroot}%{_var}/lib/ylem/

# log rotation
cp %{_sourcedir}/log_rotate.conf %{buildroot}%{_sysconfdir}/ylm_storage/
echo "0 0 * * * root /usr/sbin/logrotate -f %{_sysconfdir}/ylm_storage/log_rotate.conf" > %{buildroot}%{_sysconfdir}/cron.d/ylm_storage

%post

%preun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_sysconfdir}/ylm_storage
%{_sysconfdir}/cron.d/ylm_storage
%{_var}/lib/ylem/index.html

%changelog
* Sun Mar 07 2010 qingwu <qingwu@taobao.com>
+ lighttpd_storage_conf-1.0.0-1
- initial package


