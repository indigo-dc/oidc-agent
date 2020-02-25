# Installation
This document describes how to install oidc-agent on linux. To install
oidc-agent on MacOS refer to the [MacOS
documentation](../macos/installation.md).
## From Package
We provide packages for Debian, Ubuntu and CentOS 7. They are available at
http://repo.data.kit.edu/ or at [GitHub](https://github.com/indigo-dc/oidc-agent/releases).
Packages for CentOS 7 are available at http://repo.indigo-datacloud.eu/repository/deep-hdc/production/1/centos7/x86_64/third-party/

You can download and install the packages manually or you can include our apt
repository:

- `sudo apt-key adv --keyserver hkp://pgp.surfnet.nl --recv-keys ACDFB08FDC962044D87FF00B512839863D487A87`

- Depending on your distribution, choose one of the following lines:
     ``` 
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/buster ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/stretch ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/bullseye ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/bionic ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/xenial ./"
    ```
- `sudo apt-get update`
- `sudo apt-get install oidc-agent`

## From Source
If you want to build oidc-agent from source you can do so.

### Dependencies
#### Basic Dependencies
To be able to build oidc-agent, you need at least the following dependencies
installed on your system:
- gcc
- make
- [libcurl](https://curl.haxx.se/libcurl/) (libcurl4-openssl-dev)  
- [libsodium (>= 1.0.14)](https://download.libsodium.org/doc/) (libsodium-dev)
- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) (libmicrohttpd-dev)
- libseccomp (libseccomp-dev)
- libsecret (libsecret-1-dev)

We note that libsodium-dev  might not be available by default on all systems 
with the required version of at least `1.0.14`. It might be included in backports
or has to build from source.

##### Debian/Ubuntu
```
sudo apt-get install \
      libcurl4-openssl-dev \
      libsodium-dev \
      libseccomp-dev \
      libmicrohttpd-dev \
      libsecret-1-dev
```

##### CentOS 7
```
sudo yum install \
      libcurl-devel \
      libsodium-devel \
      libsodium-static \
      libmicrohttpd-devel \
      libseccomp-devel \
      libsecret-devel
```
#### Additional Build Dependencies
oidc-agent can be installed easiest from package. So even when building from
source it is recommended to build the package and install it.

Building the deb/rpm package might have additional dependencies.
- help2man
- check
- debhelper
- pkg-config
- perl
- sed
- fakeroot
- devscripts

#### Optional Runtime Dependencies
Some features require additional dependencies.
- ssh-askpass (required for password prompting by the agent)
- qrencode    (required for generating an optional QR-Code when using the device flow)

### Download oidc-agent
After installing the necessary dependencies, one has to obtain a copy of the
source. Possible ways are:
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
As already mentioned, oidc-agent can be installed easiest by using a debian or
rpm package. It is therefore recommend to build such a package.

#### Building a package
##### Debian / Ubuntu
To build a debian package and install it run the following commands inside the
oidc-agent source directory:
```
make deb
sudo dpkg -i ../liboidc-agent3_<version>_amd64.deb
sudo dpkg -i ../oidc-agent_<version>_amd64.deb
```

##### CentOS 7
To build an rpm package and install it run the following commands inside the
oidc-agent source directory:
```
make rpm
sudo yum install ../oidc-agent-<version>-1.x86_64.rpm
```

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
- install a custome scheme handler
- enable a linker to use the newly installed libraries
- update the desktop database to enable the custome scheme handler

If you want to install any of these files to another location you can pass a
different path to make.
E.g. `sudo make install BIN_PATH=/home/user` will install the binaries in
`/home/user/bin` instead of `/usr/bin`.

One could also run only `make` and manually copy the necessary
files to another location and / or add the binaries' location to `PATH`.
However, this is not recommend, because some files have to be placed at specific
locations or additional configuration is needed. But it is also possible to
install only a subset of all files, by calling the different `make install_`
rules. Available targets are:
```
sudo make install_bin
sudo make install_man
sudo make install_conf
sudo make install_bash
sudo make install_priv
sudo make install_scheme_handler
sudo make install_xsession_script
sudo make install_lib
sudo make install_lib-dev
```

