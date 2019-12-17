#rpmbuild -bb --define "srcdir $PWD" ./xscreensaver-xanilogo.spec
# srcdir

%if 0 == 0%{?srcdir:1}
%{error:Path to source (srcdir) is not defined}
%endif

%global rev %(bash -c 'test 0 -eq $(git -C "%{srcdir}" diff --name-only | wc -l) && git -C "%{srcdir}" rev-list --all --count || echo "dev"')

Name:		xscreensaver-xanilogo
Version:	1.0.0
Release:	%{rev}%{?dist}
Summary:	Simple X-logo animation for XScreenSaver and generic application

Group:		Amusements/Graphics
License:	GPL v3
URL:		https://github.com/beranat/xanilogo/

BuildRequires:	gcc-c++ make libX11-devel
Requires:		xscreensaver-base

%description
The application program draws in the pseudo 3D using only simplest X11-calls (draw rectangles) a animation sequence letter X reversing frames from full invisiblty up to perpendicular projection and back.

%prep
%setup -cTn %{name}-%{version}
%{__cp} -R %{srcdir}/* ./

%build
%{__make} clean
%{make_build}

%clean
%{__make} %{?_smp_mflags} clean

%install
%make_install

%post
/usr/sbin/update-xscreensaver-hacks

%files
%defattr(-,root,root,-)
%{_libexecdir}/xscreensaver/
%{_mandir}/
%{_datarootdir}/xscreensaver/config/xanilogo.xml
%{_datarootdir}/xscreensaver/hacks.conf.d/xanilogo.conf
%doc LICENSE README.md

%changelog
* Thu Oct 25 2018 1.0.0
- Initial release
