# Deployment And Administration Guide
## Installation
### From Package
We provide packages for Debian and CentOS 7. They are available at
http://marcus.hardt-it.de/oidc-agent/.

For informations on how to add the repository to your system refer to the
documentation of your operating system.

### From Source
#### Requirements
To be able to install oidc-agent from source, you need gcc, make, 
[libcurl](libcurl4-openssl-dev) and 
[libsodium (>= 1.0.11)](libcurl4-openssl-dev) installed on our system.

##### Debian/Ubuntu
```
apt-get install libcurl4-openssl-dev
apt-get install libsodium-dev
```
Note: On debian jessie you have to use jessie-backports for libsodium-dev.

##### CentOS 7
```
yum install libcurl-devel
yum install libsodium-devel
```

#### Build oidc-agent
The next steps download the source and build oidc-agent:
- clone the git repository or download a [release version](https://github.com/indigo-dc/oidc-agent/releases)
- compile oidc-agent
```
git clone https://github.com/indigo-dc/oidc-agent
cd oidc-agent
make
```
The binary executables are in the subdirectory `bin`.

One could now use ```make install``` to copy the binaries to e.g. `/usr/bin`.
