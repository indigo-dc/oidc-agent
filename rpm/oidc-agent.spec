Name: oidc-agent
Version: 4.1.1
Release: 1%{?dist}
Summary: Commandline tool for obtaining OpenID Connect access tokens on the commandline
Group: Misc
License: MIT-License
URL: https://github.com/indigo-dc/oidc-agent
Source0: %{name}-%{version}.tar

BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0.14
BuildRequires: libsodium-static >= 1.0.14
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: libseccomp-devel >= 2.3
BuildRequires: help2man >= 1.41
BuildRequires: libsecret-devel >= 0.18.4

Requires: libsodium >= 1.0.11
Requires: libcurl >= 7.29
Requires: libmicrohttpd >= 0.9.33
Requires: libseccomp >= 2.3
Requires: libsecret >= 0.18.4
Requires: yad
Requires: jq

BuildRoot:	%{_tmppath}/%{name}

%description
Commandline tool for obtaining OpenID Connect access tokens on the commandline5???

%prep
%setup -q

%build
make 

%post
ldconfig

# if [ -f /etc/X11/Xsession.options ]; then
#   grep -Fxq "use-oidc-agent" /etc/X11/Xsession.options || echo "use-oidc-agent" >> /etc/X11/Xsession.options
# fi

%postun
ldconfig

%install
echo "Buildroot: ${RPM_BUILD_ROOT}"
make install BIN_PATH=${RPM_BUILD_ROOT}/usr BIN_AFTER_INST_PATH=/usr MAN_PATH=${RPM_BUILD_ROOT}/usr/share/man CONFIG_PATH=${RPM_BUILD_ROOT}/etc BASH_COMPLETION_PATH=${RPM_BUILD_ROOT}/usr/share/bash-completion/completions LIB_PATH=${RPM_BUILD_ROOT}/usr/lib64 DESKTOP_APPLICATION_PATH=${RPM_BUILD_ROOT}/usr/share/applications XSESSION_PATH=${RPM_BUILD_ROOT}/etc/X11

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
%config /etc/oidc-agent/pubclients.config
%config /etc/oidc-agent/oidc-agent-service.options
%config /etc/X11/Xsession.d/91oidc-agent
%doc /usr/share/man/man1/oidc-add.1.gz
%doc /usr/share/man/man1/oidc-agent.1.gz
%doc /usr/share/man/man1/oidc-gen.1.gz
%doc /usr/share/man/man1/oidc-keychain.1.gz
%doc /usr/share/man/man1/oidc-token.1.gz
%doc /usr/share/man/man1/oidc-agent-service.1.gz
%doc /usr/share/bash-completion/completions/oidc-add
%doc /usr/share/bash-completion/completions/oidc-agent
%doc /usr/share/bash-completion/completions/oidc-gen
%doc /usr/share/bash-completion/completions/oidc-keychain
%doc /usr/share/bash-completion/completions/oidc-token
%doc /usr/share/bash-completion/completions/oidc-agent-service
%doc /usr/share/applications/oidc-gen.desktop
/usr/lib64/liboidc-agent.so.4
/usr/lib64/liboidc-agent.so.%{version}
%defattr(-,root,root,-)
%{_bindir}/*
#%doc

%changelog
