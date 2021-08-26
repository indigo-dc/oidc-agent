Name: oidc-agent
%define ver %(head debian/changelog -n 1|cut -d \\\( -f 2|cut -d \\\) -f 1|cut -d \- -f 1)
%define rel %(head debian/changelog -n 1|cut -d \\\( -f 2|cut -d \\\) -f 1|cut -d \- -f 2)
Version: %{ver}
Release: %{rel}
Summary: Command-line tool for obtaining OpenID Connect access tokens on the commandline

Group: Misc
License: MIT
URL: https://github.com/indigo-dc/oidc-agent
#Source0: %{name}-%{version}.tar
Source0: https://github.com/indigo-dc/oidc-agent/archive/refs/tags/v%{version}.tar.gz 

BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0.14
BuildRequires: libsodium-static >= 1.0.14
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: libseccomp-devel >= 2.3
BuildRequires: help2man >= 1.41
BuildRequires: libsecret-devel >= 0.18.4
BuildRequires: desktop-file-utils

Requires: libsodium >= 1.0.11
Requires: libcurl >= 7.29
Requires: libmicrohttpd >= 0.9.33
Requires: libseccomp >= 2.3
Requires: libsecret >= 0.18.4
Requires: yad
Requires: jq

BuildRoot:	%{_tmppath}/%{name}


%package oidc-agent-cli
Summary: Commandline tool for obtaining OpenID Connect Access tokens on the commandline

%package liboidc-agent4
Summary: oidc-agent library

%package liboidc-agent-dev
Summary: oidc-agent library development files

%package oidc-agent-desktop
Summary: oidc-agent desktop integration

%package oidc-agent
Summary: GUI integration for obtaining OpenID Connect Access tokens on the commandline


%description
oidc-agent is a set of tools to manage OpenID Connect tokens and make them
easily usable from the command line. These tools follow ssh-agent design,
so OIDC tokens can be handled in a similar way as ssh keys.  The agent
stores multiple configurations and their associated refresh tokens
securely.

%description oidc-agent-cli
Commandline tool for obtaining OpenID Connect access tokens on the commandline
This tool consists of five programs:
  - oidc-agent that handles communication with the OIDC provider
  - oidc-gen that generates config files
  - oidc-add that loads (and unloads) configuration into the agent
  - oidc-token that can be used to get access token on the command line
  - oidc-keychain that re-uses oidc-agent across logins

%description liboidc-agent4
oidc-agent dynamic library

%description liboidc-agent-dev
oidc-agent dynamic library development files

%description oidc-agent-desktop
Desktop integration files for oidc-gen and oidc-agent and for creating the user
dialog.
.
This package adds two ways for supporting the usage of oidc-agent in a
graphical environment.
The .desktop file to leverage browser integration to support the authorization
code flow in oidc-gen.
The Xsession file to consistently set the environment variables necessary to
for client tools to connect to the oidc-agent daemon.
.
This package also provides a bash script as an interface to create different
dialog windows. It uses yad to create windows.

%description oidc-agent
This metapackage bundles the commandline tools and the files for desktop
integration


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
make install \
  BIN_PATH=${RPM_BUILD_ROOT}%{_prefix} \
  BIN_AFTER_INST_PATH=${RPM_BUILD_ROOT}%{_prefix} \
  PROMPT_BIN_PATH=${RPM_BUILD_ROOT}%{_prefix} \
  MAN_PATH=${RPM_BUILD_ROOT}%{_mandir} \
  PROMPT_MAN_PATH=${RPM_BUILD_ROOT}%{_mandir} \
  CONFIG_PATH=${RPM_BUILD_ROOT}%{_sysconfdir} \
  BASH_COMPLETION_PATH=${RPM_BUILD_ROOT}%{_datarootdir}/bash-completion/completions \
  LIB_PATH=${RPM_BUILD_ROOT}%{_libdir} \
  DESKTOP_APPLICATION_PATH=${RPM_BUILD_ROOT}%{_datarootdir}/applications \
  XSESSION_PATH=${RPM_BUILD_ROOT}%{_sysconfdir}/X11 

# FIXME: This ought to be fixed elsewhere!
# fix paths in installed files
sed -i -e "s!${RPM_BUILD_ROOT}!!g" ${RPM_BUILD_ROOT}%{_sysconfdir}/X11/Xsession.d/91oidc-agent
sed -i -e "s!${RPM_BUILD_ROOT}!!g" ${RPM_BUILD_ROOT}%{_datarootdir}/applications/oidc-gen.desktop 

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/oidc-gen.desktop 

%clean
rm -rf $RPM_BUILD_ROOT

%files oidc-agent-cli
%defattr(-,root,root,-)
%config /etc/oidc-agent/
#%config /etc/oidc-agent/issuer.config
#%config /etc/oidc-agent/privileges/agentIpc.priv
#%config /etc/oidc-agent/privileges/crypt.priv
#%config /etc/oidc-agent/privileges/daemon.priv
#%config /etc/oidc-agent/privileges/general.priv
#%config /etc/oidc-agent/privileges/http.priv
#%config /etc/oidc-agent/privileges/httpserver.priv
#%config /etc/oidc-agent/privileges/kill.priv
#%config /etc/oidc-agent/privileges/logging.priv
#%config /etc/oidc-agent/privileges/memory.priv
#%config /etc/oidc-agent/privileges/print.priv
#%config /etc/oidc-agent/privileges/prompt.priv
#%config /etc/oidc-agent/privileges/read.priv
#%config /etc/oidc-agent/privileges/signal.priv
#%config /etc/oidc-agent/privileges/sleep.priv
#%config /etc/oidc-agent/privileges/socket.priv
#%config /etc/oidc-agent/privileges/time.priv
#%config /etc/oidc-agent/privileges/write.priv
#%config /etc/oidc-agent/pubclients.config
#%config /etc/oidc-agent/oidc-agent-service.options
%doc /usr/share/bash-completion/completions/
#%doc /usr/share/bash-completion/completions/oidc-agent
#%doc /usr/share/bash-completion/completions/oidc-gen
#%doc /usr/share/bash-completion/completions/oidc-keychain
#%doc /usr/share/bash-completion/completions/oidc-token
#%doc /usr/share/bash-completion/completions/oidc-agent-service
%doc /usr/share/man/man1/oidc-agent.1.gz
%doc /usr/share/man/man1/oidc-gen.1.gz
%doc /usr/share/man/man1/oidc-keychain.1.gz
%doc /usr/share/man/man1/oidc-token.1.gz
%doc /usr/share/man/man1/oidc-agent-service.1.gz
%{_bindir}/*

%files liboidc-agent4
%defattr(-,root,root,-)
/usr/lib64/liboidc-agent.so.4
/usr/lib64/liboidc-agent.so.%{version}

%files liboidc-agent-dev
%defattr(-,root,root,-)
usr/include/oidc-agent
usr/lib/x86_64-linux-gnu/liboidc-agent.so
usr/lib/x86_64-linux-gnu/liboidc-agent.a

%files oidc-agent-desktop
%defattr(-,root,root,-)
etc/X11/Xsession.d/
usr/bin/oidc-prompt
usr/share/applications/
%doc usr/share/man/man1/oidc-prompt.1
%config /etc/X11/Xsession.d/91oidc-agent
%doc /usr/share/applications/oidc-gen.desktop


%changelog
* Wed Aug 25 2021 Marcus Hardt <hardt@kit.edu>
- Restructured rpm packages to reflect debian structure
