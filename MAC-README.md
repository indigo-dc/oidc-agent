# How to use oidc-agent with MAC OS

## State of Feature Support
### What does work:
- [Installing oidc-agent using homebrew](#install-with-homebrew)
- [Building oidc-agent manually](#building-oidc-agent)
- password / consent prompts that use ssh-askpass -> requires [installing
  x11-ssh-askpass](#installing-ssh-askpass) or [building
  ssh-askpass](#building-ssh-askpass)
  - [this project](https://github.com/lukas-zronek/ssh-askpass-mac) might also
    work (not tested), but requires Xcode 9 or newer.

### What does not work:
- syslog -> we implemented a custom logging behavior. Please create
  ```/usr/local/var/log``` to find the logs in
  ```/usr/local/var/log/oidc-agent.log``` or specify another path in
  ```src/utils/logger.c```.
  - Obviously, now oidc-agent has to write to disk, which sort of breaks
    privilege separation.
- bash completion -> chances are good to make it work with a newer version of bash (feel free to check things out and submit a pull request)
- seccomp
- storing a password in the keyring -> if one can make it work, pull requests /
  instructions are welcome.
- Xsession integration

## Install with Homebrew
oidc-agent can be installed easiest using homebrew.
```
brew tap indigo-dc/oidc-agent
brew install oidc-agent
```
To use GUI prompting one most likely needs to [build ssh-askpass](#building-ssh-askpass).

## Building oidc-agent
### Download source code
- Install Xcode
- git clone: ```git clone https://github.com/indigo-dc/oidc-agent```

### Install dependencies:
- Install [brew](https://brew.sh) to install dependencies
- pkgconfig ```brew install pkgconfig```
- argp ```brew install argp-standalone```
- libsodium ```brew install libsodium```
- libmicrohttpd ```brew install libmicrohttpd```
<!-- - libsecret-1 ```brew install libsecret-1``` -->
<!--   - you might have to add ```/usr/local/opt/libffi/lib/pkgconfig``` to ```$PKG_CONFIG_PATH``` -->
- help2man ```brew install help2man```

### Build oidc-agent
Run make
```
make
```
### Post-Build
### Install using make
```
make install
make post_install
```
This installs all necessary components to correct locations.

## Getting ssh-askpass
### Installing ssh-askpass
A X11 ssh-askpass version (specially for oidc-agent) cna be isntalled using
homebrew.
```
brew tap zachmann/x11-ssh-askpass
brew install x11-ssh-askpass
```

### Building ssh-askpass
- Download source code: https://github.com/zachmann/x11-ssh-askpass
- Install [XQuartz](https://www.xquartz.org/): ```brew cask install xquartz```
- Build: ```gcc -I/usr/X11/include x11-ssh-askpass.c drawing.c dynlist.c resources.c -o x11-ssh-askpass -L/usr/X11/lib -lX11 -lXt```
- Copy it to a location in your path and name it `ssh-askpass`. ```cp x11-ssh-askpass
  /usr/local/bin/ssh-askpass```
