# Installation

This document describes how to install oidc-agent on linux. To install oidc-agent on Windows or MacOS refer to
the  [Windows documentation](windows/installation.md) and [MacOS documentation](macos/installation.md), respectively.

## From Package

Please check if your distribution already includes oidc-agent. In this case installing is as simple as

```terminal
sudo apt-get install oidc-agent
```

If your distribution does not include oidc-agent, packaged versions of oidc-agent are available for many different
distros at https://repo.data.kit.edu

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
- gtk3 (libgtk-3-dev)
- webkit2-gtk (libwebkit2gtk-4.0-dev)

##### Debian/Ubuntu

```
sudo apt-get install \
      libcurl4-openssl-dev \
      libsodium-dev \
      libmicrohttpd-dev \
      libsecret-1-dev \
      libqrencode-dev \
      libgtk-3-dev \
      libwebkit2gtk-4.0-dev
```

#### Additional Build Dependencies

oidc-agent can be installed easiest from package. So even when building from source it is recommended to build the
package and install it.

Building the deb/rpm package might have additional dependencies.

- help2man
- check
- debhelper
- pkg-config
- perl
- sed
- fakeroot
- devscripts

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
