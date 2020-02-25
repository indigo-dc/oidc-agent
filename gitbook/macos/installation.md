## Installation

### Install with Homebrew
oidc-agent can be installed easiest using homebrew.
```
brew tap indigo-dc/oidc-agent
brew install oidc-agent
```
To use GUI prompting one most likely needs to [install ssh-askpass](#installing-ssh-askpass).

### Building oidc-agent manually
#### Download source code
- Install Xcode
- git clone: `git clone https://github.com/indigo-dc/oidc-agent`

#### Install dependencies:
- Install [brew](https://brew.sh) to install dependencies
- pkgconfig `brew install pkgconfig`
- argp `brew install argp-standalone`
- libsodium `brew install libsodium`
- libmicrohttpd `brew install libmicrohttpd`
<!-- - libsecret-1 `brew install libsecret-1` -->
<!--   - you might have to add `/usr/local/opt/libffi/lib/pkgconfig` to `$PKG_CONFIG_PATH` -->
- help2man `brew install help2man`

#### Build oidc-agent
Run make
```
make
```
#### Install using make
```
make install
make post_install
```
This installs all necessary components to correct locations.

### Installing ssh-askpass
#### Install using homebrew
An X11 ssh-askpass version (specially for oidc-agent) can be installed using
homebrew.
```
brew tap zachmann/x11-ssh-askpass
brew install x11-ssh-askpass
```

#### Building ssh-askpass manually
- Download source code: https://github.com/zachmann/x11-ssh-askpass
- Install [XQuartz](https://www.xquartz.org/): `brew cask install xquartz`
- Build: `gcc -I/usr/X11/include x11-ssh-askpass.c drawing.c dynlist.c resources.c -o x11-ssh-askpass -L/usr/X11/lib -lX11 -lXt`
- Copy it to a location in your path and name it `ssh-askpass`. `cp x11-ssh-askpass
  /usr/local/bin/ssh-askpass`

