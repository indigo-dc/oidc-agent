#!/bin/bash
# This is not quite a shell script, because msys2 will exit after first
# running the # first command
##########################################
# Preparation
#
# 1. Install MSYS2 (https://www.msys2.org/)
#
# 2. Start 'MSYS2 MSYS' either from start menu or msys.exe from 'msys64'
# installation folder
#
# 3. Update packages using 'pacman -Syu' (this will terminate the msys
# session, which is fine. Just start a new one afterwards)
pacman -Syu --noconfirm
pacman -Sy --noconfirm git

# 4. Install build dependencies
pacman -Sy --noconfirm \
     mingw-w64-x86_64-libsecret \
     mingw-w64-x86_64-libmicrohttpd \
     mingw-w64-x86_64-libsodium \
     git \
     gcc \
     libargp-devel \
     libcurl-devel \
     make \
     pkgconf

cp /mingw64/lib/pkgconfig/glib-2.0.pc               /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/gio-2.0.pc                /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/libsecret-1.pc            /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/libgcrypt.pc              /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/gobject-2.0.pc            /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/libpcre.pc                /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/libffi.pc                 /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/gmodule-no-export-2.0.pc  /usr/lib/pkgconfig
cp /mingw64/lib/pkgconfig/gpg-error.pc              /usr/lib/pkgconfig

##########################################
# Build oidc-agent
#
# Clone the oidc-agent repository to {msys64}/home/{username}/
git clone https://github.com/indigo-dc/oidc-agent.git
cd oidc-agent
# and switch to #the 'windows' branch
git checkout windows

make

##########################################
# Install oidc-agent into oidc-agent/bin folder
mkdir bin/tmp
mkdir bin/config
cp config/issuer.config bin/config
cp config/pubclients.config bin/config
cp /mingw64/ssl/certs/ca-bundle.crt bin/config
cp /mingw64/bin/libffi-7.dll bin
cp /mingw64/bin/libgcc_s_seh-1.dll bin
cp /mingw64/bin/libgio-2.0-0.dll bin
cp /mingw64/bin/libglib-2.0-0.dll bin
cp /mingw64/bin/libgmodule-2.0-0.dll bin
cp /mingw64/bin/libgmp-10.dll bin
cp /mingw64/bin/libgnutls-30.dll bin
cp /mingw64/bin/libgobject-2.0-0.dll bin
cp /mingw64/bin/libgpg-error-0.dll bin
cp /mingw64/bin/libhogweed-6.dll bin
cp /mingw64/bin/libiconv-2.dll bin
cp /mingw64/bin/libidn2-0.dll bin
cp /mingw64/bin/libintl-8.dll bin
cp /mingw64/bin/libmicrohttpd-12.dll bin
cp /mingw64/bin/libnettle-8.dll bin
cp /mingw64/bin/libp11-kit-0.dll bin
cp /mingw64/bin/libpcre-1.dll bin
cp /mingw64/bin/libsecret-1-0.dll bin
cp /mingw64/bin/libsodium-23.dll bin
cp /mingw64/bin/libtasn1-6.dll bin
cp /mingw64/bin/libunistring-2.dll bin
cp /mingw64/bin/libwinpthread-1.dll bin
cp /mingw64/bin/libgcrypt-20.dll bin
cp /mingw64/bin/zlib1.dll bin
cp /usr/bin/msys-2.0.dll bin
cp /usr/bin/msys-argp-0.dll bin
cp /usr/bin/msys-asn1-8.dll bin
cp /usr/bin/msys-brotlicommon-1.dll bin
cp /usr/bin/msys-brotlidec-1.dll bin
cp /usr/bin/msys-com_err-1.dll bin
cp /usr/bin/msys-crypt-0.dll bin
cp /usr/bin/msys-crypto-1.1.dll bin
cp /usr/bin/msys-curl-4.dll bin
cp /usr/bin/msys-gcc_s-seh-1.dll bin
cp /usr/bin/msys-glib-2.0-0.dll bin
cp /usr/bin/msys-gssapi-3.dll bin
cp /usr/bin/msys-hcrypto-4.dll bin
cp /usr/bin/msys-heimbase-1.dll bin
cp /usr/bin/msys-heimntlm-0.dll bin
cp /usr/bin/msys-hx509-5.dll bin
cp /usr/bin/msys-iconv-2.dll bin
cp /usr/bin/msys-idn2-0.dll bin
cp /usr/bin/msys-intl-8.dll bin
cp /usr/bin/msys-krb5-26.dll bin
cp /usr/bin/msys-nghttp2-14.dll bin
cp /usr/bin/msys-pcre2-8-0.dll bin
cp /usr/bin/msys-psl-5.dll bin
cp /usr/bin/msys-roken-18.dll bin
cp /usr/bin/msys-sqlite3-0.dll bin
cp /usr/bin/msys-ssh2-1.dll bin
cp /usr/bin/msys-ssl-1.1.dll bin
cp /usr/bin/msys-unistring-2.dll bin
cp /usr/bin/msys-wind-0.dll bin
cp /usr/bin/msys-z.dll bin
cp /usr/bin/msys-zstd-1.dll bin

###########################################
## Build windows api library
##
## Start MSYS MinGW 32-bit (mingw32.exe)
#
#pacman -Sy --noconfirm mingw-w64-i686-libsodium \
#    mingw-w64-i686-gcc
#
##Restart MSYS MinGW 32-bit (mingw32.exe)
#
#make clean
#make install_lib_windows-lib
#
##The static oidc-windows library will be installed for mingw32 and can be used
##for linking with other sources under mingw32.
#
########################################
## putty mingw-w64 on linux:
#apt-get install mingw-w64
#wget https://the.earth.li/~sgtatham/putty/latest/putty-0.76.tar.gz
#tar xzf putty-0.76.tar.gz
#cd putty-0.76
#./mkauto.sh
