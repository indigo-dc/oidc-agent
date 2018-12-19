Name: liboidc-agent2
Version: 2.1.3
Release: 1
Summary: oidc-agent library
Group: Misc
License: MIT-License
URL: https://github.com/indigo-dc/oidc-agent
Source0: oidc-agent.tar

BuildRoot:	%{_tmppath}/%{name}

%description
oidc-agent library
 oidc-agent is a commandline tool for obtaining OpenID Connect Access tokens on
 the commandline.
 .
 This package provides a library for easy communication with oidc-agent. 
 Applications can use this library to request access token from oidc-agent.

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
make install BIN_PATH=${RPM_BUILD_ROOT}/usr MAN_PATH=${RPM_BUILD_ROOT}/usr/share/man CONFIG_PATH=${RPM_BUILD_ROOT}/etc BASH_COMPLETION_PATH=${RPM_BUILD_ROOT}/usr/share/bash-completion/completions

%files
/usr/lib/x86_64-linux-gnu/liboidc-agent.so.2.1.3
/usr/lib/x86_64-linux-gnu/liboidc-agent.so.2
%defattr(-,root,root,-)
%{_bindir}/*
#%doc
#%{_mandir}/*

%changelog
