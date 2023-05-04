%define _unpackaged_files_terminate_build 0
%define docker_client third_party/DockerClient
%define prefix %{name}
%define _build_id_links none

%define version %(echo $VERSION)

Name:           watchman_cpp
Version:        %{version}
Release:        1%{?dist}
Summary:        watchman_cpp

License:        license
URL: https://gitlab.com/senjun/watchman_cpp
Source0: https://gitlab.com/senjun/%{name}/-/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires: g++
BuildRequires: cmake
BuildRequires: make

%description
Watchman cpp project

%package devel
Summary:        Develompent files for %{name}
Requires:       libcurl-devel
Requires:       boost-devel

%description devel
Devel

%prep

%autosetup

%build
%cmake
%cmake_build -DCMAKE_BUILD_TYPE=Release

%install
%cmake_install
cp -Rp %{_builddir}/%{name}-%{version}/accomodation/etc %{buildroot}/

%{__install} -p -D -m 0644 accomodation/etc/%{name}.systemd %{buildroot}/%{_unitdir}/%{name}-%{version}.service

%files
%{_bindir}/%{name}
%{_sysconfdir}/%{name}_config.json
