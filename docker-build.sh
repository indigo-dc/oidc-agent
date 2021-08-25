#!/bin/bash

### Build using:

#DIST=ubuntu_bionic ; docker run -it --rm -v `dirname $PWD`:/home/build $DIST /home/build/`basename $PWD`/build.sh `basename $PWD` $DIST

## ASSUMPTION: /home/build/$PACKAGE holds the sources for the package to be built
## ASSUMPTION: /home/build is on the host system.

BASE="/home/build"
PACKAGE=$1
DIST=$2
OUTPUT="$BASE/results"

echo "PACKAGE: $PACKAGE"
echo "DIST: $DIST"
echo "OUTPUT: $OUTPUT"

test -z $DIST && {
    echo "Must specify DIST as 2nd parameter"
    exit
}

common_prepare_dirs() {
    mkdir -p /tmp/build
    mkdir -p $OUTPUT/$DIST
    cp -af $BASE/$PACKAGE /tmp/build
    cd /tmp/build/$PACKAGE 
}
common_fix_output_permissions() {
    UP_UID=`stat -c '%u' $BASE`
    UP_GID=`stat -c '%g' $BASE`
    chown $UP_UID:$UP_GID $OUTPUT
    chown -R $UP_UID:$UP_GID $OUTPUT/$DIST
}
debian_install_dependencies() {
    apt-get update
    apt-get -y install libsodium-dev help2man libseccomp-dev \
        libmicrohttpd-dev check pkg-config libsecret-1-dev libcjson-dev \
        libcurl4-openssl-dev
}

ubuntu_bionic_install_dependencies() {
    #echo " deb http://security.ubuntu.com/ubuntu/ focal-security main restricted" >> /etc/apt/sources.list
    #apt-get update
    apt-get install -y debhelper/bionic
}
    
bionic_build_package() {
    make bionic-debsource && \
    dpkg-buildpackage -uc -us
}
buster_build_package() {
    make buster-debsource && \
    dpkg-buildpackage -uc -us
}
focal_build_package() {
    make focal-debsource && \
    dpkg-buildpackage -uc -us
}
debian_build_package() {
    make debsource && \
    dpkg-buildpackage -uc -us
}

debian_copy_output() {
    echo "Moving output:"
    ls -l ..
    mv ../${PACKAGE}[_-]* $OUTPUT/$DIST
    #mv ../${PACKAGE}-dbgsym_* $OUTPUT/$DIST 2>/dev/null
}

centos_install_dependencies () {
    yum -y install libcurl-devel pam-devel 
}
opensuse15_install_dependencies() {
    zypper -n install libcurl-devel pam-devel 
}
rpm_build_package() {
    cd /tmp/build/$PACKAGE
    make srctar
    make rpms
}
rpm_copy_output() {
    ls -l rpm/rpmbuild/RPMS/*/*
    ls -l rpm/rpmbuild/SRPMS/
    echo "-----"
    mv rpm/rpmbuild/RPMS/x86_64/${PACKAGE}*rpm $OUTPUT/$DIST
    mv rpm/rpmbuild/SRPMS/*rpm $OUTPUT/$DIST
}
    
###########################################################################
common_prepare_dirs

case "$DIST" in
    debian_bullseye|debian_bookworm)
        debian_install_dependencies
        debian_build_package
        debian_copy_output
    ;;
    debian_buster)
        debian_install_dependencies
        buster_build_package
        debian_copy_output
    ;;
    ubuntu_bionic)
        debian_install_dependencies
        ubuntu_bionic_install_dependencies
        bionic_build_package
        debian_copy_output
    ;;
    ubuntu_focal)
        debian_install_dependencies
        ubuntu_bionic_install_dependencies
        focal_build_package
        debian_copy_output
    ;;
    centos8|centos7)
        centos_install_dependencies
        rpm_build_package
        rpm_copy_output
    ;;
    opensuse15*|opensuse_tumbleweed|sle*)
        opensuse15_install_dependencies
        rpm_build_package
        rpm_copy_output
    ;;
esac

common_fix_output_permissions
