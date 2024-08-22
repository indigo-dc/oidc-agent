#!/bin/bash
case ${DISTRO} in
    fedora)
            yum -y clean all
            yum -y install clibs-list-devel
        ;;
    centos)
            yum -y clean all
            yum -y install clibs-list-devel
        ;;
    almalinux)
            yum -y clean all
            dnf -y upgrade almalinux-release --nogpgcheck
            yum -y install clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel\
                gstreamer1 gstreamer1-plugins-base
        ;;
    rockylinux)
            yum -y clean all
            yum -y install cjson-devel clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel\
                gstreamer1 gstreamer1-plugins-base
        ;;
    opensuse)
                zypper -n install cJSON-devel systemd-rpm-macros webkit2gtk3-devel
        ;;
esac
