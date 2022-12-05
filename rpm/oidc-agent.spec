Name: oidc-agent
Version: 4.4.3
Release: 1%{?dist}
%define VersionNoTilde %(echo %{version} | sed s/~pr/-pr/)
Summary: Command-line tool for obtaining OpenID Connect access tokens
%define commented_out 0

%if 0%{?suse_version} > 0
Group: Misc
%endif

# The entire source is MIT except:
#   - src/oidc-prompt/mustache/ is ISC; it is used in oidc-prompt, which is
#     in the -desktop subpackage
#     The ISC license is include in src/oidc-prompt/mustache/LICENSE.txt
#   - src/oidc-gen/qr.c is GPLv2+; it is ussed in oidc-gen, which is in the
#     -cli package
#   - src/utils/disableTracing.* src/utils/printer.h is ISC; it is used in
#     oidc-agent oidc-token, oidc-gen and oidc-add , which are in the -cli
#     subpackage and by oidc-prompt, which is in the -destkop subpackage


License: MIT
URL: https://github.com/indigo-dc/oidc-agent
# use `make rpmsource` to generate the required tarball
#Source0:
#https://github.com/indigo-dc/oidc-agent/archive/refs/heads/master.zip
#Source0:
#https://github.com/indigo-dc/oidc-agent/archive/refs/heads/docker-builds.zip
Source0: https://github.com/indigo-dc/oidc-agent/archive/v%{version}/oidc-agent-%{version}.tar.gz
#DO_NOT_REPLACE_THIS_LINE

BuildRequires: gcc >= 4.8
BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0.14
%if 0%{?suse_version} > 0
BuildRequires: unzip >= 6
%endif
%if 0%{?suse_version} > 0
BuildRequires: libsodium23 >= 1.0.14
%else
#BuildRequires: libsodium-static >= 1.0.16
BuildRequires: libsodium-devel >= 1.0.16
%endif
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: help2man >= 1.41
BuildRequires: libsecret-devel >= 0.18.4
BuildRequires: desktop-file-utils
BuildRequires: qrencode-devel >= 3
BuildRequires: gtk3-devel
%if 0%{?fedora} >= 1
BuildRequires: cjson-devel >= 1.7.12
%endif

# webkit2gtk3
%if 0%{?suse_version} > 0
%if 0%{?sle_version} > 150300
# 15.4 and larger
BuildRequires: webkit2gtk3-soup2-devel
%else
# 15.3 and tumbleweed
%if 0%{?suse_version} > 1590
# tumbleweed
BuildRequires: webkit2gtk3-soup2-devel
%else
# 15.3 and lower
BuildRequires: webkit2gtk3-devel
%endif
%endif
# non suse
%else
BuildRequires: webkitgtk4-devel
%endif

BuildRequires: gcc-c++

Requires: %{name}-desktop%{?_isa} = %{version}-%{release}


%package -n oidc-agent-cli
License: MIT and LGPL-2.1+ and ISC
Summary: Command-line tool for obtaining OpenID Connect Access tokens
Requires: liboidc-agent4 == %{version}-%{release}
Requires: libsecret >= 0.18.6
Requires: glib2 >= 2.56.1
Requires: jq
%if 0%{?suse_version} > 0
Requires: libqrencode4 >= 4
Requires: libsodium23 >= 1.0.16
Requires: libcurl4 >= 7.29
Requires: libmicrohttpd12 >= 0.9
%else
Requires: qrencode-libs >= 3
Requires: libsodium >= 1.0.18
Requires: libcurl >= 7.29
Requires: libmicrohttpd >= 0.9
%endif
Provides: oidc-agent >= 4.0

%package -n liboidc-agent4
License: MIT
Summary: Library for oidc-agent
%if 0%{?suse_version} > 0
Requires: libsodium23 >= 1.0.16
%else
Requires: libsodium >= 1.0.18
%endif

%package -n liboidc-agent-devel
License: MIT
Summary: Library development files for oidc-agent
Requires: liboidc-agent4%{?_isa} = %{version}-%{release}

%if 0%{?commented_out} != 1
%package -n oidc-agent-desktop
License: MIT and ISC
Summary: GUI integration for obtaining OpenID Connect Access tokens on the command-line
Requires: oidc-agent-cli == %{version}-%{release}
Requires: xterm
%if 0%{?suse_version} > 0
Requires: webkit2gtk3
Requires: gtk3
%else
#Requires: webkit2gtk3-minibrowser
Requires: webkitgtk4
Requires: gtk3
%endif
%endif


%description
oidc-agent is a set of tools to manage OpenID Connect tokens and make them
easily usable from the command-line.
This meta-package bundles the command-line tools and the files for desktop
integration

%description -n oidc-agent-cli
oidc-agent is a set of tools to manage OpenID Connect tokens and make them
easily usable from the command-line. These tools follow ssh-agent design,
so OIDC tokens can be handled in a similar way as ssh keys.  The agent
stores multiple configurations and their associated refresh tokens
securely.
This tool consists of five programs:
  - oidc-agent that handles communication with the OIDC provider
  - oidc-gen that generates config files
  - oidc-add that loads (and unloads) configuration into the agent
  - oidc-token that can be used to get access token on the command-line
  - oidc-key-chain that re-uses oidc-agent across logins

%description -n liboidc-agent4
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

%if 0%{?commented_out} != 1
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
dialog windows.
%endif


%prep
rm -rf windows
%if 0%{?fedora} >= 1
rm -rf lib/cJSON
%endif
#rm -rf lib/list
%setup -q

%build
%if 0%{?fedora} >= 1
export USE_CJSON_SO=1
#export USE_LIST_SO=1
%endif
make 

%install
echo "Buildroot: %{buildroot}"
make install install_lib install_lib-dev \
    BIN_AFTER_INST_PATH=%{_bindir}\
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
mkdir -p %{buildroot}/%{_defaultdocdir}/%{name}-%{version}
cp README.md %{buildroot}/%{_defaultdocdir}/%{name}-%{version}/README.md

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/oidc-gen.desktop 

%files -n oidc-agent
%license LICENSE

%files -n oidc-agent-cli
%dir %{_sysconfdir}/oidc-agent
%doc %{_defaultdocdir}/%{name}-%{version}
%config(noreplace) %{_sysconfdir}/oidc-agent/issuer.config
%config(noreplace) %{_sysconfdir}/oidc-agent/oidc-agent-service.options
%config(noreplace) %{_sysconfdir}/oidc-agent/pubclients.config
/usr/share/bash-completion/completions/*
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-agent.1.gz
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-gen.1.gz
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-add.1.gz
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-keychain.1.gz
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-token.1.gz
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-agent-service.1.gz
%{_bindir}/oidc-add
%{_bindir}/oidc-agent
%{_bindir}/oidc-agent-service
%{_bindir}/oidc-gen
%{_bindir}/oidc-keychain
%{_bindir}/oidc-token

%files -n liboidc-agent4
%license LICENSE
%{_libdir}/liboidc-agent.so.4
%{_libdir}/liboidc-agent.so.%{VersionNoTilde}

%files -n liboidc-agent-devel
%{_includedir}/oidc-agent
%{_libdir}/liboidc-agent.so
%exclude %attr(0644, root, root) %{_libdir}/liboidc-agent.a
# Strange that this one was actually included:
%exclude /usr/lib/.build-id/44/*

# exclude desktop files
%exclude %{_bindir}/oidc-prompt
%exclude %attr(0644, root, root) %doc /usr/share/man/man1/oidc-prompt.1.gz
%exclude %dir %{_sysconfdir}/X11/Xsession.d/
%exclude %config(noreplace) %{_sysconfdir}/X11/Xsession.d/91oidc-agent
%exclude /usr/share/applications/oidc-gen.desktop

%if 0%{?commented_out} != 1
%files -n oidc-agent-desktop
%{_bindir}/oidc-prompt
%attr(0644, root, root) %doc /usr/share/man/man1/oidc-prompt.1.gz
%dir %{_sysconfdir}/X11/Xsession.d/
%config(noreplace) %{_sysconfdir}/X11/Xsession.d/91oidc-agent
/usr/share/applications/oidc-gen.desktop
%endif


%changelog
* Mon Jul 25 2022 Marcus Hardt <hardt@kit.edu> - 4.3.2-1
- Restructured rpm packages to fix fedora bugzilla #1997994
* Wed Aug 25 2021 Marcus Hardt <hardt@kit.edu> - 4.1.1-3
- Restructured rpm packages to reflect debian structure
