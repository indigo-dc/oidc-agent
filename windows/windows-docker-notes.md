# vim: ft=sh

# Simple bash

docker run -u Administrator --rm -i -e PATH=/usr/bin -v $PWD\var:c:/msys64/var/lib/pacman msys2 C:
/msys64/usr/bin/bash.exe docker run -u Administrator --rm -i -e PATH=/usr/bin msys2-x86_64 C:/msys64/usr/bin/bash.exe

# Start MYS tools:

docker run --name oidcbuilder -i -u Administrator -e HOME=/home/build -v c:/users/administrator/home-build:c:
/msys64/home/build msys2 c:/msys64/msys2_shell.cmd -defterm -no-start -msys2

# Update the packman-keys

mkdir pacman-keys cd pacman-keys curl -O http://repo.msys2.org/msys/x86_64/msys2-keyring-1~20211228-1-any.pkg.tar.zst
pacman --noconfirm -U msys2-keyring-1~20211228-1-any.pkg.tar.zst pacman --noconfirm -U --config <(echo) msys2-keyring-1~
20211228-1-any.pkg.tar.zst

# restart container

docker start oidcbuilder docker exec -i oidcbuilder C:/msys64/msys2_shell.cmd -defterm -no-start -msys2

# manually upgrade pacman

pacman -Syu

# install tools

pacman -Sy --noconfirm \
git \
mingw-w64-x86_64-libsecret \
mingw-w64-x86_64-libsodium \
mingw-w64-x86_64-libmicrohttpd \
mingw-w64-x86_64-qrencode \
mingw-w64-x86_64-check \
mingw-w64-i686-libsodium \
mingw-w64-i686-gcc git \
gcc \
libargp-devel \
libcurl-devel \
make \
pkgconf

# for putty:

pacman -Sy --noconfirm \
autotools \
automake \
mingw-w64-i686-curl \
mingw-w64-i686-expat \
mingw-w64-i686-gcc \
mingw-w64-i686-gcc-libs \
mingw-w64-i686-gettext \
mingw-w64-i686-gmp \
mingw-w64-i686-headers-git \
mingw-w64-i686-jansson \
mingw-w64-i686-jemalloc \
mingw-w64-i686-libffi \
mingw-w64-i686-libidn2 \
mingw-w64-i686-libpsl \
mingw-w64-i686-libtasn1 \
mingw-w64-i686-libunistring \
mingw-w64-i686-libwinpthread-git \
mingw-w64-i686-libxml2 \
mingw-w64-i686-nghttp2 \
mingw-w64-i686-openssl \
mingw-w64-i686-p11-kit \
mingw-w64-i686-nghttp2 \
mingw-w64-i686-openssl \
mingw-w64-i686-p11-kit \
mingw-w64-i686-xz \
mingw-w64-i686-zlib

# Notfound:

    mingw-w64-x86_64-i686-gcc \

# You now have a working docker container. Use it with:

docker start oidcbuilder docker exec -i oidcbuilder C:/msys64/msys2_shell.cmd -defterm -no-start -msys2 docker exec -i
oidcbuilder C:/msys64/msys2_shell.cmd -defterm -no-start -mingw64 docker exec -i oidcbuilder C:/msys64/msys2_shell.cmd
-defterm -no-start -mingw32

