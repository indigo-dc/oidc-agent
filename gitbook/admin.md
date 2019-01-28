# Deployment And Administration Guide
## Installation
### From Package
We provide packages for Debian and Ubuntu. They are available at
http://repo.data.kit.edu/ or at [GitHub](https://github.com/indigo-dc/oidc-agent/releases).
Packages for CentOS 7 are available at http://repo.indigo-datacloud.eu/repository/deep-hdc/production/1/centos7/x86_64/third-party/

You can download and install the packages manually or you can include our apt
repository:

- `sudo apt-key adv --keyserver hkp://hkps.pool.sks-keyservers.net --recv-keys ACDFB08FDC962044D87FF00B512839863D487A87`

- Depending on your distribution, choose one of the following lines:
     ``` 
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/stable ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/stretch ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/testing ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/debian/buster ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/16.04 ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/xenial ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/18.04 ./"
     sudo add-apt-repository "deb http://repo.data.kit.edu/ubuntu/bionic ./"
     ```

- `sudo apt-get update`

- `sudo apt-get install oidc-agent`

### From Source
#### Requirements
To be able to install oidc-agent from source, you need the following dependencies
installed on your system:
- gcc
- make
- [libcurl](https://curl.haxx.se/libcurl/) (libcurl4-openssl-dev)  
- [libsodium (>= 1.0.14)](https://download.libsodium.org/doc/) (libsodium-dev)
- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) (libmicrohttpd-dev)
- libseccomp (libseccomp-dev)
- help2man (help2man)

Optional:
- qrencode (only required for generating an optional QR-Code when using the device flow)

##### Debian/Ubuntu
```
# apt-get install \
      libcurl4-openssl-dev \
      libsodium-dev \
      help2man \
      libseccomp-dev \
      libmicrohttpd-dev 
```

##### CentOS 7
```
# yum install libcurl-devel
# yum install libsodium-devel
# yum install libsodium-static
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
Additionally the shared library has to registered with the dynamic linker. The
easist way is installing the library with ```sudo make install_lib``` and then run ```ldconfig```:
```
sudo make install_lib
sudo ldconfig
```
One can also use another way (e.g. ```LD_LIBRARY_PATH``` to register the library
with the linker).

## Configuration
An oidc-agent directory will be created when using oidc-gen for the first time. 
If ```~/.config``` exists it will be ```~/.config/oidc-agent``` otherwise ```~/.oidc-agent```

One can configure issuer urls in the config file ```issuer.config``` located in
this oidc directory. These issuer urls will be used by oidc-gen as a suggestion.

