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

DOCKER_GEN_FROM_IMAGE			= "FROM `echo $@ | sed s/docker_//| cut -d: -f 1`:`echo $@ | sed s/docker_//| cut -d: -f 2`"

DOCKER_YUM_DISABLE_FASTESTMIRROR= "RUN sed s/enabled=1/enabled=0/ -i /etc/yum/pluginconf.d/fastestmirror.conf"
DOCKER_YUM_BUILD_ESSENTIALS     = "RUN yum -y install make rpm-build"
DOCKER_YUM_GROUPS_BASE			= "RUN yum -y groups mark convert"
DOCKER_YUM_GROUPS_DEVELTOOLS    = "RUN yum -y groupinstall \"Development tools\""
DOCKER_YUM_EPEL_RELEASE         = "RUN yum -y install epel-release"
DOCKER_YUM_REMI_RELEASE         = "RUN dnf -y install https://rpms.remirepo.net/enterprise/remi-release-8.rpm"
DOCKER_YUM_ENABLE_POWERTOOLS	= "RUN dnf config-manager --set-enabled powertools"

DOCKER_ZYP_BUILD_ESSENTIALS 	= "RUN zypper -n install make rpm-build"
DOCKER_ZYP_GROUP_DEVEL			= "RUN zypper -n install -t pattern devel_C_C++"

DOCKER_COPY_DEPENDENCIES	 	= "COPY docker/`echo $@ | sed s/docker_//`.dependencies /tmp/dependencies.cache"
#DOCKER_COPY_DEPENDENCIES	 	= "COPY docker/`echo $@ | sed s/docker_// | sed s/s/XXX/g`.dependencies /tmp/dependencies.cache"
DOCKER_COPY_DEPENDENCIES	 	= "COPY docker/`echo $@ | sed s/docker_// \
								  | sed s%registry.opensuse.org/opensuse%opensuse%g \
								  | sed s%/%_%g`.dependencies /tmp/dependencies.cache"
DOCKER_YUM_INST_DEPENDENCIES	= "RUN xargs yum -y    install < /tmp/dependencies.cache"
DOCKER_ZYP_INST_DEPENDENCIES	= "RUN xargs zypper -n install < /tmp/dependencies.cache"
DOCKER_TAG						= "build-`echo $@ | sed s/docker_//\
								  | sed s%registry.opensuse.org/opensuse%opensuse%g \
								  | sed s%/%_%g`"


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
	dockerised_rpm_indigo_bcentos7\
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
.PHONY: docker_debian\:buster
docker_debian_buster:
	@echo -e "\ndebian_buster"
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
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
docker_registry.access.redhat.com/ubi8/ubi\:8.1: 
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_YUM_EPEL_RELEASE)"\n" \
	$(DOCKER_YUM_REMI_RELEASE)"\n" \
	$(DOCKER_YUM_ENABLE_POWERTOOLS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - .
.PHONY: docker_centos\:centos8
docker_centos\:centos8: 
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_YUM_REMI_RELEASE)"\n" \
	$(DOCKER_YUM_ENABLE_POWERTOOLS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - .
.PHONY: docker_centos\:centos7
docker_centos\:centos7: 
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_DISABLE_FASTESTMIRROR)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_BASE)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_YUM_EPEL_RELEASE)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - .

docker_registry.opensuse.org/opensuse/leap\:15.2:
	@echo -e "\nopensuse-15.2"
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_ZYP_GROUP_DEVEL)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_ZYP_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f -  .
docker_registry.opensuse.org/opensuse/leap\:15.3:
	@echo -e "\nopensuse-15.3"
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_ZYP_GROUP_DEVEL)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_ZYP_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f -  .
docker_registry.opensuse.org/opensuse/tumbleweed\:latest:
	@echo -e "\nopensuse-tumbleweed-latest"
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_ZYP_GROUP_DEVEL)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_ZYP_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f -  .

#docker_registry.suse.com/suse/sle15\:latest:
#    @echo -e "\nsuse SLE 15"
#    @echo -e \
#    $(DOCKER_GEN_FROM_IMAGE)"\n" \
#    $(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
#    $(DOCKER_ZYP_GROUP_DEVEL)"\n" \
#    $(DOCKER_COPY_DEPENDENCIES)"\n" \
#    $(DOCKER_ZYP_INST_DEPENDENCIES) \
#    | docker build --tag $(DOCKER_TAG) -f -  .

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
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build debian_buster \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} debian_buster ${PKG_NAME}

.PHONY: dockerised_deb_debian_bullseye
dockerised_deb_debian_bullseye: docker_debian_bullseye
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build debian_bullseye \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} debian_bullseye ${PKG_NAME}

.PHONY: dockerised_deb_debian_bookworm
dockerised_deb_debian_bookworm: docker_debian_bookworm
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build debian_bookworm \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} debian_bookworm ${PKG_NAME}

.PHONY: dockerised_deb_ubuntu_bionic
dockerised_deb_ubuntu_bionic: docker_ubuntu_bionic
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build ubuntu_bionic \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} ubuntu_bionic ${PKG_NAME} 

.PHONY: dockerised_deb_ubuntu_focal
dockerised_deb_ubuntu_focal: docker_ubuntu_focal
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build ubuntu_focal \
		/home/build/${PACKAGE}/docker-build.sh ${PACKAGE} ubuntu_focal ${PKG_NAME}

.PHONY: dockerised_rpm_centos7
dockerised_rpm_centos7: docker_centos\:centos7
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-centos:centos7 \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} `echo $@ | sed s/dockerised_rpm_//` ${PKG_NAME}

.PHONY: dockerised_rpm_centos8
dockerised_rpm_centos8: docker_centos\:centos8
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-centos:centos8 \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} `echo $@ | sed s/dockerised_rpm_//` ${PKG_NAME}

.PHONY: dockerised_rpm_opensuse15.2
dockerised_rpm_opensuse15.2: docker_registry.opensuse.org/opensuse/leap\:15.2
	@echo docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-opensuse_leap:15.2 \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} opensuse15.2 ${PKG_NAME}
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-opensuse_leap:15.2 \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} opensuse15.2 ${PKG_NAME}

.PHONY: dockerised_rpm_opensuse15.3
dockerised_rpm_opensuse15.3: docker_registry.opensuse.org/opensuse/leap\:15.3
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-opensuse_leap:15.3 \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} opensuse15.3 ${PKG_NAME}

.PHONY: dockerised_rpm_opensuse_tumbleweed
dockerised_rpm_opensuse_tumbleweed: docker_registry.opensuse.org/opensuse/tumbleweed\:latest
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-opensuse_tumbleweed:latest \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} opensuse_tumbleweed ${PKG_NAME}

.PHONY: dockerised_rpm_redhat_starship_enterprise
dockerised_rpm_redhat_starship_enterprise: docker_registry.opensuse.org/opensuse/tumbleweed\:latest
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-opensuse_tumbleweed:latest \
		/home/build/${PACKAGE}/docker/docker-build.sh ${PACKAGE} opensuse_tumbleweed ${PKG_NAME}
