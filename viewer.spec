%define debug_package %{nil}
Summary: stereo pair viewer/aligner
Name: viewer
Version: 0.7.4
Release: 1
Copyright: GPL
Group: Applications/Multimedia
Source: ftp://ftp.cs.umn.edu/dept/users/wburdick/geowall/viewer-%{version}.tar.gz
BuildRoot: %{_tmppath}/build-root-%{name}
URL: http://www-users.cs.umn.edu/~wburdick/geowall/viewer.html
Packager: Russ Burdick <wburdick@cs.umn.edu>

%description
viewer is an OpenGL based stereo pair viewer/aligner. viewer currently
only supports PPM image files (either raw or ascii), but plans are to
add other image formats in the future. viewer supports OpenGL quad
buffer stereo mode (also known as "clone mode" or "hardware stereo
mode") found on NVidia's Quadro cards. see viewer.1 manpage for full 
details.

viewer requires OpenGL, GLU, GLUT, and associated devel packages, but
these arent specifically listed as package requirements since they often
dont come from RPMs (particularly in the case of the NVidia cards).

%prep
%setup -q

%build
./configure 
make

%install
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
install -s -m 755 viewer $RPM_BUILD_ROOT%{_bindir}/viewer
install -m 644 viewer.1 $RPM_BUILD_ROOT%{_mandir}/man1/viewer.1

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README

%{_bindir}/viewer
%{_mandir}/man1/viewer.1.gz

%changelog
* Fri Feb 27 2004 Russ Burdick <wburdick@cs.umn.edu>
- inital spec
