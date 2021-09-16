# vim: ft=make

# Parallel builds:
MAKEFLAGS += -j9

DOCKER_BASE=`dirname ${PWD}`
PACKAGE_DIR=`basename ${PWD}`
DOCKER_RUN_PARAMS=--tty -it --rm

SHELL=bash

DOCKER_GEN_FROM_IMAGE			= "FROM `echo $@ | sed s/docker_//| cut -d: -f 1`:`echo $@ | sed s/docker_//| cut -d: -f 2`"

DOCKER_APT_INIT                 = "ENV DEBIAN_FRONTEND noninteractive\nRUN apt update && apt -y upgrade"
DOCKER_APT_BUILD_ESSENTIALS     = "RUN apt install -y build-essential dh-make quilt devscripts"

DOCKER_YUM_DISABLE_FASTESTMIRROR= "RUN sed s/enabled=1/enabled=0/ -i /etc/yum/pluginconf.d/fastestmirror.conf"
DOCKER_YUM_BUILD_ESSENTIALS     = "RUN yum -y install make rpm-build"
DOCKER_YUM_GROUPS_BASE			= "RUN yum -y groups mark convert"
DOCKER_YUM_GROUPS_DEVELTOOLS    = "RUN yum -y groupinstall \"Development tools\""
DOCKER_YUM_EPEL_RELEASE         = "RUN yum -y install epel-release"
DOCKER_YUM_REMI_RELEASE         = "RUN dnf -y install https://rpms.remirepo.net/enterprise/remi-release-8.rpm"
DOCKER_YUM_ENABLE_POWERTOOLS	= "RUN dnf config-manager --set-enabled powertools"

DOCKER_ZYP_BUILD_ESSENTIALS 	= "RUN zypper -n install make rpm-build"
DOCKER_ZYP_GROUP_DEVEL			= "RUN zypper -n install -t pattern devel_C_C++"

DOCKER_COPY_DEPENDENCIES	 	= "COPY docker/`echo $@ | sed s/docker_// \
								  | sed s%registry.opensuse.org/opensuse%opensuse%g \
								  | sed s%/%_%g`.dependencies /tmp/dependencies.cache"
DOCKER_APT_INST_DEPENDENCIES	= "RUN xargs apt -y    install < /tmp/dependencies.cache"
DOCKER_YUM_INST_DEPENDENCIES	= "RUN xargs yum -y    install < /tmp/dependencies.cache"
DOCKER_ZYP_INST_DEPENDENCIES	= "RUN xargs zypper -n install < /tmp/dependencies.cache"

DOCKER_TAG						= "build-$(PACKAGE_DIR)-`echo $@ | sed s/docker_//\
								  | sed s%registry.opensuse.org/opensuse%opensuse%g \
								  | sed s%/%_%g`"
DOCKER_DIST 					= "`echo $@ | sed s/dockerised_rpm_//\
								  			| sed s/dockerised_deb_//\
								  			| sed s/dockerised_test_//`"
DOCKER_LOG						= "docker/log/`echo $@ | sed s/docker_//\
								  | sed s%registry.opensuse.org/opensuse/leap%opensuse%g \
								  | sed s%registry.opensuse.org/opensuse/tumbleweed:latest%opensuse_tumbleweed%g \
								  | sed s%/%_%g\
								  | sed s/:/_/`.log"
DOCKER_BUILD_LOG				= "docker/log/`echo $@ | sed s/dockerised_rpm_//\
												       | sed s/dockerised_deb_//\
													   | sed s/dockerised_test_//`.log"
DOCKER_CONTAINER				= "`echo $@ | sed s/dockerised_test_//\
								  			| sed s/dockerised_deb_//\
											| sed s/dockerised_rpm_//\
											| sed s/_/:/\
											| sed s/opensuse:1/opensuse_leap:1/\
											| sed s/opensuse:tumbleweed/opensuse_tumbleweed:latest/`"



########################################## DOCKERS ##########################################
.PHONY: dockerised_latest_packages
dockerised_latest_packages: dockerised_deb_debian_bullseye\
	dockerised_deb_debian_bookworm\
	dockerised_deb_ubuntu_focal\
	dockerised_deb_ubuntu_hirsute\
	dockerised_rpm_centos_8\
	dockerised_rpm_opensuse_tumbleweed\
	dockerised_rpm_fedora35

.PHONY: dockerised_all_deb_packages
dockerised_all_deb_packages: dockerised_deb_debian_buster\
	dockerised_deb_debian_bullseye\
	dockerised_deb_debian_bookworm\
	dockerised_deb_ubuntu_bionic\
	dockerised_deb_ubuntu_focal\
	dockerised_deb_ubuntu_hirsute

.PHONY: dockerised_all_rpm_packages
dockerised_all_rpm_packages: dockerised_rpm_centos_7\
	dockerised_rpm_centos_8\
	dockerised_rpm_opensuse_15.2\
	dockerised_rpm_opensuse_15.3\
	dockerised_rpm_opensuse_tumbleweed\
	dockerised_rpm_fedora_34

.PHONY: dockerised_all_packages
dockerised_all_packages: dockerised_deb_debian_buster\
	dockerised_deb_debian_bullseye\
	dockerised_deb_debian_bookworm\
	dockerised_deb_ubuntu_bionic\
	dockerised_deb_ubuntu_focal\
	dockerised_deb_ubuntu_hirsute\
	dockerised_rpm_centos_7\
	dockerised_rpm_centos_8\
	dockerised_rpm_opensuse_15.2\
	dockerised_rpm_opensuse_15.3\
	dockerised_rpm_opensuse_tumbleweed\
	dockerised_rpm_fedora_34

.PHONY: dockerised_test_all
dockerised_test_all: dockerised_test_debian_buster\
	dockerised_test_debian_bullseye\
	dockerised_test_debian_bookworm\
	dockerised_test_ubuntu_bionic\
	dockerised_test_ubuntu_focal\
	dockerised_test_ubuntu_hirsute\
	dockerised_test_centos_7\
	dockerised_test_centos_8\
	dockerised_test_opensuse_15.2\
	dockerised_test_opensuse_15.3\
	dockerised_test_opensuse_tumbleweed\
	dockerised_test_fedora_34

.PHONY: dockerised_test_debs
dockerised_test_debs: dockerised_test_debian_buster\
	dockerised_test_debian_bullseye\
	dockerised_test_debian_bookworm\
	dockerised_test_ubuntu_bionic\
	dockerised_test_ubuntu_focal\
	dockerised_test_ubuntu_hirsute

.PHONY: dockerised_test_rpms
dockerised_test_rpms: dockerised_test_centos_7\
	dockerised_test_centos_8\
	dockerised_test_opensuse_15.2\
	dockerised_test_opensuse_15.3\
	dockerised_test_opensuse_tumbleweed\
	dockerised_test_fedora_34

.PHONY: docker_images
docker_images: docker_debian\:buster\
	docker_debian\:bullseye\
	docker_debian\:bookworm\
	docker_ubuntu\:bionic\
	docker_ubuntu\:focal\
	docker_ubuntu\:hirsute\
	docker_centos\:7\
	docker_centos\:8\
	docker_registry.opensuse.org/opensuse/leap\:15.2\
	docker_registry.opensuse.org/opensuse/leap\:15.3\
	docker_registry.opensuse.org/opensuse/tumbleweed\:latest\
	docker_fedora\:34

.PHONY: docker_clean
docker_clean:
	echo docker image rm build-$(PACKAGE_DIR)-debian:buster 		|| true
	@docker image rm build-$(PACKAGE_DIR)-debian:bullseye			|| true
	@docker image rm build-$(PACKAGE_DIR)-debian:bookworm			|| true
	@docker image rm build-$(PACKAGE_DIR)-ubuntu:bionic				|| true
	@docker image rm build-$(PACKAGE_DIR)-ubuntu:focal				|| true
	@docker image rm build-$(PACKAGE_DIR)-ubuntu:hirsute				|| true
	@docker image rm build-$(PACKAGE_DIR)-ubuntu:impish				|| true
	@docker image rm build-$(PACKAGE_DIR)-centos:7					|| true
	@docker image rm build-$(PACKAGE_DIR)-centos:8					|| true
	@docker image rm build-$(PACKAGE_DIR)-opensuse_leap:15.2			|| true
	@docker image rm build-$(PACKAGE_DIR)-opensuse_leap:15.3			|| true
	@docker image rm build-$(PACKAGE_DIR)-opensuse_tumbleweed:latest	|| true
	@docker image rm build-$(PACKAGE_DIR)-fedora:34					|| true
	@docker image rm build-$(PACKAGE_DIR)-fedora:35					|| true

########################################## DEBIAN ##########################################
.PHONY: dockerised_deb_buster
dockerised_deb_buster: dockerised_deb_debian_buster
.PHONY: dockerised_deb_debian_buster
dockerised_deb_debian_buster: docker_debian\:buster
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_debian\:buster
docker_debian\:buster:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_debian_buster:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_deb_debian_bullseye
dockerised_deb_debian_bullseye: docker_debian\:bullseye
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_debian\:bullseye
docker_debian\:bullseye:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_debian_bullseye:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_deb_debian_bookworm
dockerised_deb_debian_bookworm: docker_debian\:bookworm
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_debian\:bookworm
docker_debian\:bookworm:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_debian_bookworm:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

########################################## UBUNTU ##########################################
.PHONY: dockerised_deb_ubuntu_bionic
dockerised_deb_ubuntu_bionic: docker_ubuntu\:bionic
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-ubuntu:bionic \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_ubuntu\:bionic
docker_ubuntu\:bionic:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_ubuntu_bionic:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_deb_ubuntu_focal
dockerised_deb_ubuntu_focal: docker_ubuntu\:focal
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-ubuntu:focal \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_ubuntu\:focal
docker_ubuntu\:focal:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_ubuntu_focal:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-debian:buster \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_deb_ubuntu_hirsute
dockerised_deb_ubuntu_hirsute: docker_ubuntu\:hirsute
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-ubuntu:hirsute \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_ubuntu\:hirsute
docker_ubuntu\:hirsute:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_ubuntu_hirsute:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_deb_ubuntu_impish
dockerised_deb_ubuntu_impish: docker_ubuntu\:impish
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-ubuntu:impish \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_ubuntu\:impish
docker_ubuntu\:impish:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_APT_INIT)"\n" \
	$(DOCKER_APT_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_APT_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_ubuntu_impish:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}


########################################## CENTOS ##########################################
.PHONY: dockerised_rpm_centos_7
dockerised_rpm_centos_7: docker_centos\:7
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-centos:7 \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_centos\:7
docker_centos\:7: 
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_DISABLE_FASTESTMIRROR)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_BASE)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_YUM_EPEL_RELEASE)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_centos_7:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_rpm_centos_8
dockerised_rpm_centos_8: docker_centos\:8
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-centos:8 \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_centos\:8
docker_centos\:8: 
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_YUM_REMI_RELEASE)"\n" \
	$(DOCKER_YUM_ENABLE_POWERTOOLS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_centos_8:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

########################################## SUSE ##########################################
.PHONY: dockerised_rpm_opensuse_15.2
dockerised_rpm_opensuse_15.2: docker_registry.opensuse.org/opensuse/leap\:15.2
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_registry.opensuse.org/opensuse/leap\:15.2
docker_registry.opensuse.org/opensuse/leap\:15.2:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_ZYP_GROUP_DEVEL)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_ZYP_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f -  . >> ${DOCKER_LOG}
dockerised_test_opensuse_15.2:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_rpm_opensuse_15.3
dockerised_rpm_opensuse_15.3: docker_registry.opensuse.org/opensuse/leap\:15.3
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_registry.opensuse.org/opensuse/leap\:15.3
docker_registry.opensuse.org/opensuse/leap\:15.3:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_ZYP_GROUP_DEVEL)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_ZYP_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f -  . >> ${DOCKER_LOG}
dockerised_test_opensuse_15.3:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_rpm_opensuse_tumbleweed
dockerised_rpm_opensuse_tumbleweed: docker_registry.opensuse.org/opensuse/tumbleweed\:latest
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_registry.opensuse.org/opensuse/tumbleweed\:latest
docker_registry.opensuse.org/opensuse/tumbleweed\:latest:
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_ZYP_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_ZYP_GROUP_DEVEL)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_ZYP_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f -  . >> ${DOCKER_LOG}
dockerised_test_opensuse_tumbleweed:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

########################################## FEDORA ##########################################
.PHONY: dockerised_rpm_fedora_34
dockerised_rpm_fedora_34: docker_fedora\:34
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-fedora:34\
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_fedora\:34
docker_fedora\:34: 
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	@echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_fedora_34:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

########################################## NON FUNCTIONAL TARGETS HERE  ##########################################
.PHONY: dockerised_rpm_fedora_35
dockerised_rpm_fedora_35: docker_fedora\:35
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-fedora:35\
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_fedora\:35
docker_fedora\:35: 
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	"RUN echo \"nameserver 9.9.9.9\" > /etc/resolv.conf \n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}
dockerised_test_fedora_35:
	@echo "Logging $@ to ${DOCKER_BUILD_LOG}"
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-${DOCKER_CONTAINER} \
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} test >> ${DOCKER_BUILD_LOG}

.PHONY: dockerised_rpm_fedora_rawhide
dockerised_rpm_fedora_rawhide: docker_fedora\:rawhide
	@docker run ${DOCKER_RUN_PARAMS} -v ${DOCKER_BASE}:/home/build \
		build-$(PACKAGE_DIR)-fedora:rawhide\
		/home/build/${PACKAGE_DIR}/docker/docker-build.sh ${PACKAGE_DIR} ${DOCKER_DIST} >> ${DOCKER_BUILD_LOG}
.PHONY: docker_fedora\:rawhide
docker_fedora\:rawhide: 
	@echo Logging to ${DOCKER_LOG}
	@test -d docker/log || mkdir -p docker/log
	echo -e \
	$(DOCKER_GEN_FROM_IMAGE)"\n" \
	$(DOCKER_YUM_BUILD_ESSENTIALS)"\n" \
	$(DOCKER_YUM_GROUPS_DEVELTOOLS)"\n" \
	$(DOCKER_COPY_DEPENDENCIES)"\n" \
	$(DOCKER_YUM_INST_DEPENDENCIES) \
	| docker build --tag $(DOCKER_TAG) -f - . >> ${DOCKER_LOG}

delme:
	"RUN sed s_https://_http://_g -i /etc/yum.repos.d/*repo \n" \
	"RUN echo \"nameserver 9.9.9.9\" > /etc/resolv.conf \n" \
