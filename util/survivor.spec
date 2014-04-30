%define _prefix /usr/survivor
%define _group staff
%define _user survivor

Summary: SURVIVOR Scheduler Installation
Name: survivor
Version: 1.0
Release: P1
License: GPL
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Group: Applications/Internet
Source: survivor-%{version}.tar.gz
URL: http://www.columbia.edu/acis/dev/projects/survivor
Packager: Benn Oshrin <benno@columbia.edu>

%description
SURVIVOR Scheduler is the package for the Systems Monitor scheduler host.

%prep
%setup -q -n survivor-%{version}

%build
./configure --prefix=%{_prefix} --enable-user=%{_user} --enable-group=%{_group}
%{__make}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_prefix}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/etc/init.d
%{__make} do-install DESTDIR=%{buildroot}
(cd %{_builddir}/survivor-%{version}/modules/check/pingx && %{__make} install-manual DESTDIR=%{buildroot})
ln -s %{_prefix}/bin/sc %{buildroot}/usr/bin/sc
cp %{_builddir}/survivor-%{version}/init.d/survivor %{buildroot}/etc/init.d/survivor
chmod 555 %{buildroot}/etc/init.d/survivor

%clean
rm -rf %{buildroot}

%files
%attr(4555,root,%{_group}) %{_prefix}/mod/check/pingx
%defattr(-,%{_user},%{_group})
%{_bindir}/*
%{_sbindir}/*
%{_prefix}/html/*
%{_prefix}/mod/*
/usr/bin/sc
/etc/init.d/survivor

%post

%postun

%changelog
* Mon Apr 02 2007 <benno@columbia.edu> 1.0-P1
- Initial revision
