#rpmbuild -bb --define "srcdir $PWD" ./xanilogo.spec
# srcdir

%if 0 == 0%{?srcdir:1}
%{error:Path to source (srcdir) is not defined}
%endif

%global rev %(bash -c 'test 0 -eq $(git -C "%{srcdir}" diff --name-only | wc -l) && git -C "%{srcdir}" rev-list --all --count || echo "dev"')

Name: xanilogo
Version: 1.0.1
Release: %{rev}%{?dist}
Summary: Simple X-logo animation for some screen savers and generic application
Group: Amusements/Graphics
License: GPL v3
URL: https://github.com/beranat/xanilogo/
BuildRequires:	gcc-c++ make libX11-devel

%description
This is a meta package for all screen savers frameworks.

The application program draws in the pseudo 3D using only simplest X11-calls (draw rectangles) a animation sequence letter X reversing frames from full invisiblty up to perpendicular projection and back.

%package -n xscreensaver-xanilogo
Summary: Simple X-logo animation for XScreenSaver
Group: Amusements/Graphics
Requires: xanilogo
Requires: xscreensaver-base
%description -n xscreensaver-xanilogo
Xscreensaver application program draws in the pseudo 3D using only simplest X11-calls (draw rectangles) a animation sequence letter X reversing frames from full invisiblty up to perpendicular projection and back.

%package -n xfce4-screensaver-xanilogo
Summary: Simple X-logo animation for XFCE-screensaver
Group: Amusements/Graphics
Requires: xanilogo
Requires: xfce4-screensaver
%description -n xfce4-screensaver-xanilogo
Xfce screensaver application program draws in the pseudo 3D using only simplest X11-calls (draw rectangles) a animation sequence letter X reversing frames from full invisiblty up to perpendicular projection and back.

%prep
%setup -cTn %{name}-%{version}
%{__cp} -R %{srcdir}/* ./

%build
%{__make} clean
%{make_build} CONFIG=release

%clean
%{__make} %{?_smp_mflags} clean

%install
%make_install

%post -n xscreensaver-xanilogo
/usr/sbin/update-xscreensaver-hacks

%files
%defattr(-,root,root,-)
%{_mandir}/*
%{_bindir}/*
%{_datarootdir}/xscreensaver/config/xanilogo.xml
%doc LICENSE README.md

%files -n xscreensaver-xanilogo
%{_libexecdir}/xscreensaver/xanilogo
%{_datarootdir}/xscreensaver/hacks.conf.d/xanilogo.conf

%files -n xfce4-screensaver-xanilogo
%{_libexecdir}/xfce4-screensaver/xanilogo
%{_datarootdir}/applications/screensavers/xanilogo.desktop

%changelog
* Wed Jan 12 2022 1.0.1
- Xfce screen saver support

* Thu Oct 25 2018 1.0.0
- Initial release
