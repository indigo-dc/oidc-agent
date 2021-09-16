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
# Build windows api library
#
# Start MSYS MinGW 32-bit (mingw32.exe)

pacman -Sy --noconfirm mingw-w64-i686-libsodium \
    mingw-w64-i686-gcc

#Restart MSYS MinGW 32-bit (mingw32.exe)

make clean
make install_lib_windows-lib

#The static oidc-windows library will be installed for mingw32 and can be used
#for linking with other sources under mingw32.

#######################################
# putty mingw-w64 on linux:
apt-get install mingw-w64
wget https://the.earth.li/~sgtatham/putty/latest/putty-0.76.tar.gz
tar xzf putty-0.76.tar.gz
cd putty-0.76
./mkauto.sh
