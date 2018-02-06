# Deployment And Administration Guide
## Installation
### From Package
We provide packages for Debian and CentOS 7. They are available at
http://marcus.hardt-it.de/oidc-agent/ or at [GitHub](https://github.com/indigo-dc/oidc-agent/releases).

For informations on how to install the package on your system refer to the
documentation of your operating system.

### From Source
#### Requirements
To be able to install oidc-agent from source, you need the following dependencies
installed on your system:
- gcc
- make
- [libcurl](https://curl.haxx.se/libcurl/) (libcurl4-openssl-dev)  
- [libsodium (>= 1.0.11)](https://download.libsodium.org/doc/) (libcurl4-openssl-dev)
- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) (libmicrohttpd-dev)
- help2man (help2man)

##### Debian/Ubuntu
```
apt-get install libcurl4-openssl-dev
apt-get install libsodium-dev
apt-get install libmicrohttpd-dev
apt-get install help2man
```
Note: On debian jessie you have to use jessie-backports for libsodium-dev.

##### CentOS 7
```
yum install libcurl-devel
yum install libsodium-devel
yum install libmicrohttpd-devel
yum install help2man
```

#### Build oidc-agent
The next steps: Download the source and build oidc-agent:
- clone the git repository or download a [release version](https://github.com/indigo-dc/oidc-agent/releases)
- compile oidc-agent
```
git clone https://github.com/indigo-dc/oidc-agent
cd oidc-agent
make
```
The binary executables are in the subdirectory `bin`.

One could now use ```make install``` to copy the binaries to e.g. `/usr/bin`.

## Configuration
An oidc-agent directory will be created during the installation process. 
If ```~/.config``` exists it will be ```~/.config/oidc-agent``` otherwise ```~/.oidc-agent```

One can configure issuer urls in the config file ```issuer.config``` located in
this oidc directory. These issuer urls will be used by oidc-gen as a suggestion.

