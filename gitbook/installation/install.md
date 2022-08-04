# Installation

This document describes how to install oidc-agent on linux. To install oidc-agent on Windows or MacOS refer to
the  [Windows documentation](windows.md) and [MacOS documentation](macos.md), respectively.

## From Package

Please check if your distribution already includes oidc-agent. In this case installing is as simple as

```terminal
sudo apt-get install oidc-agent
```

If your distribution does not include oidc-agent, packaged versions of oidc-agent are available for many different
distros at **[http://repo.data.kit.edu](http://repo.data.kit.edu)**.

## From Source

If you want to build oidc-agent from source you can do so.

### Dependencies

#### Basic Dependencies

To be able to build oidc-agent, you need at least the following dependencies installed on your system:

- gcc
- make
- [libcurl](https://curl.haxx.se/libcurl/) (libcurl4-openssl-dev)
- [libsodium (>= 1.0.14)](https://download.libsodium.org/doc/) (libsodium-dev)
- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) (libmicrohttpd-dev)
- libsecret (libsecret-1-dev)
- libqrencode (libqrencode-dev)
- libwebkit
    - Debian / Ubuntu: webkit2-gtk (libwebkit2gtk-4.0-dev)
    - Centos / Fedora: webkitgtk4 (webkitgtk4-devel)
    - SuSE: webkit2gtk3 (webkit2gtk3-soup2-devel)

##### Debian/Ubuntu

```
sudo apt-get install \
      libcurl4-openssl-dev \
      libsodium-dev \
      libmicrohttpd-dev \
      libsecret-1-dev \
      libqrencode-dev \
      libwebkit2gtk-4.0-dev
```

##### Centos / Fedora

```
sudo yum install \
    libcurl-devel \
    libsodium-devel \
    libmicrohttpd-devel \
    libsecret-devel \
    qrencode-devel \
    webkitgtk4-devel
```

##### OpenSuSE

```
sudo zypper install \
    libcurl-devel \
    libsodium-devel \
    libmicrohttpd-devel \
    libsecret-devel \
    qrencode-devel \
    webkit2gtk3-soup2-devel
```


#### Additional Build Dependencies

oidc-agent can be installed easiest from package. So even when building from source it is recommended to build the
package and install it.

Building the deb/rpm package might have additional dependencies. More
information about those are found in the `rpm` or the `debian` subfolders.
Note: The `debian` subfolder may only be available on the [Debian
Salsa](https://salsa.debian.org/debian/oidc-agent) git.

Dockerised builds are supported via make targets, such as:
- `dockerised_rpm_centos_7`
- `dockerised_rpm_centos_8`
- `dockerised_rpm_rockylinux_8.5`
- `dockerised_rpm_opensuse_15.3`
- `dockerised_rpm_opensuse_15.4`
- `dockerised_rpm_opensuse_tumbleweed`
- `dockerised_rpm_fedora_36`
- `dockerised_all_deb_packages: dockerised_deb_debian_bullseye`

The debian package targets are defined on the branches in the [Debian
Salsa](https://salsa.debian.org/debian/oidc-agent) git:
- `dockerised_deb_debian_buster`
- `dockerised_deb_debian_bookworm`
- `dockerised_deb_ubuntu_focal`
- `dockerised_deb_ubuntu_jammy`
- `dockerised_deb_ubuntu_impish`
- `dockerised_deb_ubuntu_hirsute`
- `dockerised_deb_ubuntu_kinetic`


### Download oidc-agent

After installing the necessary dependencies, one has to obtain a copy of the source. Possible ways are:

- clone the git repository
- download a [release version](https://github.com/indigo-dc/oidc-agent/releases)
- download the source from [GitHub](https://github.com/indigo-dc/oidc-agent)

#### Using git

```
git clone https://github.com/indigo-dc/oidc-agent
cd oidc-agent
```

#### Using curl

```
curl -L  https://github.com/indigo-dc/oidc-agent/archive/master.tar.gz -o /tmp/oidc-agent-master.tar.gz
tar xzf /tmp/oidc-agent-master.tar.gz
cd oidc-agent
```

### Build and install oidc-agent

#### Building Binaries

The binaries can be build with `make`. To build and install run:

```
make
sudo make install_lib
sudo make install
sudo make post_install
```

This will:

- build the binaries
- create man pages
- install the binaries
- install the man pages
- install configuration files
- install bash completion
- install a custom scheme handler
- enable the linker to use the newly installed libraries
- update the desktop database to enable the custom scheme handler

If you want to install any of these files to another location you can pass a different path to make.
E.g. `sudo make install BIN_PATH=/home/user` will install the binaries in
`/home/user/bin` instead of `/usr/bin`.

One could also run only `make` and manually copy the necessary files to another location and / or add the binaries'
location to `PATH`. However, this is not recommend, because some files have to be placed at specific locations or
additional configuration is needed. But it is also possible to install only a subset of all files, by calling the
different `make install_`
rules. Available targets are:

```
sudo make install_bin
sudo make install_man
sudo make install_conf
sudo make install_bash
sudo make install_scheme_handler
sudo make install_xsession_script
sudo make install_lib
sudo make install_lib-dev
```
