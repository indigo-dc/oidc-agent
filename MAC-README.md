Some Information on how to use oidc-agent on MacOS
Work In Progress

What does work:
- [Building oidc-agent](#building-oidc-agent)
- password / consent prompts that use ssh-askpass -> requires [building
  ssh-askpass](#building-ssh-askpass)
  - [this project](https://github.com/lukas-zronek/ssh-askpass-mac) might also
    work (not tested), but requires Xcode 9 or newer.

What does not work:
- syslog -> we implemented a custom logging behavior. Please create
  ```/usr/local/var/log``` to find the logs in
  ```/usr/local/var/log/oidc-agent.log``` or specify another path in
  ```src/utils/logger.c```.
  - Obviously, now oidc-agent has to write to disk, which sort of breaks
    privilege separation.
- bash completion -> chances are good to make it work with a newer version of bash
- seccomp
- storing a password in keyring -> it seems that there a problems with
  dbus, instructions on how to make it work welcome
- Xsession integration


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
TODO

### Build oidc-agent
Run make
```
make
```
### Post-Build
#### copy / link binaries
```
ln -s /usr/local/bin/oidc-agent ./bin/oidc-agent
ln -s /usr/local/bin/oidc-add ./bin/oidc-add
ln -s /usr/local/bin/oidc-gen ./bin/oidc-gen
ln -s /usr/local/bin/oidc-token ./bin/oidc-token
```

#### copy issuer.config and pubclients.conf
sudo mkdir -p /etc/oidc-agent
sudo cp ./config/issuer.config /etc/oidc-agent/issuer.config
sudo cp ./config/pubclients.config /etc/oidc-agent/pubclients.config

## Building ssh-askpass
- Download source code: https://github.com/zachmann/x11-ssh-askpass
- Install [XQuartz](https://www.xquartz.org/): ```brew cask install xquartz```
- Build: ```gcc -I/usr/X11/include x11-ssh-askpass.c drawing.c dynlist.c resources.c -o x11-ssh-askpass -L/usr/X11/lib -lX11 -lXt```
