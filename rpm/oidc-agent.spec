Name: oidc-agent
%define ver %(head debian/changelog -n 1|cut -d \\\( -f 2|cut -d \\\) -f 1|cut -d \- -f 1)
%define rel %(head debian/changelog -n 1|cut -d \\\( -f 2|cut -d \\\) -f 1|cut -d \- -f 2)
Version: %{ver}
Release: %{rel}
Summary: Command-line tool for obtaining OpenID Connect access tokens on the commandline

Group: Misc
License: MIT
URL: https://github.com/indigo-dc/oidc-agent
Source0: %{name}-%{version}.tar
# use `make rpmsource` to generate the required tarball
#Source0: https://github.com/indigo-dc/oidc-agent/archive/refs/tags/v%{version}.tar.gz

BuildRequires: libcurl-devel >= 7.29
BuildRequires: libsodium-devel >= 1.0.14
BuildRequires: libsodium-static >= 1.0.14
BuildRequires: libmicrohttpd-devel >= 0.9.33
BuildRequires: libseccomp-devel >= 2.3
BuildRequires: help2man >= 1.41
BuildRequires: libsecret-devel >= 0.18.4
BuildRequires: desktop-file-utils

Requires: oidc-agent-desktop == %{version}-%{release}
BuildRoot:	%{_tmppath}/%{name}

#cp /home/build/oidc-agent/rpm/oidc-agent.spec rpm && rpmbuild --define "_topdir /tmp/build/oidc-agent/rpm/rpmbuild" -bb rpm/oidc-agent.spec
%files
%defattr(-,root,root,-)
#%doc /usr/share/doc/README.md
#%doc /usr/share/doc/oidc-agent-4.1.1/README.md
%doc %{_defaultdocdir}/%{name}-%{version}/README.md


%package -n oidc-agent-cli
Summary: Commandline tool for obtaining OpenID Connect Access tokens on the commandline
Requires: liboidc-agent4 == %{version}-%{release}
Requires: libsodium >= 1.0.18
Requires: libcurl >= 7.29
Requires: libmicrohttpd >= 0.9.33
Requires: libseccomp >= 2.3.1
Requires: libsecret >= 0.18.6
Requires: glib2 >= 2.56.1
Requires: jq
Requires: qrencode

%package -n liboidc-agent4
Summary: oidc-agent library
Requires: libsodium >= 1.0.18

%package -n liboidc-agent-dev
Summary: oidc-agent library development files
Requires: liboidc-agent4 == %{version}-%{release}

%package -n oidc-agent-desktop
Summary: GUI integration for obtaining OpenID Connect Access tokens on the commandline
Requires: oidc-agent-cli == %{version}-%{release}
Requires: yad
Requires: xterm 


%description
oidc-agent is a set of tools to manage OpenID Connect tokens and make them
easily usable from the command line.
This metapackage bundles the commandline tools and the files for desktop
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

%description -n liboidc-agent4
oidc-agent is a commandline tool for obtaining OpenID Connect Access tokens on
the commandline.

This package provides a library for easy communication with oidc-agent.
Applications can use this library to request access token from oidc-agent.

%description -n liboidc-agent-dev
oidc-agent is a commandline tool for obtaining OpenID Connect Access tokens on
the commandline.

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
echo "Buildroot: ${RPM_BUILD_ROOT}"
echo "make install install_lib install_lib-dev \
    BIN_AFTER_INST_PATH      = ${RPM_BUILD_ROOT}%{_prefix}\
    BIN_PATH                 = ${RPM_BUILD_ROOT}%{_prefix}\
    MAN_PATH                 = ${RPM_BUILD_ROOT}%{_mandir}\
    CONFIG_PATH              = ${RPM_BUILD_ROOT}%{_sysconfdir}\
    CONFIG_AFTER_INST_PATH   = ${_sysconfdir}\
    BASH_COMPLETION_PATH     = ${RPM_BUILD_ROOT}%{_datarootdir}/bash-completion/completions\
    DESKTOP_APPLICATION_PATH = ${RPM_BUILD_ROOT}%{_datarootdir}/applications\
    XSESSION_PATH            = ${RPM_BUILD_ROOT}%{_sysconfdir}/X11\
    PROMPT_MAN_PATH          = ${RPM_BUILD_ROOT}%{_mandir}\
    PROMPT_BIN_PATH          = ${RPM_BUILD_ROOT}%{_prefix}\
    LIB_PATH                 = ${RPM_BUILD_ROOT}%{_libdir}\
    LIBDEV_PATH              = ${RPM_BUILD_ROOT}%{_libdir}\
    INCLUDE_PATH             = ${RPM_BUILD_ROOT}%{_includedir}"
make install install_lib install_lib-dev \
    BIN_AFTER_INST_PATH=${RPM_BUILD_ROOT}%{_prefix}\
    BIN_PATH=${RPM_BUILD_ROOT}%{_prefix}\
    MAN_PATH=${RPM_BUILD_ROOT}%{_mandir}\
    CONFIG_PATH=${RPM_BUILD_ROOT}%{_sysconfdir}\
    CONFIG_AFTER_INST_PATH=${_sysconfdir}\
    BASH_COMPLETION_PATH=${RPM_BUILD_ROOT}%{_datarootdir}/bash-completion/completions\
    DESKTOP_APPLICATION_PATH=${RPM_BUILD_ROOT}%{_datarootdir}/applications\
    XSESSION_PATH=${RPM_BUILD_ROOT}%{_sysconfdir}/X11\
    PROMPT_MAN_PATH=${RPM_BUILD_ROOT}%{_mandir}\
    PROMPT_BIN_PATH=${RPM_BUILD_ROOT}%{_prefix}\
    LIB_PATH=${RPM_BUILD_ROOT}%{_libdir}\
    LIBDEV_PATH=${RPM_BUILD_ROOT}%{_libdir}\
    INCLUDE_PATH=${RPM_BUILD_ROOT}%{_includedir}
# FIXME: This ought to be fixed elsewhere!
# fix paths in installed files
sed -i -e "s!${RPM_BUILD_ROOT}!!g" ${RPM_BUILD_ROOT}%{_sysconfdir}/X11/Xsession.d/91oidc-agent
sed -i -e "s!${RPM_BUILD_ROOT}!!g" ${RPM_BUILD_ROOT}%{_datarootdir}/applications/oidc-gen.desktop 
mkdir -p ${RPM_BUILD_ROOT}/%{_defaultdocdir}/%{name}-%{version}
cp README.md ${RPM_BUILD_ROOT}/%{_defaultdocdir}/%{name}-%{version}/README.md

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/oidc-gen.desktop 

%clean
rm -rf $RPM_BUILD_ROOT

%files -n oidc-agent-cli
%defattr(-,root,root,-)
%config /etc/oidc-agent/
%doc /usr/share/bash-completion/completions/
#%doc /usr/share/bash-completion/completions/oidc-agent
#%doc /usr/share/bash-completion/completions/oidc-gen
#%doc /usr/share/bash-completion/completions/oidc-keychain
#%doc /usr/share/bash-completion/completions/oidc-token
#%doc /usr/share/bash-completion/completions/oidc-agent-service
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
%{_libdir}/liboidc-agent.so.4
%{_libdir}/liboidc-agent.so.%{version}

%files -n liboidc-agent-dev
%defattr(-,root,root,-)
%{_includedir}/oidc-agent
%{_libdir}/liboidc-agent.so
%{_libdir}/liboidc-agent.a

%files -n oidc-agent-desktop
%defattr(-,root,root,-)
%{_sysconfdir}/X11/Xsession.d/
%{_bindir}/oidc-prompt
/usr/share/applications/
%doc /usr/share/man/man1/oidc-prompt.1.gz
%config /etc/X11/Xsession.d/91oidc-agent
%doc /usr/share/applications/oidc-gen.desktop


%changelog
* Wed Aug 25 2021 Marcus Hardt <hardt@kit.edu>
- Restructured rpm packages to reflect debian structure
