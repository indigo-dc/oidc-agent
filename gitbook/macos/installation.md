## Installation

### Install with Homebrew

oidc-agent can be installed easiest using homebrew.

```
brew tap indigo-dc/oidc-agent
brew install oidc-agent
```

### Building oidc-agent manually

#### Download source code

- git clone: `git clone https://github.com/indigo-dc/oidc-agent`

#### Install dependencies:

- Install [brew](https://brew.sh) to install dependencies
- pkgconfig `brew install pkgconfig`
- argp `brew install argp-standalone`
- libsodium `brew install libsodium`
- libmicrohttpd `brew install libmicrohttpd`
- qrencode `brew install qrencode`

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


