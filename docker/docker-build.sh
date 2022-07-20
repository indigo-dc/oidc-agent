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
ACTION=$3
OUTPUT="$BASE/results"

echo "===================================================================="
echo "=========docker-build.sh============================================"
echo "export BASE=$BASE"
echo "export PACKAGE_DIR=$PACKAGE_DIR"
echo "export DIST=$DIST"
echo "export ACTION=$ACTION"
echo "export OUTPUT=$OUTPUT"

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

get_debian_files_from_salsa() {
    git clone -b devel http://salsa.debian.org/debian/oidc-agent.git delme
    test -e debian ||           mv delme/debian .
    test -e docker/debian.mk || mv delme/docker/debian.mk docker/
    rm -rf delme
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

rpm_centos7_patch() {
    make centos7_patch
}
rpm_build_package() {
    cd /tmp/build/$PACKAGE_DIR
    make distclean
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

[ "x${ACTION}" == "xtest" ] && {
    case "$DIST" in
        debian_*|ubuntu_*)
            echo -e "\n\ninstalling oidc-agent-cli and liboidc-agent4"
            apt install -y \
                $OUTPUT/$DIST/oidc-agent-cli_*deb \
                $OUTPUT/$DIST/liboidc-agent4_*deb || exit 1
            echo -e "\n\ninstalling oidc-agent-desktop"
            apt install -y \
                $OUTPUT/$DIST/oidc-agent-desktop_4*deb || exit 2
            echo -e "\n\ninstalling liboidc-agent-dev"
            apt install -y \
                $OUTPUT/$DIST/liboidc-agent-dev_4*deb || exit 3
        ;;
    fedora_*|centos_*|rockylinux*)
            echo -e "\n\ninstalling oidc-agent-cli and liboidc-agent4"
            yum install -y \
                $OUTPUT/$DIST/oidc-agent-cli-4*rpm \
                $OUTPUT/$DIST/liboidc-agent4-4*rpm || exit 1
            echo -e "\n\ninstalling oidc-agent-desktop"
            yum install -y \
                $OUTPUT/$DIST/oidc-agent-desktop-4*rpm || exit 2
            echo -e "\n\ninstalling liboidc-agent-dev"
            yum install -y \
                $OUTPUT/$DIST/liboidc-agent-devel-4*rpm || exit 3
        ;;
        opensuse_*)
            echo -e "\n\ninstalling oidc-agent-cli and liboidc-agent4"
            zypper -n --no-gpg-checks install \
                $OUTPUT/$DIST/oidc-agent-cli-4*rpm \
                $OUTPUT/$DIST/liboidc-agent4-4*rpm || exit 1
            #echo -e "\n\ninstalling oidc-agent-desktop"
            #zypper -n --no-gpg-checks install \
            #    $OUTPUT/$DIST/oidc-agent-desktop-4*rpm || exit 2
            echo -e "\n\ninstalling liboidc-agent-dev"
            zypper -n --no-gpg-checks install \
                $OUTPUT/$DIST/liboidc-agent-devel-4*rpm || exit 3
        ;;
    esac
    
}

[ "x${ACTION}" == "xtest" ] || {

    case "$DIST" in
        debian_bullseye|debian_bookworm)
            get_debian_files_from_salsa
            debian_build_package
            debian_copy_output
        ;;
        debian_buster)
            get_debian_files_from_salsa
            buster_build_package
            debian_copy_output
        ;;
        ubuntu_*)
            get_debian_files_from_salsa
            focal_build_package
            debian_copy_output
        ;;
        centos_7)
            rpm_centos7_patch
            rpm_build_package
            rpm_copy_output
        ;;
        centos_8|centos_7|fedora*|rockylinux*)
            rpm_build_package
            rpm_copy_output
        ;;
        opensuse_15*|opensuse_tumbleweed|sle*)
            rpm_build_package
            rpm_copy_output
        ;;
    esac

    common_fix_output_permissions
}
