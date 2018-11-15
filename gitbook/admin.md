# Deployment And Administration Guide
## Installation
### From Package
We provide packages for Debian and Ubuntu. They are available at
http://cvs.fzk.de/oidc-agent/ or at [GitHub](https://github.com/indigo-dc/oidc-agent/releases).

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
- libseccomp (libseccomp-dev)
- help2man (help2man)

Optional:
- qrencode (only required for generating an optional QR-Code when using the device flow)

##### Debian/Ubuntu
```
# apt-get install libcurl4-openssl-dev
# apt-get install libsodium-dev
# apt-get install libmicrohttpd-dev
# apt-get install libseccomp-dev
# apt-get install help2man
```

##### CentOS 7
```
# yum install libcurl-devel
# yum install libsodium-devel
# yum install libmicrohttpd-devel
# yum install libseccomp-devel
# yum install help2man
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

One can now use ```make install``` to copy the binaries to e.g. `/usr/bin` or
add the directory ```oidc-agent/bin``` to your ```$PATH```. When the binaries
are not installed using ```make install``` you also have to copy some
configuration files:
```
sudo mkdir /etc/oidc-agent
sudo cp config/issuer.config /etc/oidc-agent/issuer.config
sudo cp -r config/privileges/ /etc/oidc-agent/
sudo cp config/bash-completion/* /usr/share/bash-completion/completions/
```

## Configuration
An oidc-agent directory will be created when using oidc-gen for the first time. 
If ```~/.config``` exists it will be ```~/.config/oidc-agent``` otherwise ```~/.oidc-agent```

One can configure issuer urls in the config file ```issuer.config``` located in
this oidc directory. These issuer urls will be used by oidc-gen as a suggestion.

