#!/bin/bash
case ${DISTRO} in
    debian|ubuntu)
        case ${RELEASE} in
            buster)
                # make buster-debsource
                ;;
        esac
        ;;
    fedora)
        case ${RELEASE} in
            38|39)
                yum clean all
                yum -y install clibs-list-devel
                ;;
        esac
        ;;
    centos)
        case ${RELEASE} in
            7)
                yum clean all
                yum -y install clibs-list-devel
            ;;
        esac
        ;;
    almalinux)
        case ${RELEASE} in
            8.7)
                yum clean all
                dnf upgrade almalinux-release --nogpgcheck
                yum -y install clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel
            9)
                yum clean all
                dnf upgrade almalinux-release --nogpgcheck
                yum -y install clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel
            ;;
        esac
        ;;
    rockylinux)
        case ${RELEASE} in
            9)
                yum clean all
                yum -y install cjson-devel clibs-list-devel curl-devel systemd-rpm-macros webkit2gtk3-devel
            ;;
        esac
        ;;
    opensuse)
        case ${RELEASE} in
            15.4|15.5)
                zypper -n install cJSON-devel systemd-rpm-macros webkit2gtk3-devel
            ;;
        esac
        ;;
esac
