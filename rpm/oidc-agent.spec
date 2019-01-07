Name: oidc-agent
%define version %(cat VERSION)
Version: %{version}
Release: 1
Summary: Commandline tool for obtaining OpenID Connect Access tokens on the commandline
Group: Misc
License: MIT-License
URL: https://github.com/indigo-dc/oidc-agent
Source0: oidc-agent.tar

BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0.11
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: libseccomp-devel >= 2.3
BuildRequires: help2man >= 1.41

Requires: libsodium >= 1.0.11
Requires: libcurl >= 7.29
Requires: libmicrohttpd >= 0.9.33
Requires: libseccomp >= 2.3

BuildRoot:	%{_tmppath}/%{name}

%description
Commandline tool for obtaining OpenID Connect Access tokens on the commandline5???

%prep
%setup -q

%build
make 

%install
echo "Buildroot: ${RPM_BUILD_ROOT}"
make install BIN_PATH=${RPM_BUILD_ROOT}/usr MAN_PATH=${RPM_BUILD_ROOT}/usr/share/man CONFIG_PATH=${RPM_BUILD_ROOT}/etc BASH_COMPLETION_PATH=${RPM_BUILD_ROOT}/usr/share/bash-completion/completions LIB_PATH=${RPM_BUILD_ROOT}/usr/lib/`uname -p`-linux-gnu/ LIB_PATH=${RPM_BUILD_ROOT}/usr/lib/`uname -p`-linux-gnu/

%files
%config /etc/oidc-agent/issuer.config
%config /etc/oidc-agent/privileges/agentIpc.priv
%config /etc/oidc-agent/privileges/crypt.priv
%config /etc/oidc-agent/privileges/daemon.priv
%config /etc/oidc-agent/privileges/general.priv
%config /etc/oidc-agent/privileges/http.priv
%config /etc/oidc-agent/privileges/httpserver.priv
%config /etc/oidc-agent/privileges/kill.priv
%config /etc/oidc-agent/privileges/logging.priv
%config /etc/oidc-agent/privileges/memory.priv
%config /etc/oidc-agent/privileges/print.priv
%config /etc/oidc-agent/privileges/prompt.priv
%config /etc/oidc-agent/privileges/read.priv
%config /etc/oidc-agent/privileges/signal.priv
%config /etc/oidc-agent/privileges/sleep.priv
%config /etc/oidc-agent/privileges/socket.priv
%config /etc/oidc-agent/privileges/time.priv
%config /etc/oidc-agent/privileges/write.priv
%doc /usr/share/man/man1/oidc-add.1.gz
%doc /usr/share/man/man1/oidc-agent.1.gz
%doc /usr/share/man/man1/oidc-gen.1.gz
%doc /usr/share/man/man1/oidc-token.1.gz
%doc /usr/share/bash-completion/completions/oidc-add
%doc /usr/share/bash-completion/completions/oidc-agent
%doc /usr/share/bash-completion/completions/oidc-gen
%doc /usr/share/bash-completion/completions/oidc-token
/usr/lib/x86_64-linux-gnu/liboidc-agent.so.2
/usr/lib/x86_64-linux-gnu/liboidc-agent.so.2.1.3
%defattr(-,root,root,-)
%{_bindir}/*
#%doc

%changelog

