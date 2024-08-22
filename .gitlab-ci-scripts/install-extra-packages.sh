#!/bin/bash
case ${DISTRO} in
    fedora)
        case ${RELEASE} in
            38|39)
                yum -y clean all
                yum -y install clibs-list-devel
                ;;
        esac
        ;;
    centos)
            yum -y clean all
            yum -y install clibs-list-devel
        ;;
    almalinux)
        case ${RELEASE} in
            "8.7")
                yum -y clean all
                dnf -y upgrade almalinux-release --nogpgcheck
                yum -y install clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel\
                    gstreamer1 gstreamer1-plugins-base

            ;;
            9)
                yum -y clean all
                dnf -y upgrade almalinux-release --nogpgcheck
                yum -y install clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel\
                    gstreamer1 gstreamer1-plugins-base
            ;;
        esac
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
