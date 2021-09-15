#!/bin/bash

### Build using:

# DIST=ubuntu_bionic
# IMAGE=build-oidc-agent-ubuntu:bionic
# DIST=fedora_34
# IMAGE=build-oidc-agent-fedora:34
# docker run -it --rm -v `dirname $PWD`:/home/build $IMAGE /home/build/`basename $PWD`/docker/docker-build.sh `basename $PWD` $DIST

## ASSUMPTION: /home/build/$PACKAGE_DIR holds the sources for the package to be built
## ASSUMPTION: /home/build is on the host system.
## ASSUMPTION: /home/build/results is on the host system.

BASE="/home/build"
PACKAGE_DIR=$1
DIST=$2
OUTPUT="$BASE/results"

echo "===================================================================="
echo "=========docker-build.sh============================================"
echo "PACKAGE_DIR: $PACKAGE_DIR"
echo "DIST: $DIST"
echo "OUTPUT: $OUTPUT"

test -z $DIST && {
    echo "Must specify DIST as 2nd parameter"
    exit
}

common_prepare_dirs() {
    mkdir -p /tmp/build
    mkdir -p $OUTPUT/$DIST
    cp -af $BASE/$PACKAGE_DIR /tmp/build
    cd /tmp/build/$PACKAGE_DIR 
}
common_fix_output_permissions() {
    UP_UID=`stat -c '%u' $BASE`
    UP_GID=`stat -c '%g' $BASE`
    chown $UP_UID:$UP_GID $OUTPUT
    chown -R $UP_UID:$UP_GID $OUTPUT/$DIST
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
    mv ../${PACKAGE_DIR}[_-]* $OUTPUT/$DIST
    mv ../lib* $OUTPUT/$DIST
}

rpm_unfuck_source_tarball_location() {
	cat rpm/oidc-agent.spec \
        | grep -v ^Source \
        > rpm/oidc-agent.spec.bckp
    VERSION=`head debian/changelog -n 1|cut -d \( -f 2|cut -d \) -f 1|cut -d \- -f 1`
    RELEASE=`head debian/changelog -n 1|cut -d \( -f 2|cut -d \) -f 1|cut -d \- -f 2`
    sed "s/#DO_NOT_REPLACE_THIS_LINE/Source0: oidc-agent-${VERSION}.tar.gz/" -i rpm/oidc-agent.spec.bckp
    rm -f rpm/oidc-agent.spec
    mv rpm/oidc-agent.spec.bckp rpm/oidc-agent.spec
}
rpm_build_package() {
    cd /tmp/build/$PACKAGE_DIR
    make rpmsource
    make rpms
}
rpm_copy_output() {
    ls -l rpm/rpmbuild/RPMS/*/*
    ls -l rpm/rpmbuild/SRPMS/
    echo "-----"
    mv rpm/rpmbuild/RPMS/*/*rpm $OUTPUT/$DIST
    mv rpm/rpmbuild/SRPMS/*rpm $OUTPUT/$DIST
}
    
###########################################################################
common_prepare_dirs

case "$DIST" in
    debian_bullseye|debian_bookworm)
        debian_build_package
        debian_copy_output
    ;;
    debian_buster)
        buster_build_package
        debian_copy_output
    ;;
    ubuntu_bionic)
        bionic_build_package
        debian_copy_output
    ;;
    ubuntu_focal|ubuntu_hirsute|ubuntu_impish)
        focal_build_package
        debian_copy_output
    ;;
    centos_8|centos_7|fedora*)
        rpm_unfuck_source_tarball_location
        rpm_build_package
        rpm_copy_output
    ;;
    opensuse_15*|opensuse_tumbleweed|sle*)
        rpm_unfuck_source_tarball_location
        rpm_build_package
        rpm_copy_output
    ;;
esac

common_fix_output_permissions
