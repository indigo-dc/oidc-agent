.PHONY: preparedeb
preparedeb: clean
	@quilt pop -a || true
	@debian/rules clean
	( cd ..; tar czf ${PKG_NAME}_${VERSION}+ds.orig.tar.gz \
		--exclude="oidc-agent/.git" \
		--exclude="oidc-agent/.pc" \
		--exclude="oidc-agent/windows/webview" \
		--exclude="oidc-agent/windows/webview-build/WebView2Loader.dll.lib" \
		--exclude="oidc-agent/docker/debian.mk" \
		--exclude="oidc-agent/debian" \
		--transform='s_${PKG_NAME}_${PKG_NAME}-$(VERSION)_' \
		${PKG_NAME})

.PHONY: debsource
debsource: distclean preparedeb
	dpkg-source -b .

.PHONY: buster-debsource
buster-debsource: distclean reduce_debhelper_version_13_12 reduce_libjson_version disable_dh_dwz preparedeb
	dpkg-source -b .

.PHONY: focal-debsource
focal-debsource: distclean reduce_debhelper_version_13_12 undepend_libcjson use_own_cjson preparedeb
	dpkg-source -b .

.PHONY: bionic-debsource
bionic-debsource: reduce_debhelper_version_13_12 undepend_libcjson use_own_cjson distclean preparedeb
	# re-add the desktop triggers by hand, because I'm not sure about the
	# debhelpers for this in ubuntu. This is a dirty, but short-term fix.
	@echo "activate-noawait update-desktop-database" > debian/oidc-agent-desktop.triggers
	dpkg-source -b .

.PHONY: reduce_debhelper_version_13_12
reduce_debhelper_version_13_12:
	@mv debian/control debian/control.bck
	@cat debian/control.bck \
		| sed s/"Build-Depends: debhelper-compat (= 13),"/"Build-Depends: debhelper-compat (= 12),"/ \
		> debian/control
	
	@mv debian/liboidc-agent-dev.install debian/liboidc-agent-dev.install.bck
	cat debian/liboidc-agent-dev.install.bck \
		| sed s/"\$${DEB_TARGET_MULTIARCH}"/`dpkg-architecture -qDEB_TARGET_MULTIARCH`/ \
		| sed s/"\$${DEB_HOST_MULTIARCH}"/`dpkg-architecture -qDEB_HOST_MULTIARCH`/ \
		> debian/liboidc-agent-dev.install 
	
	@mv debian/liboidc-agent4.install debian/liboidc-agent4.install.bck
	cat debian/liboidc-agent4.install.bck \
		| sed s/"\$${DEB_TARGET_MULTIARCH}"/`dpkg-architecture -qDEB_TARGET_MULTIARCH`/ \
		| sed s/"\$${DEB_HOST_MULTIARCH}"/`dpkg-architecture -qDEB_HOST_MULTIARCH`/ \
		> debian/liboidc-agent4.install 

.PHONY: reduce_libjson_version
reduce_libjson_version:
	@mv debian/control debian/control.bck
	@cat debian/control.bck \
		| sed s/"libcjson-dev (>= 1.7.14)"/"libcjson-dev (>= 1.7.10-1.1)"/ \
		> debian/control

.PHONY: undepend_libcjson
undepend_libcjson:
	@mv debian/control debian/control.bck
	@cat debian/control.bck \
		|  sed s/"libcjson-dev (>= 1.7.10-1.1)"// \
		> debian/control

.PHONY: use_own_cjson
use_own_cjson:
	@mv debian/rules debian/rules.bck
	@cat debian/rules.bck \
		| sed s/^"export USE_CJSON_SO = 1"/"export USE_CJSON_SO = 0"/ \
		> debian/rules
	@chmod 755 debian/rules

.PHONY: disable_dh_dwz
disable_dh_dwz:
	@echo -e "override_dh_dwz:\n\t@echo disabled" \
		>> debian/rules
	@chmod 755 debian/rules

.PHONY: deb
deb: cleanapi create_obj_dir_structure preparedeb debsource
	debuild -i -b -uc -us
	@echo "Success: DEBs are in parent directory"

.PHONY: buster-deb
buster-deb: cleanapi create_obj_dir_structure preparedeb buster-debsource deb buster-cleanup-debsource

.PHONY: buster-cleanup-debsource
buster-cleanup-debsource:
	@mv debian/control.bck debian/control

.PHONY: bionic-deb
bionic-deb: cleanapi create_obj_dir_structure preparedeb bionic-debsource deb bionic-cleanup-debsource

.PHONY: bionic-cleanup-debsource
bionic-cleanup-debsource:
	@mv debian/control.bck debian/control
	@rm debian/oidc-agent-desktop.triggers

.PHONY: deb-buster
deb-buster: buster-deb

.PHONY: deb-bionic
deb-bionic: bionic-deb

