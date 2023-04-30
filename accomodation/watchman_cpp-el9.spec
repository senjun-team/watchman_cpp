%define _unpackaged_files_terminate_build 0
%define docker_client third_party/DockerClient
%define prefix %{name}
%define _build_id_links none

Name:           watchman_cpp
Version:        0.0.8
Release:        1%{?dist}
Summary:        Hello

License:        license
URL: https://gitlab.com/senjun/watchman_cpp
Source0: https://gitlab.com/senjun/%{name}/-/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires: g++
BuildRequires: cmake
BuildRequires: make

%description
Watchman cpp projec

%package devel
Summary:        Develompent files for %{name}
Requires:       libcurl-devel
Requires:       boost-devel

%description devel
Devel

%prep

%autosetup
git clone git@gitlab.com:senjun/watchman_cpp.git
cd %{name}
git checkout %{version}
git submodule update --init --recursive
cd ..
mv -f %{name}/%{docker_client} third_party
rm -rf %{name}

%build
%cmake
%cmake_build -DCMAKE_BUILD_TYPE=Release

%install
%cmake_install
cp -Rp %{_builddir}/%{name}-%{version}/accomodation/etc %{buildroot}/

%{__install} -p -D -m 0644 accomodation/etc/%{name}.systemd %{buildroot}/%{_unitdir}/%{name}.service

%{__install} -p -D -m 0644 accomodation/etc/%{name}.logrotate %{buildroot}/%{_sysconfdir}/logrotate.d/%{name}
%{__install} -p -D -m 0644 accomodation/etc/%{name}.rsyslog %{buildroot}/%{_sysconfdir}/rsyslog.d/%{name}.conf

%files
%{_bindir}/%{name}
%{_sysconfdir}

%changelog
* Tue Apr 04 2023 root
