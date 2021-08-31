# vim: ft=make
PKG_NAME  = oidc-agent
PKG_NAME_UPSTREAM = oidc-agent

SPECFILE := rpm/${PKG_NAME}.spec
RPM_VERSION := $(shell grep ^Version ${SPECFILE} | cut -d : -f 2 | sed s/\ //g)

DEBIAN_VERSION := $(shell head debian/changelog  -n 1 | cut -d \( -f 2 | cut -d \) -f 1 | cut -d \- -f 1)
VERSION := $(DEBIAN_VERSION)

# Parallel builds:
MAKEFLAGS += -j9

#BASEDIR = $(PWD)
#BASENAME := $(notdir $(PWD))
DOCKER_BASE=`dirname ${PWD}`
PACKAGE=`basename ${PWD}`
#SRC_TAR:=$(PKG_NAME).tar.gz
DOCKER_RUN_PARAMS=--tty -it --rm

SHELL=bash


info:
	@echo "DESTDIR:         >>$(DESTDIR)<<"
	@echo "INSTALLDIRS:     >>$(INSTALLDIRS)<<"
	@echo "VERSION:         >>$(VERSION)<<"
	@echo "RPM_VERSION:     >>$(RPM_VERSION)<<"
	@echo "DEBIAN_VERSION:  >>$(DEBIAN_VERSION)<<"
	@echo "PKG_NAME:        >>$(PKG_NAME)<<"
	@echo "PKG_NAME_UPSTREAM>>$(PKG_NAME_UPSTREAM)<<"
	@echo "DOCKER_BASE      >>$(DOCKER_BASE)<<"
	@echo "PACKAGE          >>$(PACKAGE)<<"

### Dockers
dockerised_most_packages: dockerised_deb_debian_bullseye\
	dockerised_deb_debian_bookworm\
	dockerised_deb_ubuntu_focal\
	dockerised_rpm_centos8\
	dockerised_rpm_opensuse_tumbleweed

dockerised_all_packages: dockerised_deb_debian_buster\
	dockerised_deb_debian_bullseye\
	dockerised_deb_debian_bookworm\
	dockerised_deb_ubuntu_bionic\
	dockerised_deb_ubuntu_focal\
	dockerised_rpm_centos7\
	dockerised_rpm_centos8\
	dockerised_rpm_opensuse15.2\
	dockerised_rpm_opensuse15.3\
	dockerised_rpm_opensuse_tumbleweed

.PHONY: docker_images
docker_images: docker_centos8\
	docker_centos7\
	docker_debian_bullseye\
	docker_debian_buster\
	docker_ubuntu_bionic\
	docker_ubuntu_focal\
	docker_opensuse15.2\
	docker_opensuse15.3\
	docker_opensuse_tumbleweed

# Create the dockers. They are actually rebuilt, even if a dependency is # _removed_
.PHONY: docker_debian_buster
docker_debian_buster:
	@echo -e "\ndebian_buster"
	@echo -e "FROM debian:buster\n"\
	"RUN apt-get update && apt-get -y upgrade \n"\
	"RUN apt-get -y install build-essential dh-make quilt devscripts\n" \
	"RUN apt-get -y install libsodium-dev help2man libseccomp-dev " \
		"libmicrohttpd-dev check pkg-config libsecret-1-dev libcjson-dev "\
		"libcurl4-openssl-dev " \
	| docker build --tag debian_buster -f - .
.PHONY: docker_debian_bullseye
docker_debian_bullseye:
	@echo -e "\ndebian_bullseye"
	@echo -e "FROM debian:bullseye\n" \
	"RUN apt-get update && apt-get -y upgrade \n" \
	"RUN apt-get -y install build-essential dh-make quilt devscripts\n" \
	"RUN apt-get -y install libsodium-dev help2man libseccomp-dev "\
		"libmicrohttpd-dev check pkg-config libsecret-1-dev libcjson-dev "\
		"libcurl4-openssl-dev " \
	| docker build --tag debian_bullseye -f - .
.PHONY: docker_debian_bookworm
docker_debian_bookworm:
	@echo -e "\ndebian_bookworm"
	@echo -e "FROM debian:bookworm\n"\
	"RUN apt-get update && apt-get -y upgrade \n"\
	"RUN apt-get -y install build-essential dh-make quilt devscripts \n"\
	"RUN apt-get -y install libsodium-dev help2man libseccomp-dev "\
		"libmicrohttpd-dev check pkg-config libsecret-1-dev libcjson-dev "\
		"libcurl4-openssl-dev " \
	| docker build --tag debian_bookworm -f - .
.PHONY: docker_ubuntu_bionic
docker_ubuntu_bionic:
	@echo -e "\nubuntu_bionic"
	@echo -e "FROM ubuntu:bionic\n"\
	"RUN apt-get update && apt-get -y upgrade \n"\
	"RUN apt-get -y install build-essential dh-make quilt devscripts \n" \
	"RUN apt-get -y install libsodium-dev help2man libseccomp-dev "\
		"libmicrohttpd-dev check pkg-config libsecret-1-dev "\
		"libcurl4-openssl-dev " \
	| docker build --tag ubuntu_bionic -f - .
.PHONY: docker_ubuntu_focal
docker_ubuntu_focal:
	@echo -e "\nubuntu_focal"
	@echo -e "FROM ubuntu:focal\n"\
	"ENV DEBIAN_FRONTEND=noninteractive\n"\
	"ENV  TZ=Europe/Berlin\n"\
	"RUN apt-get update && apt-get -y upgrade \n"\
	"RUN apt-get -y install build-essential dh-make quilt devscripts \n" \
	"RUN apt-get -y install libsodium-dev help2man libseccomp-dev "\
		"libmicrohttpd-dev check pkg-config libsecret-1-dev libcjson-dev "\
		"libcurl4-openssl-dev " \
	| docker build --tag ubuntu_focal -f - .
.PHONY: docker_centos7
docker_centos7:
	@echo -e "\ncentos7"
	@echo -e "FROM centos:7\n"\
	"RUN yum -y install make rpm-build\n"\
	"RUN yum -y groups mark convert\n"\
	"RUN yum -y groupinstall \"Development tools\"\n" \
	| docker build --tag centos7 -f - .
.PHONY: docker_indigo_bcentos7
docker_indigo_bcentos7:
	@echo -e "\nindigo_bcentos7"
	@echo -e "FROM indigodatacloud/ci-images:bcentos7\n"\
	"RUN yum -y install make rpm-build\n"\
	"RUN yum -y groups mark convert\n"\
	"RUN yum -y groupinstall \"Development tools\"\n" \
	"RUN yum -y install libsodium-devel libsodium-static \
        help2man libseccomp-devel libsecret-devel libmicrohttpd-devel \
        libcurl-devel desktop-file-utils \n" \
	| docker build --tag bcentos7 -f - .
.PHONY: docker_centos8
docker_centos8:
	@echo -e "\ncentos8"
	@echo -e "FROM centos:8\n"\
	"RUN yum install -y make rpm-build\n" \
	"RUN dnf -y group install \"Development Tools\"\n" \
	"RUN yum install -y help2man libmicrohttpd-devel libseccomp-devel libsecret-devel libsodium-devel libsodium-static \n" \
	| docker build --tag centos8 -f -  .
.PHONY: docker_opensuse15.2
docker_opensuse15.2:
	@echo -e "\nopensuse-15.2"
	@echo -e "FROM registry.opensuse.org/opensuse/leap:15.2\n"\
	"RUN zypper -n install make rpm-build\n" \
	"RUN zypper -n install -t pattern devel_C_C++" \
	| docker build --tag opensuse15.2 -f -  .
.PHONY: docker_opensuse15.3
docker_opensuse15.3:
	@echo -e "\nopensuse-15.3"
	@echo -e "FROM registry.opensuse.org/opensuse/leap:15.3\n"\
	"RUN zypper -n install make rpm-build\n" \
	"RUN zypper -n install -t pattern devel_C_C++" \
	| docker build --tag opensuse15.3 -f -  .
.PHONY: docker_opensuse_tumbleweed
docker_opensuse_tumbleweed:
	@echo -e "\nopensuse_tumbleweed"
	@echo -e "FROM registry.opensuse.org/opensuse/tumbleweed:latest\n"\
	"RUN zypper -n install make rpm-build\n" \
	"RUN zypper -n install -t pattern devel_C_C++" \
	| docker build --tag opensuse_tumbleweed -f -  .
.PHONY: docker_sle15
docker_sle15:
	@echo -e "\nsle15"
	@echo -e "FROM registry.suse.com/suse/sle15\n"\
	"RUN zypper -n install make rpm-build\n" \
	"RUN zypper -n install -t pattern devel_C_C++" \
	| docker build --tag sle15 -f -  .

.PHONY: docker_clean
docker_clean:
	docker image rm sle15 || true
	docker image rm	opensuse_tumbleweed || true
	docker image rm opensuse15.2 || true
	docker image rm	opensuse15.3 || true
	docker image rm centos8 || true
	docker image rm	centos7 || true
	docker image rm	bcentos7 || true
	docker image rm ubuntu_bionic || true
	docker image rm	ubuntu_focal || true
	docker image rm debian_buster || true
	docker image rm	debian_bullseye || true

.PHONY: dockerised_deb_debian_buster
dockerised_deb_debian_buster: docker_debian_buster
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build debian_buster \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} debian_buster ${PKG_NAME}

.PHONY: dockerised_deb_debian_bullseye
dockerised_deb_debian_bullseye: docker_debian_bullseye
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build debian_bullseye \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} debian_bullseye ${PKG_NAME}

.PHONY: dockerised_deb_debian_bookworm
dockerised_deb_debian_bookworm: docker_debian_bookworm
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build debian_bookworm \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} debian_bookworm ${PKG_NAME}

.PHONY: dockerised_deb_ubuntu_bionic
dockerised_deb_ubuntu_bionic: docker_ubuntu_bionic
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build ubuntu_bionic \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} ubuntu_bionic ${PKG_NAME} 

.PHONY: dockerised_deb_ubuntu_focal
dockerised_deb_ubuntu_focal: docker_ubuntu_focal
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build ubuntu_focal \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} ubuntu_focal ${PKG_NAME}

.PHONY: dockerised_rpm_centos7
dockerised_rpm_centos7: docker_centos7
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build centos7 \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} centos7 ${PKG_NAME}

.PHONY: dockerised_rpm_indigo_bcentos7
dockerised_rpm_indigo_bcentos7: docker_indigo_bcentos7
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build bcentos7 \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} bcentos7 ${PKG_NAME} 

.PHONY: dockerised_rpm_centos8
dockerised_rpm_centos8: docker_centos8
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build centos8 \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} centos8 ${PKG_NAME}

.PHONY: dockerised_rpm_opensuse15.2
dockerised_rpm_opensuse15.2: docker_opensuse15.2
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build opensuse15.2 \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} opensuse15.2 ${PKG_NAME}

.PHONY: dockerised_rpm_opensuse15.3
dockerised_rpm_opensuse15.3: docker_opensuse15.3
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build opensuse15.3 \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} opensuse15.3 ${PKG_NAME}

.PHONY: dockerised_rpm_opensuse_tumbleweed
dockerised_rpm_opensuse_tumbleweed: docker_opensuse_tumbleweed
	@echo "Writing build log to $@.log"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build opensuse_tumbleweed \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} opensuse_tumbleweed ${PKG_NAME}
