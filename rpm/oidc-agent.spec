Name: oidc-agent
Version: 4.3.1
Release: 1%{?dist}

Summary: Command-line tool for obtaining OpenID Connect access tokens

%if 0%{?suse_version} > 0
Group: Misc
%endif
License: MIT
URL: https://github.com/indigo-dc/oidc-agent
# use `make rpmsource` to generate the required tarball
#Source0: https://github.com/indigo-dc/oidc-agent/archive/refs/heads/master.zip
#Source0: https://github.com/indigo-dc/oidc-agent/archive/refs/heads/docker-builds.zip
Source0: https://github.com/indigo-dc/oidc-agent/archive/refs/tags/v%{version}.tar.gz
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
BuildRequires: libsodium-static >= 1.0.16
%endif
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: help2man >= 1.41
BuildRequires: libsecret-devel >= 0.18.4
BuildRequires: desktop-file-utils
BuildRequires: qrencode-devel >= 3
BuildRequires: gtk3-devel

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
BuildRequires: webkit2gtk3-devel
%endif

BuildRequires: gcc-c++

Requires: oidc-agent-desktop == %{version}-%{release}
BuildRoot:	%{_tmppath}/%{name}

#cp /home/build/oidc-agent/rpm/oidc-agent.spec rpm && rpmbuild --define "_topdir /tmp/build/oidc-agent/rpm/rpmbuild" -bb rpm/oidc-agent.spec
%files
%defattr(-,root,root,-)
%doc %{_defaultdocdir}/%{name}-%{version}
%license LICENSE


%package -n oidc-agent-cli
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

%package -n liboidc-agent4
Summary: oidc-agent library
%if 0%{?suse_version} > 0
Requires: libsodium23 >= 1.0.16
%else
Requires: libsodium >= 1.0.18
%endif

%package -n liboidc-agent-devel
Summary: oidc-agent library development files
Requires: liboidc-agent4 == %{version}-%{release}

%package -n oidc-agent-desktop
Summary: GUI integration for obtaining OpenID Connect Access tokens on the command-line
Requires: oidc-agent-cli == %{version}-%{release}
Requires: xterm 
%if 0%{?suse_version} > 0
Requires: webkit2gtk3
Requires: gtk3
%else
#Requires: webkit2gtk3-minibrowser
Requires: webkit2gtk3
Requires: gtk3
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
mkdir -p %{buildroot}/%{_defaultdocdir}/%{name}-%{version}
cp README.md %{buildroot}/%{_defaultdocdir}/%{name}-%{version}/README.md

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/oidc-gen.desktop 

%files -n oidc-agent-cli
%defattr(-,root,root,-)
%license LICENSE
%config(noreplace) /etc/oidc-agent/issuer.config
%config(noreplace) /etc/oidc-agent/oidc-agent-service.options
%config(noreplace) /etc/oidc-agent/pubclients.config
/usr/share/bash-completion/completions/
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
%defattr(-,root,root,-)
%license LICENSE
%{_libdir}/liboidc-agent.so.4
%{_libdir}/liboidc-agent.so.%{version}

%files -n liboidc-agent-devel
%defattr(-,root,root,-)
%license LICENSE
%{_includedir}/oidc-agent
%{_libdir}/liboidc-agent.so
%attr(0644, root, root) %{_libdir}/liboidc-agent.a

%files -n oidc-agent-desktop
%defattr(-,root,root,-)
%license LICENSE
%{_bindir}/oidc-prompt
%doc /usr/share/man/man1/oidc-prompt.1.gz
%config(noreplace) /etc/X11/Xsession.d/91oidc-agent
/usr/share/applications/oidc-gen.desktop


%changelog
* Wed Aug 25 2021 Marcus Hardt <hardt@kit.edu> - 4.1.1-3
- Restructured rpm packages to reflect debian structure
