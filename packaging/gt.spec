Name:            gt
Summary:         Command line tool for USB gadget management
Version:         0.0.1
Release:         0
License:         Apache-2.0
Group:           Base/Device Management

Source0:         %{name}-%{version}.tar.gz
Source1001:      %{name}.manifest
BuildRequires:   cmake
BuildRequires:   pkgconfig
BuildRequires:   pkgconfig(glib-2.0)
BuildRequires:   pkgconfig(libusbg)
BuildRequires:   asciidoc
BuildRequires:   xsltproc

%description
Gt is a command line tool for USB gadget management through
ConfigFS. It allows user to create, modify and remove custom
gadget and also load or save gadet from/to file.

%prep
%setup -q
cp %{SOURCE1001} .
cmake ./source/

%build
make

%install
%make_install

%files
%manifest %{name}.manifest
%license LICENSE
%defattr(-,root,root)
/usr/local/bin/gt
/etc/gt/gt.conf
/etc/bash_completion.d/gt