Name: oidc-agent
Version: 1.1.0
Release: 1
Summary: Commandline tool for obtaining OpenID Connect Access tokens on the commandline
Group: Misc
License: MIT-License
URL: https://github.com/indigo-dc/oidc-agent
Source0: oidc-agent.tar

BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0

Requires: libsodium18 >= 1.0.11
Requires: libcurl13 >= 7.52

BuildRoot:	%{_tmppath}/%{name}

%description
Commandline tool for obtaining OpenID Connect Access tokens on the commandline5???

%prep
%setup -q

%build
make 

%install
echo "Buildroot: ${RPM_BUILD_ROOT}"
echo "ENV: "
env | grep -i rpm
echo "PWD"
pwd
make install INSTALL_PATH=${RPM_BUILD_ROOT}/usr

%files
%defattr(-,root,root,-)
%{_bindir}/*
#%doc
#%{_mandir}/*

%changelog
