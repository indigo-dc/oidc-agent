Name: oidc-agent
Version: 4.1.1
Release: 1%{?dist}
Summary: Command-line tool for obtaining OpenID Connect access tokens on the command-line

License: MIT
URL: https://github.com/indigo-dc/oidc-agent
# use `make rpmsource` to generate the required tarball
Source0: https://github.com/indigo-dc/oidc-agent/archive/refs/tags/v%{version}.tar.gz

BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0.14
%if 0%{?suse_version} > 0
BuildRequires: libsodium23 >= 1.0.14
%else
BuildRequires: libsodium-static >= 1.0.16
%endif
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: libseccomp-devel >= 2.3
BuildRequires: help2man >= 1.41
BuildRequires: libsecret-devel >= 0.18.4
BuildRequires: desktop-file-utils

Requires: oidc-agent-desktop = %{version}-%{release}

%package -n oidc-agent-cli
Summary: Command-line tool for obtaining OpenID Connect Access tokens on the command-line
Requires: liboidc-agent = %{version}-%{release}
Requires: libsodium >= 1.0.18
Requires: libcurl >= 7.29
Requires: libmicrohttpd >= 0.9.33
Requires: libseccomp >= 2.3.1
Requires: libsecret >= 0.18.6
Requires: glib2 >= 2.56.1
Requires: jq
Requires: qrencode

%package -n liboidc-agent
Summary: oidc-agent library
Requires: libsodium >= 1.0.18

%package -n liboidc-agent-devel
Summary: oidc-agent library development files
Requires: liboidc-agent = %{version}-%{release}

%package -n oidc-agent-desktop
Summary: GUI integration for obtaining OpenID Connect Access tokens on the command-line
Requires: oidc-agent-cli = %{version}-%{release}
Requires: yad
Requires: xterm


%description
oidc-agent is a set of tools to manage OpenID Connect tokens and make them
easily usable from the command line.
This meta-package bundles the command-line tools and the files for desktop
integration

%description -n oidc-agent-cli
oidc-agent is a set of tools to manage OpenID Connect tokens and make them
easily usable from the command line. These tools follow ssh-agent design,
so OIDC tokens can be handled in a similar way as ssh keys.  The agent
stores multiple configurations and their associated refresh tokens
securely.
This tool consists of five programs:
  - oidc-agent that handles communication with the OIDC provider
  - oidc-gen that generates config files
  - oidc-add that loads (and unloads) configuration into the agent
  - oidc-token that can be used to get access token on the command line
  - oidc-keychain that re-uses oidc-agent across logins

%description -n liboidc-agent
oidc-agent is a command-line tool for obtaining OpenID Connect Access tokens on
the command-line.

This package provides a library for easy communication with oidc-agent.
Applications can use this library to request access token from oidc-agent.

%description -n liboidc-agent-devel
oidc-agent is a command-line tool for obtaining OpenID Connect Access tokens on
the command-line.

This package provides the development files (static library and headers)
required for building applications with liboidc-agent, a library for
communicating with oidc-agent.

%description -n oidc-agent-desktop
Desktop integration files for oidc-gen and oidc-agent and for creating the user
dialog.

This package adds two ways for supporting the usage of oidc-agent in a
graphical environment.
The .desktop file to leverage browser integration to support the authorization
code flow in oidc-gen.
The Xsession file to consistently set the environment variables necessary to
for client tools to connect to the oidc-agent daemon.

This package also provides a bash script as an interface to create different
dialog windows. It uses yad to create windows.


%prep
%setup -q


%build
export USE_CJSON_SO=0
export USE_LIST_SO=0
make


%install
echo "Buildroot: %{buildroot}"
make install install_lib install_lib-dev \
    BIN_AFTER_INST_PATH=%{buildroot}%{_prefix}\
    BIN_PATH=%{buildroot}%{_prefix}\
    MAN_PATH=%{buildroot}%{_mandir}\
    CONFIG_PATH=%{buildroot}%{_sysconfdir}\
    CONFIG_AFTER_INST_PATH=${_sysconfdir}\
    BASH_COMPLETION_PATH=%{buildroot}%{_datarootdir}/bash-completion/completions\
    DESKTOP_APPLICATION_PATH=%{buildroot}%{_datarootdir}/applications\
    XSESSION_PATH=%{buildroot}%{_sysconfdir}/X11\
    PROMPT_MAN_PATH=%{buildroot}%{_mandir}\
    PROMPT_BIN_PATH=%{buildroot}%{_prefix}\
    LIB_PATH=%{buildroot}%{_libdir}\
    LIBDEV_PATH=%{buildroot}%{_libdir}\
    INCLUDE_PATH=%{buildroot}%{_includedir}
# FIXME: This ought to be fixed elsewhere!
# fix paths in installed files
sed -i -e "s!%{buildroot}!!g" %{buildroot}%{_sysconfdir}/X11/Xsession.d/91oidc-agent
sed -i -e "s!%{buildroot}!!g" %{buildroot}%{_datarootdir}/applications/oidc-gen.desktop

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/oidc-gen.desktop


%files
%defattr(-,root,root,-)
%doc README.md

%files -n oidc-agent-cli
%defattr(-,root,root,-)
%config %{_sysconfdir}/oidc-agent/
%{_bindir}/oidc-add
%{_bindir}/oidc-agent
%{_bindir}/oidc-agent-service
%{_bindir}/oidc-gen
%{_bindir}/oidc-keychain
%{_bindir}/oidc-token
%{_datarootdir}/bash-completion/completions
%{_mandir}/man1/oidc-agent.1.gz
%{_mandir}/man1/oidc-gen.1.gz
%{_mandir}/man1/oidc-add.1.gz
%{_mandir}/man1/oidc-keychain.1.gz
%{_mandir}/man1/oidc-token.1.gz
%{_mandir}/man1/oidc-agent-service.1.gz

%files -n liboidc-agent
%defattr(-,root,root,-)
%{_libdir}/liboidc-agent.so.4
%{_libdir}/liboidc-agent.so.%{version}

%files -n liboidc-agent-devel
%defattr(-,root,root,-)
%{_includedir}/oidc-agent
%{_libdir}/liboidc-agent.so
%{_libdir}/liboidc-agent.a

%files -n oidc-agent-desktop
%defattr(-,root,root,-)
%config %{_sysconfdir}/X11/Xsession.d/91oidc-agent
%{_bindir}/oidc-prompt
%{_datarootdir}/applications/oidc-gen.desktop
%{_mandir}/man1/oidc-prompt.1.gz


%changelog
* Wed Aug 25 2021 Marcus Hardt <hardt@kit.edu> - 4.1.1-1
- Restructured rpm packages to reflect debian structure
